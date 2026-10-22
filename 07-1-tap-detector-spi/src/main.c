/******************************************************
 * Tap Detector
 * 
 * The ADXL345 will detect if there is a sinlge tap or a double tap, and light up LED1 or LED2 accordingly, 
 * and log which event happened 
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>

// Define the SPI device

#define CHILL_TIME 200
LOG_MODULE_REGISTER(Lab7B, LOG_LEVEL_INF);

#define SPIOP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA)
struct spi_dt_spec acc = SPI_DT_SPEC_GET(DT_NODELABEL(acc), SPIOP, 0);

// Define the GPIO input (connected to INT2)

#define INT2_NI DT_NODELABEL(gpio0)
const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
#define INT2_PIN 4
#define SINGLE_TAP (1 << 6)
#define DOUBLE_TAP (1 << 5)


// Define the LED pins

#define LED0_NI DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NI, gpios);

static int adxl_write(uint8_t reg, uint8_t val)
{
    uint8_t tx_buf[2] = { reg, val };
    struct spi_buf buf = { .buf = tx_buf, .len = 2 };
    struct spi_buf_set tx = { .buffers = &buf, .count = 1 };

    return spi_write_dt(&acc, &tx);
}

static int adxl_read(uint8_t reg, uint8_t *val) {
    uint8_t tx_buf[2] = {reg | 0x80, 0x00};
    uint8_t rx_buf[2];

    struct spi_buf txb = {.buf = tx_buf, .len = 2};
    struct spi_buf rxb = {.buf = rx_buf, .len = 2};

    struct spi_buf_set tx = {.buffers = &txb, .count = 1};
    struct spi_buf_set rx = {.buffers = &rxb, .count = 1};

    int ret = spi_transceive_dt(&acc, &tx, &rx);
    *val = rx_buf[1];
    return ret;
}


int main(void) {
    // Define variables
    
    if(!spi_is_ready_dt(&acc)) {
        LOG_ERR("SPI device not ready");
        return -1;
    }

    if(!device_is_ready(gpio0)) {
        LOG_ERR("GPIO0 not ready");
        return -1;
    }
    /********************************************************
     * Configure the accelerometer for tap detection
    ********************************************************/
    
    adxl_write(0x1D, 0x30); 
    adxl_write(0x21, 0x30);
    adxl_write(0x22, 0x10);
    adxl_write(0x23, 0x20); 
    adxl_write(0x2A, 0xFF);
    adxl_write(0x2E, 0x60);
    adxl_write(0x2F, 0x60);

    // Configure the GPIO input

    gpio_pin_configure(gpio0, INT2_PIN, GPIO_INPUT | GPIO_PULL_DOWN);

    // Configure the LED pins (initialize to off)

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    // Put the accelerometer in measurement mode

    adxl_write(0x2D, 0x08);

    while(true) {
        if(gpio_pin_get(gpio0, INT2_PIN) == 1) {
            uint8_t int_source;
            adxl_read(0x30, &int_source);
            if(int_source & DOUBLE_TAP) {
                LOG_INF("Double tap detected");
                gpio_pin_set_dt(&led0, 1);
                k_msleep(1000);
                gpio_pin_set_dt(&led0, 0);
            } 
            else if(int_source & SINGLE_TAP) {
                LOG_INF("Single tap detected");
                gpio_pin_set_dt(&led0, 1);
                k_msleep(1000);
                gpio_pin_set_dt(&led0, 0);
            }
        }
        k_msleep(10);
    }
}
