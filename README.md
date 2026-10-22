# Embedded Systems Projects

Fifteen firmware projects written in C with Zephyr RTOS for the Nordic nRF52840 DK. They read sensors, drive LEDs, a servo and a display, and communicate over GPIO, PWM, ADC, I²C and SPI. I completed them for my embedded systems coursework at Whitworth University in January 2026.

## Projects

| # | Project | Concepts | Demo |
|---|---------|----------|------|
| [01-1](01-1-led-flash-counts/) | **LED Flash Counts**: Flashes LED1 ten times, then LED2 five times, repeating forever. | GPIO output, timing | |
| [01-2](01-2-two-hearts/) | **Two Hearts**: Two LEDs flash in alternating bursts, one speeding up and the other slowing down. | GPIO output, timing | |
| [02-1](02-1-three-rates-one-heart/) | **Three Rates, One Heart**: A button cycles an LED between fast, medium and slow flash rates. | GPIO input and output, state handling | |
| [03-1](03-1-letter-counter/) | **Letter Counter**: Counts letters passing through a photointerrupter and logs the count; a button resets it. | GPIO input, logging | |
| [04-1](04-1-letter-counter-seven-segment/) | **Letter Counter, Seven-Segment Display**: The letter counter shown on a seven-segment display, with an LED marking counts above 10. | GPIO output, display driving, logging | |
| [05-1](05-1-servo-controller/) | **Servo Controller**: Four buttons move a servo to set angles. | PWM, button input | |
| [05-2](05-2-led-dimmer/) | **LED Dimmer**: Buttons change an external LED in 10% brightness steps and toggle it on or off, remembering the level. | PWM, button input | |
| [07-1](07-1-tap-detector-spi/) | **Tap Detector (SPI)**: An ADXL345 accelerometer detects single and double taps; each lights a different LED. | SPI, accelerometer registers | |
| [08-1](08-1-digital-compass-i2c/) | **Digital Compass (I²C)**: Reads an HMC5883L compass and logs the X, Y and Z magnetic field in µT. | I²C, sensor data conversion | |
| [08-2](08-2-tap-detector-i2c/) | **Tap Detector (I²C)**: The tap detector rebuilt with the ADXL345 on I²C. | I²C, accelerometer registers | |
| [09-1](09-1-analog-temperature-logger/) | **Analog Temperature Logger**: Samples a TMP36 every five seconds into a 12-reading buffer; a button logs the average and clears it. | ADC, circular buffer | |
| [10-1](10-1-temperature-alert/) | **Temperature Alert**: TMP36 with a TLC272 comparator circuit; LEDs show too hot and was too hot. | ADC, analog circuit design | |
| [10-2](10-2-night-light/) | **Night Light**: A light sensor turns on an LED for 30 seconds in low light; a button turns it off early. | ADC, GPIO | |
| [11-1](11-1-tap-temperature-logger/) | **Tap Temperature Logger**: A tap triggers 20 TMP36 readings, then the average is logged. | SPI, ADC, GPIO | |
| [12-1](12-1-tap-temperature-logger-i2c/) | **Tap Temperature Logger (I²C Sensor)**: Single tap queues a TMP102 reading (up to 20); double tap logs the average; a button cycles C, K, F and R. | SPI, I²C, queue | |

Each folder is a standalone Zephyr application with its own README that covers the purpose, devicetree and kernel configuration, wiring, and a flowchart.

## Hardware

- Nordic nRF52840 DK
- ADXL345 accelerometer, HMC5883L digital compass, TMP36 and TMP102 temperature sensors
- Isocom H21A1 photointerrupter, light-dependent resistor, TLC272 op-amp
- Hitec HS-422 servo, LIGITEK LSD3211 seven-segment display

## Building

Open a project folder with the nRF Connect extension for VS Code (or use `west build` for the nRF52840 DK), then build and flash to the board. Build output goes in `build*/`, which is excluded by each project's `.gitignore`.

## Author

Mitchell Endicott
