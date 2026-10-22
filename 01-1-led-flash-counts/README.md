# Program: Your Turn 1.1
**Author(s):** Mitchell Endicott
**Date:** 1/5/26

**Modified by:** Mitchell Endicott
**Date:** 1/5/26

**Purpose:** This program flashes two of the internal LEDs on the nRF52840 DK. One flashes ten times and then the other flashes 5 times. The cycle repeats forever.

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
  D -- Yes --> E{Is i < 20?}
  E -- Yes --> F[Turn LED0 on]
  E -- No --> K{Is i > 10?}
  F --> G([Sleep 500 ms])
  G --> H[Turn LED0 off]
  H --> I([Sleep 500 ms])
  I --> J(Increase i by 1)
  J --> E
  K -- Yes --> L[Turn LED1 on]
  L --> M([Sleep 250 ms])
  M --> N[Turn LED1 off]
  N --> O([Sleep 250 ms])
  O --> P(Decrease i by 1)
  P --> K
  K -- No --> T[Set i to 0] --> D
```

`Completed:` 1/5/26 11:58 am