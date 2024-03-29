// FINITE STATE MACHINE STATES
const int state_off = 0;
const int state_idle = 1;
const int state_pouring = 2;
const int state_programming_idle = 3;
const int state_programming_button = 4;
const int state_flush = 5;
const int state_preinfuse = 6;
int state = state_off;

boolean WATER_NEEDED;
boolean HEAT_NEEDED;

// INPUTS
const int FLOW_METER = 2;
const int FLOW_INT = 0;  // 0 = digital pin 2
const int FILL_STAT = A0;
const int PRESSURE_STAT = A1; // use as digital input pin.
// KEYPAD BUTTONS
const int BUTTON_TWO_BIG = 7;
const int BUTTON_FLUSH = 8;
const int BUTTON_ONE_BIG = 9;
const int BUTTON_ONE_SMALL = 10;
const int BUTTON_STOP = 11;
const int BUTTON_TWO_SMALL = 12;
const int BUTTON[4] = {BUTTON_ONE_SMALL, BUTTON_TWO_SMALL, BUTTON_ONE_BIG, BUTTON_TWO_BIG};

// OUTPUTS
const int RELAY_PUMP = 6;
const int RELAY_FILL_SOLENOID = 5;
const int RELAY_GRP_SOLENOID = 4;
const int RELAY_HEATER = 3;
const int led_13 = 13;

// LED STRIP
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN A2
Adafruit_NeoPixel strip = Adafruit_NeoPixel(15, PIN, NEO_GRB + NEO_KHZ800);
const int LED_STRIP = A2; //(A2 LED Strip)
int rainbow_j=0;
int runlight_j=0;
int num_pix_old = 0;

// PRESET STORAGE
#include <EEPROM.h>
//#include "/home/mw/git/faema_compact_control.h"
#include "C:\Users\matt.jones\Downloads\faema_compact_control-master\faema_compact_control-master/EEPROMAnything.h"
struct config_t
{
   int preset[4];
} configuration;


// FLOW_METER DOSE PRESETS
int dose;
volatile int flow_counter=0;

int selected_button;
// BUTTON TIME COUNTER
int stop_cnt = 0;
int flush_cnt = 0;

// SETUP SERIAL COMM
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete


// definition options
#define DEBUG
//Type of pump definitions for best preinfusion
//#define VIBE
#define ROTARY

void setup_ledstrip(){
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void setup() {
    // INPUTS
    Serial.begin(9600);
    inputString.reserve(200); // reserve mem for received message on serial port
    EEPROM_readAnything(0, configuration);
    for (byte x=0;x<4;x++){
      if (configuration.preset[x] == -1) {
        configuration.preset[x] = 10;
      }
    }
    EEPROM_writeAnything(0, configuration);

    pinMode(BUTTON[0], INPUT);
    pinMode(BUTTON[1], INPUT);
    pinMode(BUTTON[2], INPUT);
    pinMode(BUTTON[3], INPUT);
    pinMode(BUTTON_FLUSH, INPUT);
    pinMode(BUTTON_STOP, INPUT);
   
    pinMode(FILL_STAT, INPUT);
    pinMode(PRESSURE_STAT, INPUT);
    pinMode(FLOW_METER, INPUT);
    digitalWrite(FLOW_METER, HIGH);
    attachInterrupt(FLOW_INT, pulseCounter, FALLING);

    // OUTPUTS
    pinMode(RELAY_PUMP, OUTPUT);
    pinMode(RELAY_FILL_SOLENOID, OUTPUT);
    pinMode(RELAY_GRP_SOLENOID, OUTPUT);
    pinMode(RELAY_HEATER, OUTPUT);
    pinMode(led_13, OUTPUT);
    digitalWrite(led_13, LOW);

    setup_ledstrip();

}

void pulseCounter()
{
  flow_counter++; 
}

unsigned long STOP_BUTTON_PUSH_START = 0;
unsigned long FLUSH_BUTTON_PUSH_START = 0;
const long STOP_BUTTON_TIME_THRESHOLD = 1500;
const long FLUSH_BUTTON_TIME_THRESHOLD = 1500;
const long preinfusion_time = 3000; //3 seconds of time for water pressure to infuse puck before pump starts
unsigned long preinfuse_counter = 0;
unsigned long preinfuse_present;
unsigned long present;


//Led strip stuff
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

int color_cnt=0;

void led_mode_rainbow(){
  color_cnt++;
  if (color_cnt>10){
    color_cnt = 0;
    rainbow_j++;
    if (rainbow_j==255){
      rainbow_j = 1;
    }
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+rainbow_j) & 255));
    }
    strip.show();
  }
}

