/********************************
 * Letter counter
 *******************************/
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
/* Step 5.A: include the logger header */

#include <zephyr/logging/log.h>

/* Step 5.B: register with the logger module */

LOG_MODULE_REGISTER(letter_counter, LOG_LEVEL_INF);

#define BUTTON_NI DT_ALIAS(sw0)
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NI, gpios);

#define PHOTO_NI DT_ALIAS(photo0)
const struct gpio_dt_spec photo = GPIO_DT_SPEC_GET(PHOTO_NI, gpios);

int main() {
    /* Step 5.C: create variables for state of the photointerrupter */
    LOG_INF("Program starting");

    /* Step 5.D: create variable for letter count and set to 0 */

    int count = 0;

    if (gpio_is_ready_dt(&button) && gpio_is_ready_dt(&photo)) {
        gpio_pin_configure_dt(&button, GPIO_INPUT);
        gpio_pin_configure_dt(&photo, GPIO_INPUT);
    } else {
        LOG_ERR("GPIO port is not ready to configure");
        return -1;
    }

    /* Get "previous" state of photointerrupter before superloop */

    int last_photo = gpio_pin_get_dt(&photo);

    while (true) {
        /* Step 5.E: Check if photointerrupter state is unblocked
         *  after blocked. If so, increase the letter count.
         *  Send logger message with count. Update "previous state"
         */
        int photo_now = gpio_pin_get_dt(&photo);
        if(last_photo == 1 && photo_now == 0) {
            count++;
            LOG_INF("Total count: %d", count);
        }
        last_photo = photo_now;

        /* Step 5.F: Check if button is pressed. If so, zero count. */
        if(gpio_pin_get_dt(&button) == 1) {
            count = 0;
            LOG_INF("Total count: %d", count);
            k_msleep(300);
        }

        /* Step 5.G: A short sleep is necessary so logger can print. */
        k_msleep(10);
    }
}
