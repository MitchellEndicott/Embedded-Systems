#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Lab05_Exercise1, LOG_LEVEL_INF);

#define PWM_LED_NI DT_NODELABEL(pwm_ext_led)
const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(PWM_LED_NI);

#define PORT0_NI DT_NODELABEL(gpio0)
const struct device *port = DEVICE_DT_GET(PORT0_NI);

int PWM_PERIOD_MS = 35;
int PWM_PERIOD_NS = 35000000;

int main(void) {
  int err;
  gpio_port_value_t pin_values;

  if(device_is_ready(port)) {
    gpio_pin_configure(port, 11, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    gpio_pin_configure(port, 12, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    gpio_pin_configure(port, 24, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
  }
  else return -1;
    
  if(!pwm_is_ready_dt(&pwm_led)) {
    LOG_ERR("Error: PWM device is not ready");
    return -1;
  }
  err = pwm_set_dt(&pwm_led, PWM_PERIOD_NS, PWM_PERIOD_NS);
  if(err) {
    LOG_ERR("Error %d in pwm_set_dt()", err);
    return -1;
  }
  LOG_INF("PWM period is %d ms",PWM_PERIOD_MS);
  k_msleep(1000);

  int brightness = 50;
  int saved_brightness = 50;
  bool led_on = true;
  bool b1_last = false;
  bool b2_last = false;
  bool b3_last = false;

  while (true) {
      gpio_port_get(port, &pin_values);
      bool b1_now = !(pin_values & BIT(11));
      bool b2_now = !(pin_values & BIT(12));
      bool b3_now = !(pin_values & BIT(24));
      if (b1_now && !b1_last && brightness < 100) {
          brightness += 10;
      }
      if (b2_now && !b2_last && brightness > 0) {
          brightness -= 10;
      }
      if (b3_now && !b3_last) {
          if (led_on) {
              saved_brightness = brightness;
              brightness = 0;
              led_on = false;
          } else {
              brightness = saved_brightness;
              led_on = true;
          }
      }
      b1_last = b1_now;
      b2_last = b2_now;
      b3_last = b3_now;
      uint32_t pulse = (PWM_PERIOD_NS * brightness) / 100;
      pwm_set_dt(&pwm_led, PWM_PERIOD_NS, pulse);

      k_msleep(50);
  }
}