void led_mode_white(){
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0x949494);
   }
   strip.show();
}

void led_mode_off(){
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0x000000);
   }
   strip.show();
}


void led_mode_percent(){
  float fraction;
  fraction = float(flow_counter)/dose;
  int num_pix = int(strip.numPixels()*fraction);
  if (num_pix != num_pix_old){
    num_pix_old = num_pix;
    for(int i=0; i<strip.numPixels()/2+1; i++) {
      if (i<num_pix/2+1){
        long c =  0x840000+i*(20)*256;
        strip.setPixelColor(i, c );
        strip.setPixelColor(strip.numPixels()-1-i, c);
      }else{
        strip.setPixelColor(i, 0x000000);
        strip.setPixelColor(strip.numPixels()-1-i, 0x000000);
      }
    }
    strip.show();
  }
}


void manage_ledstrip(){
  switch (state) {
    case state_off:
      led_mode_off();
      break;
    case state_idle:
      led_mode_white();
      break;
    case state_pouring:
      led_mode_percent();
      break;
    case state_programming_idle:
      led_mode_white();
      break;
    case state_programming_button:
      led_mode_white();
      break;
    case state_flush:
      led_mode_rainbow();
      break;
  }
}

void loop() {
  //Serial.println(state);
  switch (state) {
    case state_off:
      proc_off();
      break;
    case state_idle:
      proc_idle();
      break;
    case state_pouring:
      proc_pouring();
      break;
    case state_programming_idle:
      proc_programming_idle();
      break;
    case state_programming_button:
      proc_programming_button();
      break;
    case state_flush:
      proc_flush();
      break;
    #ifdef ROTARY
    case state_preinfuse:
      proc_preinfuse();
      break;
    #endif
  }
  manage_outputs();
  manage_ledstrip();
  if (stringComplete){
    inputString = "";
    stringComplete = false;
  }
  delay(10);
  
}


void push_n_hold_stop_button_listener(int target_state){
  if (digitalRead(BUTTON_STOP) == HIGH){
    if (STOP_BUTTON_PUSH_START == 0){
      STOP_BUTTON_PUSH_START = millis();
    }
    unsigned long now = millis();
    unsigned long stop_button_time_pushed = now-STOP_BUTTON_PUSH_START;
    if (stop_button_time_pushed > STOP_BUTTON_TIME_THRESHOLD){
      STOP_BUTTON_PUSH_START = 0;
      if (target_state == state_off){
        Serial.println("m:publish(\"coffee/state\",0,0,0, function(conn) end )");
      }
      if (target_state == state_idle){
        Serial.println("m:publish(\"coffee/state\",1,0,0, function(conn) end )");
      }
      state = target_state;
    }
  }else {
    STOP_BUTTON_PUSH_START = 0;
  }
}

void push_n_hold_flush_button_listener(int target_state){
  if (digitalRead(BUTTON_FLUSH) == HIGH){
    if (FLUSH_BUTTON_PUSH_START == 0){
      FLUSH_BUTTON_PUSH_START = millis();
    }
    unsigned long now = millis();
    unsigned long stop_button_time_pushed = now-FLUSH_BUTTON_PUSH_START;
    if (stop_button_time_pushed > FLUSH_BUTTON_TIME_THRESHOLD){
      FLUSH_BUTTON_PUSH_START = 0;
      state = target_state;
    }
  }else {
    FLUSH_BUTTON_PUSH_START = 0;
  }
}


