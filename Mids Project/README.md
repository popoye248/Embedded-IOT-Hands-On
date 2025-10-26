# Assignment1-Question3-TaskB

An ESP32 project demonstrating button press detection to control a LED and Buzzer.
The LED toggles with a short press and the buzzer beeps for 5 seconds on a long press.

## Features

- Detects Short and Long Press
- Short Press (<1.5 s): togles LED
- Short Press (≥1.5 s): togles Buzzer
- Compatible with PlatformIO and Arduino Framework


## Requirements

- ESP32 Devkit
- Platform IO extension for VS Code
- Micro USB cable
- Buzzzer and LED connected to ESP32 with wires


## Usage

- Open the project in PlatformIO in VS Code
- Connect ESP2 via USB
- Upload the code using the PlatformIO: Upload button
- Press the button:
                   Short Press -> LED Toggles
                   Long Press -> Buzzer Beeps

## Wokwi Link
https://wokwi.com/projects/445689693043778561

## Pinmap
Here’s the Pinmap diagram for the ESP32 button–LED–buzzer setup:

![Pinmap Diagram](Pinmap.jpg)

## Screenshots
Here’s the circuit diagram for the ESP32 button–LED–buzzer setup:

![Circuit Screenshot](TaskB-Screenshot.png)

