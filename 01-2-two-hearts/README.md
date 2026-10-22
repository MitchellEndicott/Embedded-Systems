# Program: Two Hearts
**Author(s):** Mitchell Endicott
**Date:** 1/5/26

**Modified by:** Mitchell Endicott
**Date:** 1/5/26

**Purpose:** This program flashes two of the internal LEDs on the nRF52840 DK. One flashes ten times, speeding up each time, and then the other flashes 5 times, slowing down each time. The cycle repeats forever.

## Configuration
Uses default devicetree configuration

## Hardware
### Internal
* LED1 (digital output)
* LED2 (digital output)

## Flow
```mermaid
graph TB
  B("main()") --> C[Set i to 0]
  C --> D{Do it forever?}
  D -- Yes --> S(Set SLEEP_TIME_MS1 to 100 and SLEEP_TIME_MS1 to 1000)
  S --> E{Is i < 10?}
  E -- Yes --> F[Turn LED0 on]
  E -- No --> K{Is i > 0?}
  F --> G([Sleep 100 ms])
  G --> H[Turn LED0 off]
  H --> I([Sleep 100 ms])
  I --> J(Increase i by 1)
  J --> Q(Increase sleep by 100 ms)
  Q --> E
  K -- Yes --> L[Turn LED1 on]
  L --> M([Sleep 1000 ms])
  M --> N[Turn LED1 off]
  N --> O([Sleep 1000 ms])
  O --> P(Decrease i by 1)
  P --> R(Decrease sleep by 200 ms)
  R --> K
  K -- No --> D
```

`Completed:` 1/5/26 2:21pm