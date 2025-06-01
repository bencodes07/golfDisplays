# GolfDisplays

GolfDisplays is a project designed to provide a seamless way to monitor engine statistics and metrics for a highly tuned Golf MK1. Despite the car's original appearance and interior, this project allows you to view crucial engine data on discreet screens embedded in the dash and radio. Additionally, an Arduino Nano manages the gear shifting for the DQ250 gearbox used in the car.

## Purpose
This project aims to provide an unobtrusive way to display engine data for a Golf MK1 with extensive tuning and modifications, including a DQ250 gearbox. This of course only works if the displays have been installed into the dash and radio

## Hardware Used
- Arduino Nano
- OLED displays
- MCP2515 CAN Bus Module
- MCP4725 DAC

## Software Requirements
This project requires specific libraries to read data from the CAN Bus of the ECU and display it on the OLED screens.

## Setup Instructions
1. Download the code from this repository.
2. Connect your hardware components as specified in the code.
3. Upload the provided sketches to the Arduino Nano using the Arduino IDE.

## Usage
The system will automatically start displaying engine data once powered on. The data will cycle through different types and values on button presses, providing a comprehensive overview of the engine's performance metrics.

## Features
- Displays various engine statistics and metrics.
- Cycles through different types and values of engine data.
- Uses OLED displays for clear and compact data visualization.
- Gear shifting for the DQ250 gearbox.

## License
This project is licensed under the MIT License. You are free to use, modify, and distribute this project.