void proc_off(){
  #ifdef DEBUG
  Serial.println("off");
  #endif
  if (stringComplete) {
    if (inputString.startsWith("Turn On Services")) {
      inputString = "";
      stringComplete = false;
      Serial.println("m:publish(\"coffee/state\",1,0,0, function(conn) end )");
      state = state_idle;
      delay(10);
    }
  }
  push_n_hold_stop_button_listener(state_idle);
  push_n_hold_flush_button_listener(state_programming_idle);
}

void proc_idle(){
  #ifdef DEBUG
  Serial.println("idle");
  #endif
  if (stringComplete) {
    if (inputString.startsWith("Turn Off Services")) {
      inputString = "";
      stringComplete = false;
      Serial.println("m:publish(\"coffee/state\",0,0,0, function(conn) end )");
      state = state_off;
      delay(10);
    }
  }
  push_n_hold_stop_button_listener(state_off);
  for (byte x=0;x<4;x++){
    if (digitalRead(BUTTON[x]) == HIGH){
      Serial.println("Button selected!");
      dose = configuration.preset[x];
      //dose = 20;
      detachInterrupt(FLOW_INT);
      flow_counter = 0;
      preinfuse_counter = 0;
      attachInterrupt(FLOW_INT, pulseCounter, FALLING);
      #ifdef VIBE
      state = state_pouring;
      #endif
      #ifdef ROTARY
      Serial.println("ROTARY!");
      state = state_preinfuse;
      #endif
    }
  }
  if (digitalRead(BUTTON_FLUSH) == HIGH){
    state = state_flush;
  }
}

void proc_pouring(){
  #ifdef DEBUG
  Serial.println("pouring...");
  Serial.println(flow_counter);
  Serial.println(dose);
  #endif
   if (flow_counter > dose){
    state = state_idle;
  }
  if (digitalRead(BUTTON_STOP) == HIGH){
    state = state_idle;
  }
}

void proc_preinfuse(){
  //preinfusion counter
  if (preinfuse_counter == 0){
    preinfuse_counter = millis();
  }
  unsigned long present = millis();
  preinfuse_present = present - preinfuse_counter;
  #ifdef DEBUG
  Serial.print("Pouring: Preinfuse time is ");
  Serial.println(preinfuse_present);
  Serial.print("Flow counter now at: ");
  Serial.println(flow_counter);
  #endif
  if (preinfusion_time < preinfuse_present){
     state = state_pouring;
  }
  if (flow_counter > dose){
    state = state_idle;
    preinfuse_counter = 0;
    preinfuse_present = 0;
  }
  if (digitalRead(BUTTON_STOP) == HIGH){
    state = state_idle;
    preinfuse_counter = 0;
    preinfuse_present = 0;
  }
}

void proc_programming_idle(){
  #ifdef DEBUG
  Serial.println("programming idle");
  #endif

  push_n_hold_stop_button_listener(state_off);
  for (byte x=0;x<4;x++){
    if (digitalRead(BUTTON[x]) == HIGH){
      detachInterrupt(FLOW_INT);
      flow_counter = 0;
      attachInterrupt(FLOW_INT, pulseCounter, FALLING);
      selected_button = x;
      state = state_programming_button;
    }
  }
}


void attach_test() {
  Serial.println("interrup received!");
  flow_counter++;
}

