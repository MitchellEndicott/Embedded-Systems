#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(letter_counter, LOG_LEVEL_INF);

#define PORT1_NI DT_NODELABEL(gpio1)
const struct device *port = DEVICE_DT_GET(PORT1_NI);

#define LED_NI DT_ALIAS(led0)
const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NI, gpios);

#define BUTTON_NI DT_ALIAS(sw0)
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NI, gpios);

#define PHOTO_NI DT_ALIAS(photo0)
const struct gpio_dt_spec photo = GPIO_DT_SPEC_GET(PHOTO_NI, gpios);

int sevenSegConvert(int n) {
  int display;
    switch(n) {
      case 0: display = 0x3F; break;
      case 1: display = 0x06; break;
      case 2: display = 0x5B; break;
      case 3: display = 0x4F; break;
      case 4: display = 0x66; break;
      case 5: display = 0x6D; break;
      case 6: display = 0x7D; break;
      case 7: display = 0x7; break;
      case 8: display = 0x7F; break; 
      case 9: display = 0x67; break;
      default: display = 0x00; break;
    }
  return display;
}

int main(void) {
  LOG_INF("Program starting");

  int count = 0;
  int last_photo = 1;

  int pin_mask = BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7);

  for(int pin = 1; pin <= 7; pin++) {
    gpio_pin_configure(port, pin, GPIO_OUTPUT_INACTIVE);
  }
  gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
  gpio_pin_configure_dt(&photo, GPIO_INPUT | GPIO_PULL_UP);

  while(true) {
    int photo_now = gpio_pin_get_dt(&photo);

    if(last_photo == 1 && photo_now == 0) {
      count++;
    }
    last_photo = photo_now;

    gpio_port_set_masked(port, pin_mask, sevenSegConvert(count % 10) << 1);
    gpio_pin_set_dt(&led, count >= 10);
    
    if(gpio_pin_get_dt(&button) == 1) {
      LOG_INF("Total count: %d", count);
      count = 0;
      k_msleep(300);
    }
    k_msleep(10);
  }
}