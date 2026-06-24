# SmartCruiseControl

SmartCruiseControl is an embedded ESP32 project that implements cruise control using CAN Bus data, with vehicle state monitoring and ELM327 command emulation for the **Smart forfour 454** car

## Description

The software runs on an ESP32 using the Arduino framework and focuses on:

- Reading CAN Bus frames from a Smart vehicle.
- Estimating speed, RPM, distance, fuel level, and other critical vehicle parameters.
- Cruise control with PID, button-based enable/disable, and throttle override handling.
- Basic ELM327 emulation for OBD-II `01xx` and `22xx` command responses.
- Cruise control status indication using an LED.

## Key features

- CAN Bus listen-only mode (`TWAI_MODE_LISTEN_ONLY`) at 500 kbps.
- Support for reading the following key CAN IDs:
  - `0x09E` - total distance
  - `0x200` - speed
  - `0x208` - brake intensity and extra speed data
  - `0x210` - accelerator pedal position
  - `0x308` - engine RPM
  - `0x408` - fuel level
  - `0x416` - battery voltage
  - `0x423` - door status
  - `0x608` - refrigerant temperature and fuel rate
- Cruise control state machine with:
  - `STATE_OFF`
  - `STATE_ACTIVE`
  - `STATE_OVERRIDE`
- ELM327 interface over `Serial` with basic `AT`, `01`, and `22` command handling.

## Project structure

- `platformio.ini` - PlatformIO configuration for ESP32.
- `src/main.cpp` - main entry point and FreeRTOS task setup.
- `include/` - constants, state definitions, and shared headers.
- `lib/` - modular libraries:
  - `can/` - CAN bus reading and decoding.
  - `cruise_control/` - cruise control logic.
  - `elm327/` - ELM327 emulation and OBD response handling.
  - `fuel_range_estimator/` - fuel range estimation.
  - `indicator_led/` - PWM LED status indicator.
  - `pid_controller/` - generic PID controller.
  - `speed_estimator/` - speed estimation from CAN frames.
  - `throttle/` - throttle output and override management.
  - `buttons/` - analog button reading and cruise control button detection.
- `doc/` - CAN Bus reverse engineering documentation.
- `test/` - PlatformIO testing notes and setup.

## Requirements

See `requirements.md` file for required software and hardware

## How to build

See `how-to-build.md` file to build software and hardware

## Usage

- The system starts by creating FreeRTOS tasks for CAN reception, CAN processing, and ELM327 emulation.
- The indicator LED shows cruise control status:
  - off = cruise control disabled
  - full brightness = cruise control active
  - half brightness = throttle override active
- Cruise control buttons support `SET`, `RESUME`, and `CANCEL`.
- Pressing the accelerator while cruise control is active transitions the system to `STATE_OVERRIDE`.

## Notes

- The project is designed for the Smart Forfour 454 (2006) model, as described at the beginning of `doc/CAN bus reverse engineering.md`.
- With the necessary modifications, it could be adapted to other vehicles that use CAN Bus, electronic throttle, and physical brake/clutch switches.
- The implementation is based on CAN data documented in `doc/CAN bus reverse engineering.md`.
- The ESP32 pinout reference image is available at `doc/ESP32 pinout.png` and is used under Creative Commons BY-NC-ND via www.mischianti.org.

## Inspirations

This project was inspired by:

- [esp32_cruise_control](https://github.com/successdt/esp32_cruise_control)
- [autocruise](https://github.com/SantiagoDelEstero/autocruise)
- [esp32-elm327-cruise-control](https://github.com/ipepe-oss/esp32-elm327-cruise-control)

## License

See the `LICENSE` file for the project license terms.
