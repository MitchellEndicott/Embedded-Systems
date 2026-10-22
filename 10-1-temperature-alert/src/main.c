/******************************************************
 * Temperature Alert
 * 
 * LED4 flashes continuously, If the temperature goes above 27°C, LED1 turns on. 
 * When the temperature drops below 27­° C, LED2 turns on and LED1 turns off. 
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(YT_10_1, LOG_LEVEL_INF);

#define TMP36_NODE DT_NODELABEL(temp0)
static const struct adc_dt_spec adc_spec = ADC_DT_SPEC_GET(TMP36_NODE);

#define TMP36_V25_MV 750
#define TMP36_SLOPE 10
#define TEMP_THRESHOLD_C 27.0f
#define LED1_NI DT_ALIAS(led0)
#define LED2_NI DT_ALIAS(led1) 
#define LED4_NI DT_ALIAS(led3)

static const struct gpio_dt_spec led_hot   = GPIO_DT_SPEC_GET(LED1_NI, gpios);
static const struct gpio_dt_spec led_was_hot = GPIO_DT_SPEC_GET(LED2_NI, gpios);
static const struct gpio_dt_spec led_blink = GPIO_DT_SPEC_GET(LED4_NI, gpios);
static int16_t sample_buffer;
static bool above_threshold = false;
static bool was_too_hot = false;

void main(void) {
    int ret;

    if(!adc_is_ready_dt(&adc_spec)) {
        LOG_ERR("ADC not ready");
        return -1;
    }

    const struct gpio_dt_spec leds[] = {led_hot, led_was_hot, led_blink};
    for(int i = 0; i < 3; i++) {
        if(!device_is_ready(leds[i].port)) {
            LOG_ERR("LED %d not ready", i);
            return -1;
        }
        gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT);
        gpio_pin_set_dt(&leds[i], 0);
    }

    ret = adc_channel_setup_dt(&adc_spec);
    if(ret) {
        LOG_ERR("ADC channel setup failed (%d)", ret);
        return -1;
    }

    struct adc_sequence sequence = {
        .buffer = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
    };

    int led4_timer = 0;
    bool led4_state = false;

    while(true) {
        led4_timer += 50;
        if (led4_timer >= 250) {
            led4_state = !led4_state;
            gpio_pin_set_dt(&led_blink, led4_state ? 1 : 0);
            led4_timer = 0;
        }
        ret = adc_sequence_init_dt(&adc_spec, &sequence);
        if(ret) {
            LOG_ERR("ADC sequence init failed (%d)", ret);
            k_msleep(50);
            continue;
        }
        ret = adc_read(adc_spec.dev, &sequence);
        if(ret) {
            LOG_ERR("ADC read failed (%d)", ret);
            k_msleep(50);
            continue;
        }
        int32_t mv = sample_buffer;
        adc_raw_to_millivolts_dt(&adc_spec, &mv);
        float temp_c = (mv - TMP36_V25_MV) / TMP36_SLOPE + 25.0f;
        LOG_INF("Temperature: %.2f C", (double)temp_c);
        if(!above_threshold && temp_c >= TEMP_THRESHOLD_C) {
            above_threshold = true;
            gpio_pin_set_dt(&led_hot, 1);
            LOG_INF("Temp above threshold");
        } 
        else if (above_threshold && temp_c < TEMP_THRESHOLD_C) {
            above_threshold = false;
            gpio_pin_set_dt(&led_hot, 0); 
            gpio_pin_set_dt(&led_was_hot, 1); 
            was_too_hot = true;
            LOG_INF("Temp below threshold");
        }
        k_msleep(50);
    }
}
