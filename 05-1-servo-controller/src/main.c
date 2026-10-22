#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#define SERVO DT_NODELABEL(servo)
const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(SERVO);
#define PORT0_NI DT_NODELABEL(gpio0)
const struct device *port = DEVICE_DT_GET(PORT0_NI);

#define SERVO_MIN_PULSE_WIDTH DT_PROP(SERVO, min_pulse)
#define SERVO_MAX_PULSE_WIDTH DT_PROP(SERVO, max_pulse)
#define SERVO_CENTER DT_PROP(SERVO, center_pw)
#define SERVO_DEG_CONV DT_PROP(SERVO, deg_to_pw)

/**
 * @brief Convert angle to PWM pulse width
 * 
 * @param angle Servo angle (integer degrees)
 * @return int representing the pulse width (in nanoseconds)
 */
int angle_to_pulsewidth(int angle) {
  int pw = SERVO_CENTER + angle*SERVO_DEG_CONV;
  if (pw < SERVO_MIN_PULSE_WIDTH) {
    pw = SERVO_MIN_PULSE_WIDTH;
  }
  if (pw > SERVO_MAX_PULSE_WIDTH) {
    pw = SERVO_MAX_PULSE_WIDTH;
  }
  return pw;
}

int main(void) {
  gpio_port_value_t pin_values;
  int angle = 0;
  pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
  
  if(device_is_ready(port)) {
    gpio_pin_configure(port, 11, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    gpio_pin_configure(port, 12, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    gpio_pin_configure(port, 24, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    gpio_pin_configure(port, 25, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
  }
  else return -1;

  while(true) {
    gpio_port_get(port, &pin_values);
    if (angle > 90) angle = 90;
    if (angle < -90) angle = -90;

    if((pin_values & BIT(11))) {
      angle -= 10;
      pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
    }
    if((pin_values & BIT(12))) {
      angle += 10;
      pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
    }
    if((pin_values & BIT(24))) { 
      angle = 0;
      pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
    }
    if((pin_values & BIT(25))) {
      if(angle < 0) {
        angle = -90;
        pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
      }
      else {
        angle = 90;
        pwm_set_pulse_dt(&servo, angle_to_pulsewidth(angle));
      }
    }
    k_msleep(200);
  }
}