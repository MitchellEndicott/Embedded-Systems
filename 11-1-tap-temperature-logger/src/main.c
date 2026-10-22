/******************************************************
 * Tap Temperature Logger
 * 
 * LED flashes until a tap interrupts,
 * then tmp36 records the temperature every 500ms and logs the average
 * LED goes back to flashing
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(YT_11_1, LOG_LEVEL_INF);

#define BLINK_PERIOD_MS 1000
#define SAMPLE_INTERVAL_MS 500
#define MAX_SAMPLES 20
#define NUM_ADC_READINGS 10

#define SINGLE_TAP (1 << 6)

#define SPIOP (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA)
struct spi_dt_spec acc = SPI_DT_SPEC_GET(DT_NODELABEL(acc), SPIOP, 0);

#define INT2_PIN 4
const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define TMP36_NODE DT_NODELABEL(temp0)
const struct adc_dt_spec tmp36 = ADC_DT_SPEC_GET(TMP36_NODE);

#define MV_AT_25C DT_PROP(TMP36_NODE, vtemp25)
#define MV_PER_DEG_C DT_PROP(TMP36_NODE, sensor_slope)

static uint16_t adc_buf[NUM_ADC_READINGS];
static float temp_buffer[MAX_SAMPLES];
static int sample_count;

static struct adc_sequence_options adc_options = {
    .extra_samplings = NUM_ADC_READINGS - 1,
    .interval_us = 100,
};

static struct adc_sequence adc_seq = {
    .options = &adc_options,
    .buffer = adc_buf,
    .buffer_size = sizeof(adc_buf),
};

static struct k_timer temp_timer;
static struct k_work adc_work;

static bool collecting_temp = false;

static int adxl_write(uint8_t reg, uint8_t val) {
    uint8_t tx[2] = { reg, val };
    struct spi_buf buf = { .buf = tx, .len = 2 };
    struct spi_buf_set tx_set = { .buffers = &buf, .count = 1 };
    return spi_write_dt(&acc, &tx_set);
}

static int adxl_read(uint8_t reg, uint8_t *val) {
    uint8_t tx[2] = { reg | 0x80, 0x00 };
    uint8_t rx[2];
    struct spi_buf txb = { tx, 2 };
    struct spi_buf rxb = { rx, 2 };
    struct spi_buf_set txs = { &txb, 1 };
    struct spi_buf_set rxs = { &rxb, 1 };

    int ret = spi_transceive_dt(&acc, &txs, &rxs);
    *val = rx[1];
    return ret;
}

void adc_work_handler(struct k_work *work) {
    int err, val_mV;
    float avg_mV = 0;
    err = adc_read_dt(&tmp36, &adc_seq);
    if(err) {
        LOG_ERR("ADC read failed");
        return;
    }
    for(int i = 0; i < NUM_ADC_READINGS; i++) {
        val_mV = adc_buf[i];
        adc_raw_to_millivolts_dt(&tmp36, &val_mV);
        avg_mV += val_mV;
    }
    avg_mV /= NUM_ADC_READINGS;
    temp_buffer[sample_count++] =
        25.0f + (avg_mV - MV_AT_25C) / MV_PER_DEG_C;
    if(sample_count == MAX_SAMPLES) {
        k_timer_stop(&temp_timer);
        float sum = 0;
        for(int i = 0; i < MAX_SAMPLES; i++) {
            sum += temp_buffer[i];
        }
        LOG_INF("Average temperature: %.1f C", sum / MAX_SAMPLES);
        sample_count = 0;
        collecting_temp = false;

    }
}

void temp_timer_handler(struct k_timer *timer) {
    k_work_submit(&adc_work);
}

int main(void) {
    LOG_INF("Program started");
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(gpio0, INT2_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    
    if(!adc_is_ready_dt(&tmp36)) {
        LOG_ERR("ADC not ready");
        return -1;
    }
    
    adc_channel_setup_dt(&tmp36);
    adc_sequence_init_dt(&tmp36, &adc_seq);
    k_work_init(&adc_work, adc_work_handler);
    k_timer_init(&temp_timer, temp_timer_handler, NULL);

    adxl_write(0x1D, 0x30);
    adxl_write(0x21, 0x30);
    adxl_write(0x22, 0x10);
    adxl_write(0x23, 0x20);
    adxl_write(0x2A, 0xFF);
    adxl_write(0x2E, 0x40); 
    adxl_write(0x2F, 0x40); 
    adxl_write(0x2D, 0x08);

    int64_t last_blink = 0;
    bool led_on = false;

    while(true) {
        if(!collecting_temp) {
            int64_t now = k_uptime_get();
            if(now - last_blink >= BLINK_PERIOD_MS / 2) {
                led_on = !led_on;
                gpio_pin_set_dt(&led, led_on);
                last_blink = now;
            }
            if(gpio_pin_get(gpio0, INT2_PIN)) {
                uint8_t src;
                adxl_read(0x30, &src);
                if(src & SINGLE_TAP && !collecting_temp) {
                    LOG_INF("Single tap detected");
                    gpio_pin_set_dt(&led, 1);
                    collecting_temp = true;
                    k_timer_start(&temp_timer, K_NO_WAIT, K_MSEC(SAMPLE_INTERVAL_MS));
                }
            }
        }
        k_msleep(10);
    }
}