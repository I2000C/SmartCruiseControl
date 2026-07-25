# Smart Forfour 454 (2006) CAN Bus Reverse Engineering (1.5 CDI Diesel)

This document describes observed CAN bus frames from a **Smart Forfour 454 (2006, 1.5 CDI diesel)**.  
Values are based on real-world sniffing and are partially reverse engineered.


It has been observed that some signals are similar between this vehicle and the **Smart Fortwo 451**, as documented in the following project:
https://hackaday.io/project/19931-smart-fortwo-can-bus-reverse-engineering

---

## 0x002 — Length: 2 bytes
- Byte 0: Oscillates between 0 and 1
- Byte 1: Counter from 0 to 0xFE (254)

---

## 0x038 — Length: 5 bytes
- Bytes 0–4: Always `0xFF`

---

## 0x09E (+) — Length: 7 bytes
- Bytes 0–2: Always `0xFF`
- Byte 3: Always `0xFE`
- Bytes 4–6: 24-bit value → **total distance traveled (in 0.1 km units)**

---

## 0x0C0 — Length: 5 bytes
- Bytes 0–1: Likely signed 16-bit value (unknown meaning)
- Byte 2: Possibly braking intensity?
- Byte 3: Always `0x07`
- Byte 4: Unknown counter-like value

---

## 0x101 (*) — Length: 8 bytes
Very unusual frame (only sent 5 times)

- Byte 0: 0 → 255 cycle
- Byte 1: 0xC5 → 0
- Byte 2: 0x65 → 0
- Byte 3: 0xEE → 0
- Byte 4: 0x38 → 0
- Byte 5: 0x89 → 0
- Byte 6: 0xB0 → 0
- Byte 7: 0x50 → 0

---

## 0x111 (*) — Length: 8 bytes
Sent only 4 times

- Byte 0: Always 0
- Byte 1: Always 0xAB
- Byte 2: Always 0x17
- Byte 3: Always 0x1A
- Bytes 4–7: Always 0

---

## 0x119 (*) — Length: 8 bytes
Sent only 9 times

- Bytes 0–1: Always 0
- Byte 2: 1 → 0 transition
- Bytes 3–4: Always 0
- Byte 5: Always 0x08
- Byte 6: Increases slowly from 1 to 9
- Byte 7: Varies (0x03 → 0xB8)

---

## 0x130 — Length: 8 bytes
- Byte 0: Oscillates between 0 and 11
- Byte 1: Fast counter (0 → 0xFE)
- Byte 2: Unknown
- Bytes 3–5: Oscillating values
- Byte 6: Usually 0 (rare peak at 0x08)
- Byte 7: Counter-like (0 → 0xFF)

---

## 0x194 (*) — Length: 1 byte
- Byte 0: Always `0xFE`

---

## 0x1A4 (*) — Length: 8 bytes
Constant frame

- Byte 0: 0x05
- Byte 1: 0x03
- Byte 2: 0x20
- Byte 3: 0x02
- Byte 4: 0x0B
- Byte 5: 0xC3
- Bytes 6–7: Always 0

---

## 0x1E1 — Length: 8 bytes
- Byte 0: 0x81 → 0 → stays 0
- Bytes 1–7: Always 0

---

## 0x200 — Length: 8 bytes
- Byte 0:
  - Bit 1: **Handbrake status**
- Byte 1: Counter (0 → 0x7D)
- Bytes 2–3: **Vehicle speed (km/h × 0.05)**
- Bytes 4–5: **Vehicle speed (km/h × 0.05)**
- Bytes 6–7: **Vehicle speed (km/h × 0.05)**

---

## 0x208 (+) — Length: 8 bytes
- Byte 0: Always 0
- Byte 1: 0x30 → 0x20 → stable
- Bytes 2–3: **Brake intensity (max value: 0x4230)**
- Bytes 4–5: **Vehicle speed (km/h × 0.05)**
- Bytes 6–7: **Vehicle speed (km/h × 0.05)**

---

## 0x210 (+) — Length: 8 bytes
- Bytes 0–1: Always 0
- Byte 2: **Accelerator pedal sensor 1 (0 → 0xA1)**
- Byte 3: Transient value, stabilizes at 0x40
- Bytes 4–7: Always 0

---

## 0x212 (+) — Length: 8 bytes
- Byte 0: 0 → 3
- Byte 1: 0 → 0x86 → 0x84
- Bytes 2–3: Always 0
- Bytes 4–5: **Accelerator pedal sensor 2 (0x277C → 0x2A0A)**
- Bytes 6–7: Always 0

---

