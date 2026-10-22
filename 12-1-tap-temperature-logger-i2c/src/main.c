/******************************************************
 * Tap Temperature Logger
 * 
 * Waits for single and double taps from an accelerometer, 
 * on a single tap it reads the temperature and stores it in a queue (max 20),
 * on a double tap, calculates and logs the average of the stored temperatures.
 * A button cycles the displayed units
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(YT_12_1, LOG_LEVEL_INF);

#define TMP_NODE DT_NODELABEL(tempsensor)
#define SINGLE_TAP (1 << 6)
#define DOUBLE_TAP (1 << 5)
#define QUEUE_SIZE 20

const struct i2c_dt_spec tempSensor = I2C_DT_SPEC_GET(TMP_NODE);

#define SPIOP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA)
struct spi_dt_spec acc = SPI_DT_SPEC_GET(DT_NODELABEL(acc), SPIOP, 0);

#define INT2_PORT DT_NODELABEL(gpio0)
#define INT2_PIN 4
const struct device *gpio0 = DEVICE_DT_GET(INT2_PORT);

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb;

K_MUTEX_DEFINE(i2c_mutex);

K_MSGQ_DEFINE(temp_msgq, sizeof(float), QUEUE_SIZE, 4);

#define STACK_SIZE 1024
#define PRIORITY 5

static int unit_mode = 0;

static int adxl_write(uint8_t reg, uint8_t val) {
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

float convert_temp(float c) {
    if(unit_mode == 1) {
        return c + 273.15f;
    }
    if(unit_mode == 2) {
        return (c * 9.0f / 5.0f) + 32.0f;
    }
    if(unit_mode == 3) {
        return (c + 273.15f) * 9.0f / 5.0f;
    }
    return c;
}

const char *unit_string(void)
{
    if(unit_mode == 1) {
        return "K";
    }
    if(unit_mode == 2) {
        return "F";
    }
    if(unit_mode == 3) {
        return "R";
    }
    return "C";
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    unit_mode = (unit_mode + 1) % 4;
    LOG_INF("Units set to %s", unit_string());
}

float read_temperature_c(void)
{
    uint8_t reg = 0x00;
    uint8_t buf[2];
    int16_t raw;
    float temp;
    k_mutex_lock(&i2c_mutex, K_FOREVER);
    i2c_write_read_dt(&tempSensor, &reg, 1, buf, 2);
    k_mutex_unlock(&i2c_mutex);
    raw = (buf[0] << 4) | (buf[1] >> 4);
    temp = raw * 0.0625f;
    return temp;
}

void average_and_clear_queue(void) {
    float sum = 0.0f;
    float temp;
    int count = 0;
    while(k_msgq_get(&temp_msgq, &temp, K_NO_WAIT) == 0) {
        sum += temp;
        count++;
    }
    if(count == 0) {
        LOG_INF("No data");
        return;
    }
    float avg_c = sum / count;
    float avg = convert_temp(avg_c);
    LOG_INF("Average temperature: %.2f %s (%d samples)", avg, unit_string(), count);
}

void tap_thread(void) {
    uint8_t int_source;
    while(true) {
        if(gpio_pin_get(gpio0, INT2_PIN)) {
            adxl_read(0x30, &int_source);
            if(int_source & DOUBLE_TAP) {
                LOG_INF("Double tap detected");
                average_and_clear_queue();
            }
            else if(int_source & SINGLE_TAP) {
                float temp = read_temperature_c();
                if(k_msgq_put(&temp_msgq, &temp, K_NO_WAIT) != 0) {
                    float old;
                    k_msgq_get(&temp_msgq, &old, K_NO_WAIT);
                    k_msgq_put(&temp_msgq, &temp, K_NO_WAIT);
                }
                LOG_INF("Stored temp: %.2f %s, Queue count: %d", convert_temp(temp), unit_string(), k_msgq_num_used_get(&temp_msgq));
            }
        }
        k_sleep(K_MSEC(10));
    }
}

K_THREAD_DEFINE(tap_thread_id, STACK_SIZE, tap_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void) {
    if(!i2c_is_ready_dt(&tempSensor)) {
        LOG_ERR("I2C not ready");
        return -1;
    }
    if(!spi_is_ready_dt(&acc)) {
        LOG_ERR("SPI not ready");
        return -1;
    }
    if(!device_is_ready(gpio0)) {
        LOG_ERR("GPIO not ready");
        return -1;
    }

    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb);

    adxl_write(0x1D, 0x50); 
    adxl_write(0x21, 0x10); 
    adxl_write(0x22, 0x05); 
    adxl_write(0x23, 0xFF); 
    adxl_write(0x2A, 0x07); 
    adxl_write(0x2E, 0x60); 
    adxl_write(0x2F, 0x60);

    adxl_write(0x2D, 0x08);

    gpio_pin_configure(gpio0, INT2_PIN, GPIO_INPUT | GPIO_PULL_DOWN);

    LOG_INF("Program started");

    while(true) {
        k_sleep(K_FOREVER);
    }
}
