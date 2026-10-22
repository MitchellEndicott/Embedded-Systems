# Program: Analog temperature logger
**Author(s):** Mitchell Endicott
**Date:** 1/21/26

**Modified by:** Mitchell Endicott
**Date:** 1/21/26

**Purpose:** This application uses a TMP36 analog temperature sensor to record temperature once every five seconds. Temperature samples are stored in an array with a maximum capacity of twelve readings. When the buffer becomes full, an LED is turned on to indicate that the maximum number of samples has been reached. If additional samples are collected, the oldest values are overwritten so that only the twelve most recent measurements are kept. When a button is pressed, all stored temperature values and their average are logged, the buffer is cleared, and the LED is turned off.

## Configuration

Devicetree overlay:

* ADC enabled for analog temperature measurement
* SAADC channel configured for TMP36 input (AIN1)
* TMP36 defined as an analog temperature sensor device

Kernel configuration via `prj.conf`:

* Logging
* ADC

## Hardware
### External

* TMP36 temperature sensor

### Internal

* Button (digital input)
* LED0 (digital output – buffer full indicator)

## Flow
```mermaid
graph TB
  A("main()") --> B{Do it forever?}
  B -- Yes --> C[Record and log temperature every 5 seconds]
  C --> D{Is there more than 12 temperatures stored?}
  D -- Yes --> E[Turn on LED0]
  E --> F[Overwrite the oldest temperatures with the newest ones] --> C
  D -- No --> C
  C --> G{Was a button pressed?}
  G -- Yes --> H[Log the average temperature]
  H --> I[Turn off LED0 if on]
  I --> J[Clear stored data] --> C
  G -- No --> C


```
`Completed:` 1/21/26 8:42pm