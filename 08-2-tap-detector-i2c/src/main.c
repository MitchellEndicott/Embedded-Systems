/******************************************************
 * Tap Detector
 *
 * Detects single and double taps using an accelerometer,
 * LED1 lights for a single tap, and LED2 lights for a double tap.
 *****************************************************/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(YT_8_1, LOG_LEVEL_INF);

#define ACC_NODE DT_NODELABEL(acc)
static const struct i2c_dt_spec acc = I2C_DT_SPEC_GET(ACC_NODE);

#define SINGLE_TAP (1 << 6)
#define DOUBLE_TAP (1 << 5)
#define INT2_PORT DT_NODELABEL(gpio0)
#define INT2_PIN  2
static const struct device *gpio0 = DEVICE_DT_GET(INT2_PORT);

#define LED1_NI DT_ALIAS(led0)
#define LED2_NI DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NI, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NI, gpios);

static int adxl_write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val };
    return i2c_write_dt(&acc, buf, sizeof(buf));
}

static int adxl_read(uint8_t reg, uint8_t *val) {
    return i2c_write_read_dt(&acc, &reg, 1, val, 1);
}

void main(void) {
    if(!i2c_is_ready_dt(&acc)) {
        LOG_ERR("I2C not ready");
        return;
    }
    if(!device_is_ready(gpio0)) {
        LOG_ERR("GPIO0 not ready");
        return;
    }
    if(!device_is_ready(led1.port) || !device_is_ready(led2.port)) {
        LOG_ERR("LEDs not ready");
        return;
    }

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(gpio0, INT2_PIN, GPIO_INPUT);

    adxl_write(0x1D, 0x30);
    adxl_write(0x21, 0x10);
    adxl_write(0x22, 0x30); 
    adxl_write(0x23, 0xA0); 
    adxl_write(0x2A, 0x07);  
    adxl_write(0x2E, 0x60); 
    adxl_write(0x2F, 0x60); 
    adxl_write(0x2D, 0x08);

    LOG_INF("Program Started");

    while(true) {
        if(gpio_pin_get(gpio0, INT2_PIN) == 1) {
            uint8_t int_source;
            adxl_read(0x30, &int_source);
            if(int_source & DOUBLE_TAP) {
                LOG_INF("Double tap detected");
                gpio_pin_set_dt(&led2, 1);
                k_msleep(1000);
                gpio_pin_set_dt(&led2, 0);
            }
            else if (int_source & SINGLE_TAP) {
                LOG_INF("Single tap detected");
                gpio_pin_set_dt(&led1, 1);
                k_msleep(1000);
                gpio_pin_set_dt(&led1, 0);
            }
            k_msleep(10);
        }
    
        k_msleep(10);
    }
}
