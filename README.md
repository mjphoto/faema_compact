# Coffee Machine Controller

## Overview

This project implements a finite state machine (FSM) for controlling a Faema Compact S coffee machine. The code handles various states of the machine, inputs from buttons and sensors, and outputs to relays and an LED strip. The FSM ensures the machine transitions smoothly between different states such as idle, pouring, programming, and flushing.

## Features

- **Finite State Machine**: Manages different states of the coffee machine.
- **Button Inputs**: Handles various button presses for controlling the machine.
- **Sensor Inputs**: Monitors water level and pressure.
- **LED Strip Control**: Provides visual feedback using an LED strip.
- **EEPROM Storage**: Stores preset configurations for the coffee doses.
- **Serial Communication**: Allows interaction via the serial interface.

## Hardware Requirements

- Arduino Nano
- Adafruit NeoPixel LED Strip
- Relays

### Faema Compact Hardware
- Flow Meter
- Pressure Sensor
- Water Level Sensor
- Keypad with multiple buttons

## Pin Configuration

### Inputs

| Component         | Pin  | Description                |
|-------------------|------|----------------------------|
| Flow Meter        | 2    | Digital input for flow meter pulses |
| Fill Status       | A0   | Analog input for water level |
| Pressure Stat     | A1   | Digital input for pressure |
| Button Two Big    | 7    | Digital input for large button 2 |
| Button Flush      | 8    | Digital input for flush button |
| Button One Big    | 9    | Digital input for large button 1 |
| Button One Small  | 10   | Digital input for small button 1 |
| Button Stop       | 11   | Digital input for stop button |
| Button Two Small  | 12   | Digital input for small button 2 |

### Outputs

| Component                | Pin  | Description              |
|--------------------------|------|--------------------------|
| Relay - Pump             | 6    | Controls the pump relay  |
| Relay - Fill Solenoid    | 5    | Controls the fill solenoid relay |
| Relay - Group Solenoid   | 4    | Controls the grouphead solenoid relay |
| Relay - Heater           | 3    | Controls the heater relay |
| LED Strip                | A2   | Controls the NeoPixel LED strip |

## Software Configuration

### States

The FSM has the following states:

- `state_off`: The machine is off.
- `state_idle`: The machine is idle and ready.
- `state_pouring`: The machine is pouring coffee.
- `state_programming_idle`: Idle state for programming presets.
- `state_programming_button`: State for programming a specific button.
- `state_flush`: The machine is flushing.
- `state_preinfuse`: Pre-infusion state.
- `state_preinfuse_delay`: Pre-infusion soak state.

### Setup

1. **Include Necessary Libraries**:
   - Adafruit NeoPixel library for controlling the LED strip.
   - EEPROM library for storing and retrieving preset configurations.

2. **Initialize Components**:
   - Set up pins for inputs and outputs.
   - Initialize the LED strip.
   - Configure serial communication.

### Main Loop

1. **State Management**: 
   - Switches between different states and calls corresponding processing functions.

2. **LED Strip Management**:
   - Provides visual feedback based on the current state.

3. **Output Management**:
   - Controls relays based on sensor inputs and the current state.

4. **Button Listeners**:
   - Listens for long presses on the stop and flush buttons to trigger specific state transitions.

### Configurable Variables

- `preinfusion_time`: Change this value to set the preinfusion time.
- `preinfusion_delay_time`: Change this value to set the preinfusion soak time.

## Usage

- **Power On/Off**: Hold the stop button to toggle between off and idle states.
- **Start Pouring**: Press one of the coffee buttons to start the coffee extraction process.
- **Flush**: Press the flush button to start the flushing process.

### Programming Presets
- Hold the flush button while the machine is off to enter programming mode.
- Press the desired button to start programming its preset
- Press the stop button to stop pouring once the desired volume is poured, this will set the volume for the preset.
- Press & hold the stop button to exit programming mode.

## Debugging

- Enable debugging by defining `DEBUG` to print state information and sensor values to the serial monitor.

```cpp
#define DEBUG
```

## Dependencies

- Adafruit NeoPixel library
- EEPROM library
