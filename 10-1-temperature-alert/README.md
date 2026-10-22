# Program: Temperature Alert
**Author(s):** Mitchell Endicott
**Date:** 1/22/26

**Modified by:** Mitchell Endicott
**Date:** 1/22/26

**Purpose:** This program monitors a TMP36 temperature sensor and flashes LED4 continuously. If the temperature exceeds 27°C, LED1 turns on. When the temperature drops below the threshold, LED2 turns on and LED1 turns off. LED1 indicates that the temperature is currently too hot and LED2 indicates the temperature was too hot.

## Configuration

Devicetree overlay:

* ADC enabled for TMP36 analog input
* TMP36 configured as an ADC channel on AIN1

Kernel configuration via `prj.conf`:

* Log
* GPIO
* ADC
* Float point support

## Hardware
### External

* TMP36 temperature sensor connected to TLC272
* TLC272 op-amp comparator circuit for temperature threshold
* Resister circut 

### Internal

* LED0
* LED1
* LED3

## Flow
```mermaid
graph LR
  A("main()") --> B{Do it forever?}
  B  -- Yes --> C[Toggle LED4 every 0.25s]
  C --> D{Is the temp >= 27 deg C?}
  D -- Yes --> E[Turn on LED1] --> D
  D -- No --> F[Turn off LED1, turn on LED2 if was >= 27 deg C] --> D
  ```
`Completed:` 1/22/26 11:46pm