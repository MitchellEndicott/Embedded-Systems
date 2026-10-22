#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define FAST_SLEEP_TIME   200
#define MEDIUM_SLEEP_TIME 400
#define SLOW_SLEEP_TIME   1000

/* Get node identifiers */
#define LED0_NI DT_ALIAS(led0)
#define BTN0_NI DT_ALIAS(sw0)

/* Get gpio specs */
const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NI, gpios);
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BTN0_NI, gpios);

enum rateState_t {FastMode, MediumMode, SlowMode};

int main() {
    enum rateState_t rateMode = FastMode;
    bool prevBtn, currBtn;

    if(gpio_is_ready_dt(&led) && gpio_is_ready_dt(&button)) {
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        gpio_pin_configure_dt(&button, GPIO_INPUT);
    } 
    else {
        return -1;
    }

    prevBtn = gpio_pin_get_dt(&button);
    while(true) {
        gpio_pin_toggle_dt(&led);
        currBtn = gpio_pin_get_dt(&button);
        if((currBtn != prevBtn) && (!currBtn)) {
            switch (rateMode) {
              case FastMode: rateMode = MediumMode; break;
              case MediumMode: rateMode = SlowMode; break;
              case SlowMode: rateMode = FastMode; break;
            }
        }
        prevBtn = currBtn;
        switch(rateMode) {
            case FastMode: k_msleep(FAST_SLEEP_TIME); break;
            case MediumMode: k_msleep(MEDIUM_SLEEP_TIME); break;
            case SlowMode: k_msleep(SLOW_SLEEP_TIME); break;
        }
    }
}
