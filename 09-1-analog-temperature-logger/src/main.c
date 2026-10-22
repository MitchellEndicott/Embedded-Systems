/******************************************************
 * Analog temperature logger
 * 
 * TMP36 gets the temperature every 5 seconds, once there are 12 pieces of data stored,
 * an internal led lights up to let you know the buffer is full, and each new data overwrites the oldest data.
 * an internal button logs the average temperature of the stored data, and clears the buffer
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(AnalogTemp, LOG_LEVEL_INF);

#define NUM_ADC_READINGS 10
#define SAMPLE_INTERVAL_MS 5000
#define MAX_SAMPLES 12

#define TMP36 DT_NODELABEL(temp0)
const struct adc_dt_spec tmp36 = ADC_DT_SPEC_GET(TMP36);

#define MV_AT_25C DT_PROP(TMP36, vtemp25)
#define MV_PER_DEG_C DT_PROP(TMP36, sensor_slope)

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static uint16_t adc_buf[NUM_ADC_READINGS];
static struct adc_sequence_options adc_options = {.extra_samplings = NUM_ADC_READINGS - 1, .interval_us = 100,};
static struct adc_sequence adc_seq = {.options = &adc_options, .buffer = adc_buf, .buffer_size = sizeof(adc_buf),};

static float temp_buffer[MAX_SAMPLES];
static int sample_count = 0;
static int write_index = 0;
static struct k_timer temp_timer;
static struct k_work adc_work;

void adc_work_handler(struct k_work *work) {
    int err;
    int val_mV;
    float avg_mV = 0.0f;
    float temp_c;

    err = adc_read_dt(&tmp36, &adc_seq);
    if(err < 0) {
        LOG_ERR("ADC read failed (%d)", err);
        return;
    }
    for(int i = 0; i < NUM_ADC_READINGS; i++) {
        val_mV = adc_buf[i];
        adc_raw_to_millivolts_dt(&tmp36, &val_mV);
        avg_mV += val_mV;
    }
    avg_mV /= NUM_ADC_READINGS;
    temp_c = 25.0f + (avg_mV - MV_AT_25C) / MV_PER_DEG_C;
    temp_buffer[write_index] = temp_c;
    write_index = (write_index + 1) % MAX_SAMPLES;
    if(sample_count < MAX_SAMPLES) {
        sample_count++;
        if(sample_count == MAX_SAMPLES) {
            gpio_pin_set_dt(&led, 1);
        }
    }
    LOG_INF("Recorded: %.1f C", temp_c);
}

void temp_timer_handler(struct k_timer *timer) {
    k_work_submit(&adc_work);
}

int main(void) {
    bool button_prev = true;
    LOG_INF("Program started");
    if(!adc_is_ready_dt(&tmp36)) {
        LOG_ERR("ADC not ready");
        return -1;
    }

    adc_channel_setup_dt(&tmp36);
    adc_sequence_init_dt(&tmp36, &adc_seq);

    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    k_work_init(&adc_work, adc_work_handler);

    k_timer_init(&temp_timer, temp_timer_handler, NULL);
    k_timer_start(&temp_timer, K_NO_WAIT, K_MSEC(SAMPLE_INTERVAL_MS));

    while(true) {
        bool button_now = gpio_pin_get_dt(&button);
        if(button_prev && !button_now) {
            float sum = 0.0f;
            LOG_INF("Stored temperatures:");
            for(int i = 0; i < sample_count; i++) {
                LOG_INF("Sample %d: %.1f C", i + 1, temp_buffer[i]);
                sum += temp_buffer[i];
            }
            if(sample_count > 0) {
                LOG_INF("Average: %.1f C", sum / sample_count);
            }
            sample_count = 0;
            write_index = 0;
            gpio_pin_set_dt(&led, 0);
        }
        button_prev = button_now;
        k_msleep(20);
    }
}