## 0x270 (*) — Length: 8 bytes
- Bytes 0–1: Counters (0 → 0xFF)
- Bytes 2–7: Always 0xFF

---

## 0x2F1 — Length: 2 bytes
- Byte 0: Unknown
- Byte 1: 0x80 → 0

---

## 0x300 — Length: 8 bytes
- Byte 0: Always 0
- Byte 1: 0x08 → 0x8D
- Byte 2: Always 0x1F
- Byte 3: Always 0xFF
- Byte 4: 0 → 0x80 oscillation
- Byte 5: Always 0
- Bytes 6–7: Unknown 16-bit value

---

## 0x308 (+) — Length: 8 bytes
- Byte 0: 0 or 0x80 (state-dependent)
- Bytes 1–2: **Engine RPM (16-bit)**
- Byte 3: 0x0C → 0x00 decay
- Byte 4: 0x10 → spike → 0
- Byte 5: Unknown (0xFF → 0x82 stabilizing)
- Byte 6: Always 0x20
- Byte 7: Always 0

---

## 0x312 — Length: 8 bytes
All fields unknown (16-bit blocks)

---

## 0x328 — Length: 8 bytes
- Bytes 0–1: Always 0xFF
- Byte 2: Oscillates (0x3B → 0xB8)
- Byte 3: Always 0
- Byte 4: 0xDF → 0xC0 → 0xD0 → 0xC0
- Byte 5: Oscillates (0–0x47)
- Bytes 6–7: Counters (0 → 0xFF)

---

## 0x408 (+) — Length: 8 bytes
- Byte 0: **Fuel tank level (liters)**
- Byte 1: Always 0
- Byte 2: ~110 (0x6E)
- Byte 3: Always 0
- Byte 4: Always 0xFE
- Byte 5: Always 0xC3
- Byte 6: Always 0x4F
- Byte 7: Always 0

---

## 0x40C (+) — Length: 5 bytes
- Byte 0: Always 0xFF
- Byte 1: **Speed (dashboard km/h)**
- Byte 2: Always 0x80
- Byte 3: Always 0
- Byte 4: Always 0x40

---

## 0x412 — Length: 8 bytes
- Byte 0: Always 1
- Byte 1: **Speed (dashboard km/h + slightly offset)**
- Byte 2: Always 2
- Byte 3: 0xA0
- Byte 4: Slowly increasing unknown value
- Byte 5: Always 0
- Byte 6: Always 1
- Byte 7: 0xDF

---

## 0x416 (+) — Length: 8 bytes
- Byte 0: **Battery voltage (in 0.1V units)**
- Bytes 1–7: Always 0

---

## 0x423 (+) — Length: 6 bytes
- Byte 0:
  - Bits 0–1: **Ignition state (LOCK = 0b00 / ACC = 0b01 / ON = 0b11 / START = 0b10)**
  - Bit 2: **Position lights**
  - Bit 3: **Front fog lights**
  - Bit 4: **Rear fog light**
- Byte 1:
  - Bit 0: **Right turn signal**
  - Bit 1: **Left turn signal**
  - Bit 2: **High beam headlights**
  - Bit 3: **Reverse light**
- Byte 2:
  - Bit 0: **Right front door**
  - Bit 1: **Left front door**
  - Bit 3: **Right rear door**
  - Bit 4: **Left rear door**
  - Bit 6: **Trunk door**
- Byte 3:
  - Bit 0: **Fan state**
- Byte 4: Always 0x15
- Byte 5: 0x05–0x06

---

## 0x443 (*) — Length: 6 bytes
- Byte 0: 0 → 2 spike
- Byte 1: 0xFF → 0 → 0x23 cycle
- Bytes 2–5: Always 0

---

## 0x536 (*) — Length: 8 bytes
Constant diagnostic-like frame

- Byte 0: 0xAB
- Byte 1: 0x68
- Byte 2: 0x4C
- Byte 3: 0xB1
- Byte 4: 0x0C
- Byte 5: 0
- Byte 6: 0x02
- Byte 7: 0

---

## 0x608 (+) — Length: 8 bytes
- Byte 0: **Coolant temperature (°C + offset 40)**
- Byte 1: Always 0
- Byte 2: 0x18
- Byte 3: 0x66
- Byte 4: 0xFF
- Bytes 5–6: **Fuel consumption (L/h × 0.01)**
- Byte 7: Always 0

---

## 0x7C0 (*) — Length: 2 bytes
Single transmission frame

- Byte 0: 0x53
- Byte 1: 0xF5

---

## Notes
- Frames marked with `(*)` are rare or burst transmissions.
- Frames marked with `(+)` likely contain key vehicle state signals.
- Several signals are still unknown and require further reverse engineering.