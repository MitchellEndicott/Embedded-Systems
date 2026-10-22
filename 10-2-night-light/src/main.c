/******************************************************
 * Night Light
 * 
 * When light drops below normal room levels,
 * the internal LED turns on for 30 seconds. An external push button can turn off the LED sooner.
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(YT_10_2, LOG_LEVEL_INF);

#define SAMPLE_INTERVAL_MS 1000
#define LED_ON_TIME_MS 30000
#define LDR_THRESHOLD_MV 1200

const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

#define LED_NI DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NI, gpios);

#define BUTTON_NI DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NI, gpios);

static bool led_active = false;
static struct k_timer led_timer;
static struct gpio_callback button_cb;

void led_timer_handler(struct k_timer *timer) {
    gpio_pin_set_dt(&led, 0);
    led_active = false;
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if(led_active) {
        gpio_pin_set_dt(&led, 0);
        led_active = false;
        k_timer_stop(&led_timer);
    }
}

void main(void) {
    int err;
    uint16_t buf;
    int val_mV;
    struct adc_sequence seq = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };
    
    if(!adc_is_ready_dt(&adc_channel)) {
        LOG_ERR("ADC not ready");
        return -1;
    }
    err = adc_channel_setup_dt(&adc_channel);
    if(err < 0) {
        LOG_ERR("ADC channel setup failed");
        return -1;
    }
    err = adc_sequence_init_dt(&adc_channel, &seq);
    if(err < 0) {
        LOG_ERR("ADC sequence init failed");
        return -1;
    }
    if(!device_is_ready(led.port)) {
        LOG_ERR("LED device not ready");
        return -1;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    
    if(!device_is_ready(button.port)) {
        LOG_ERR("Button device not ready");
        return -1;
    }
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    k_timer_init(&led_timer, led_timer_handler, NULL);

    while(true) {
        err = adc_read_dt(&adc_channel, &seq);
        if(err < 0) {
            LOG_ERR("ADC read failed");
            return -1;
        }
        val_mV = (int)buf;
        if(val_mV > LDR_THRESHOLD_MV && !led_active) {
            gpio_pin_set_dt(&led, 1);
            led_active = true;
            k_timer_start(&led_timer, K_MSEC(LED_ON_TIME_MS), K_NO_WAIT);
        }
        k_msleep(SAMPLE_INTERVAL_MS);
    }
}