void proc_programming_button(){
  #ifdef DEBUG
  Serial.println("programming button...");
  #endif

  if (digitalRead(BUTTON_STOP) == HIGH){
    //detachInterrupt(FLOW_INT);
    configuration.preset[selected_button] = flow_counter;
    //flow_counter = 0;
    //attachInterrupt(FLOW_INT, pulseCounter, FALLING);
    //attachInterrupt(FLOW_INT, attach_test, FALLING);
    Serial.println(flow_counter);
    //Serial.println(configuration);
    EEPROM_writeAnything(0, configuration);
    state = state_programming_idle;
  }
}

void proc_flush(){
  #ifdef DEBUG
  Serial.println("flushing...");
  Serial.println(flow_counter);
  #endif

  if (digitalRead(BUTTON_STOP) == HIGH){
    state = state_idle;
  }
}


unsigned long WATER_NEEDED_TIME_THRESHOLD = 300000; // 5 minutes in milli seconds
unsigned long WATER_NEEDED_START = 0;
unsigned long FILL_STAT_HIGH_START = 0;
unsigned int  FILL_STAT_LVL_THRESHOLD = 720; // normally (open) high -> 1023. when touching water (closed) 420
unsigned long FILL_TRIGGER_TIME_THRESHOLD = 3000;  // 3s in milli seconds 

void manage_outputs(){
//  unsigned long now = millis(); //to remove as per lates changes in commit "added more recent version and changed number leds to 15"
  unsigned int fill_stat_analog_val=0;
  fill_stat_analog_val = analogRead(FILL_STAT);
  #ifdef DEBUGf
  //Serial.print("Fill stat value: ");
  //Serial.println(fill_stat_analog_val);
  //Serial.println(flow_counter);
  #endif

  if (fill_stat_analog_val>FILL_STAT_LVL_THRESHOLD){ // high -> water needed
    if (FILL_STAT_HIGH_START == 0){
      FILL_STAT_HIGH_START = millis();
    }
    unsigned long now = millis();
    unsigned long fill_stat_time_high = now-FILL_STAT_HIGH_START;
    if (fill_stat_time_high>FILL_TRIGGER_TIME_THRESHOLD){
      WATER_NEEDED = true;
    } else {
      WATER_NEEDED = false;
    }
  }else {
    FILL_STAT_HIGH_START = 0;
    WATER_NEEDED = false;
  }
  
  if (digitalRead(PRESSURE_STAT)==LOW){
    HEAT_NEEDED = true;
  }else {
    HEAT_NEEDED = false;
  }
  if ((state != state_off) and WATER_NEEDED) {
    if (WATER_NEEDED_START == 0){
      WATER_NEEDED_START = millis();
    }
    unsigned long now = millis();
    unsigned long time_on = (now-WATER_NEEDED_START);
    if (WATER_NEEDED_TIME_THRESHOLD < time_on){
      #ifdef DEBUG
      Serial.println("ERROR: TURNED OFF WATER NEEDED TOO LONG");
      #endif
      state = state_off;
    }
  } else {
    WATER_NEEDED_START = 0;
  }

  
  if ((state != state_off) and HEAT_NEEDED){
    digitalWrite(RELAY_HEATER,HIGH);}
  else{
    digitalWrite(RELAY_HEATER,LOW);
  }
  
  if ((WATER_NEEDED and (state == state_idle)) or (state == state_flush) or (state == state_pouring) or (state == state_programming_button)){
    digitalWrite(RELAY_PUMP,HIGH);
  }else{
    digitalWrite(RELAY_PUMP,LOW);
  }
  
  if ((state == state_flush) or (state == state_pouring) or (state == state_programming_button) or (state == state_preinfuse)){
    digitalWrite(RELAY_GRP_SOLENOID,HIGH);
  } else{
    digitalWrite(RELAY_GRP_SOLENOID,LOW);
  }
  if ((state == state_idle) and WATER_NEEDED) {
    digitalWrite(RELAY_FILL_SOLENOID,HIGH);
  }else{
    digitalWrite(RELAY_FILL_SOLENOID,LOW);
  }
}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();  // add it to the inputString:
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}
