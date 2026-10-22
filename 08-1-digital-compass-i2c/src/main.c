/******************************************************
 * Digital Compass
 * 
 * reads magnetic field data from an HMC5883L using i2c,
 * prints the X, Y, and Z components. 
 * It continuously updates the readings.
 ******************************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <stdint.h>

#define HMC5883_ADDR 0x1E
#define HMC5883_MODE_REG 0x02
#define HMC5883_DATA_REG 0x03

void main(void){
    uint8_t buffer[6];
    int16_t magComponents[3];
    float magMicroT[3]; 

    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return;
    }
    
    uint8_t mode_data[2] = {HMC5883_MODE_REG, 0x00};
    int ret = i2c_write(i2c_dev, mode_data, 2, HMC5883_ADDR);
    if (ret != 0) {
        printk("Failed to set mode: %d\n", ret);
        return;
    }

    while(true) {
        ret = i2c_burst_read(i2c_dev, HMC5883_ADDR, HMC5883_DATA_REG, buffer, 6);
        if (ret != 0) {
            printk("I2C read failed: %d\n", ret);
            k_sleep(K_MSEC(500));
            continue;
        }
        magComponents[0] = (int16_t)(buffer[0] << 8 | buffer[1]);
        magComponents[1] = (int16_t)(buffer[4] << 8 | buffer[5]);
        magComponents[2] = (int16_t)(buffer[2] << 8 | buffer[3]);

        for(int i = 0; i < 3; i++) {
            magMicroT[i] = magComponents[i] * 0.092f;
        }
        printk("Magnetic field: X=%.2f uT, Y=%.2f uT, Z=%.2f uT\n",
               magMicroT[0], magMicroT[1], magMicroT[2]);

        k_sleep(K_MSEC(500));
    }
}
