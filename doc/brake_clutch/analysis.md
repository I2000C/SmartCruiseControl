# Brake and Clutch Switch Analysis (Smart Forfour 454, 2006)

## System Overview

Both the brake and clutch pedals use simple switch assemblies that interface with the ECU through pull-up resistors. The ECU reads these signals as logic levels (approximately 0 V or ~12 V depending on pedal state), rather than clean digital GPIO-style signals.

Both switches behave similarly:

* **ECU input is pulled up internally to ~12 V**
* **Switch closure pulls the signal to GND**

# Brake Switch

## Pinout

The brake switch uses a 4-pin connector with the following wiring:

* Brown wire: **+12 V supply**
* Black wire: **GND**
* Green/black wire: **ECU signal**
* Blue/orange wire: **Brake lights output**

### Internal grouping

The switch contains two independent circuits:

* **Power to brake lights**

  * +12 V ↔ Brake lights
* **ECU sensing circuit**

  * GND ↔ ECU signal

## Physical location and identification

The brake switch is located at the pedal assembly and its connector and wiring can be identified in the following images:

* `brake switch connector.jpg`
* `brake switch connector2.jpg`
* `brake switch connector3.jpg`

These images show the physical connector, pin arrangement, and wiring colors used for identification.

## Electrical behavior

### Brake NOT pressed (idle state)

* GND is connected to ECU pin
* ECU sees approximately **0 V**
* Brake lights circuit is open

### Brake PRESSED

* +12 V is connected to brake lights output
* ECU input is disconnected from GND and pulled up internally
* ECU sees approximately **11–12 V**

Measured behavior:

* Idle: ~0 V at ECU pin
* Active: ~11–12 V at ECU pin

## ECU pull-up resistor estimation

Measurements using an external 100 kΩ resistor allowed estimation of the internal pull-up:

* Without external load: ~11.22 V
* With 100 kΩ load: ~10.98 V

Estimated internal pull-up resistance:

* **≈ 2.2 kΩ**

## Signal conditioning for microcontroller (ESP32)

Since the ECU signal operates at automotive voltage levels (~12 V), direct connection to an ESP32 GPIO is not safe.

### Recommended approach: NPN transistor interface

A simple level-shifting circuit can be used using a **2N3904 NPN transistor**.

**Connections:**

* Base (B): ECU signal via **10 kΩ resistor**
* Collector (C): ESP32 GPIO input (with pull-up enabled)
* Emitter (E): GND

This inverts the signal:

* Brake NOT pressed → GPIO HIGH
* Brake pressed → GPIO LOW

# Clutch Switch

## Pinout

The clutch switch is electrically similar to the brake ECU sensing circuit but simpler:

* Black wire: **GND**
* Red/yellow wire: **ECU signal**

Only two wires are present because this switch does not control any secondary load like brake lights.

## Physical location and identification

The clutch switch is located behind the fuse box and is not directly visible. Access is limited, so the correct wire was identified by probing, testing, and carefully pulling and tracing the harness until the correct signal line was found.

The identified clutch signal wire is shown in:

* `clutch switch wire.jpg`
* `clutch switch wire2.jpg`

## Electrical behavior

### Clutch NOT pressed (idle state)

* ECU signal is pulled up internally
* Measured voltage: **~11.7 V**

### Clutch PRESSED

* Signal is pulled to ground
* Measured voltage: **~0 V**

Measured with external 100 kΩ reference resistor:

* With load: ~11.4 V
* Without load: ~11.7 V

## ECU pull-up resistor estimation

Estimated internal pull-up:

* **≈ 2.6 kΩ**

This is consistent with the brake circuit and indicates a similar ECU input design.

## Signal conditioning for ESP32

The same NPN transistor approach applies:

* Base: ECU signal via 10 kΩ resistor
* Collector: ESP32 GPIO (pull-up enabled)
* Emitter: GND

Logic is inverted:

* Clutch released → HIGH
* Clutch pressed → LOW

# Notes

* Both pedal inputs are **12 V automotive logic signals**, not direct digital inputs.
* Internal ECU pull-ups are relatively low (~2–3 kΩ), indicating relatively strong signal driving capability.
* Using a transistor interface is strongly recommended for reliability and protection of microcontroller inputs.
* Measurement consistency suggests both brake and clutch inputs share a similar ECU input architecture, despite slight differences in pull-up values.

# Circuit visualization

The electrical circuit of the brake pedal switch (the clutch switch is not shown due to its very similar behavior) can be visualized using the Falstad Circuit Simulator:

* Go to: https://www.falstad.com/circuit/circuitjs.html
* Load the file: `brake-pedal-switch-circuit.txt`
