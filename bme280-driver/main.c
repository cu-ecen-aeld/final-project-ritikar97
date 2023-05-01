/**
 * @file    bme280.c
 * @brief   Functions and data related to the BME280 driver implementation
 *
 * @author  Ritika Ramchandani
 * @date    2023-04-12
 * 
 * @ref    BME280 GitHub repo by Bosch Sensortec GmbH
 *         BME280 Datasheet 
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "bme280.h"
#include <linux/slab.h>	// kmalloc, krealloc, kfree

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#define CALIB_ADDR (0x88)
#define CALIB_DATA_T_LEN (6)
#define TEMP_REG_ADDR (0xFA)
#define PRESSURE_REG_ADDR (0xF7)
#define BME280_SENSOR_ADDR (0x77)
#define BME280_STATUS_REG_ADDR (0xF3)
#define BME280_CHIP_ID_REG_ADDR (0xD0)
#define BME280_CHIP_ID (0x60)
#define BME280_CONFIG_REG_ADDR (0xF5)
#define BME280_CTRL_MEAS_REG_ADDR (0xF4)
#define IIR_FILTER_BIT_MASK (0x1C)
#define MODE_LSB (0)
#define OSRS_P_LSB (2) 
#define OSRS_T_LSB (5)
#define SLEEP_MASK (0xFC)
#define MEASUREMENT_IN_PROGRESS (0x00001001)
#define P_CALC (128000)
#define P_CALC_2 (1048576)


int bme280_major =   0; // use dynamic major
int bme280_minor =   0;

MODULE_AUTHOR("Ritika Ramchandani"); 
MODULE_LICENSE("Dual BSD/GPL");

struct bme280_dev bme280_device;
long signed int tFine;

// Define and initialize the i2c_driver struct
static struct i2c_driver bme280_i2c_driver = 
{
    .driver = {
        .name = "bme280",
        .owner = THIS_MODULE,                  
    },
};


// Initialize I2C board info for BME280 device
struct i2c_board_info bme280_i2c_board = 
{
    .type = "bme280",               // Set device type to "bme280"
    .addr = BME280_SENSOR_ADDR,     // Set I2C address of the BME280 device
    .flags = 0,                     // Set flags to 0, or specify any required flags
    .platform_data = NULL,          // Set platform-specific data, if applicable
};


// Function to open the device
int bme280_open(struct inode *inode, struct file *filp)
{
    struct bme280_dev* dev;
    PDEBUG("open");

    dev = container_of(inode -> i_cdev, struct bme280_dev, cdev);

    filp -> private_data = dev;

    return 0;
}


// Function to close the device
int bme280_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    return 0;
}


// Each measurement - temperature, pressure and humidity, requires certain calibration data already present in the registers
static void get_calibration_data(void)
{
    int ret_val = 0;

    // Read preset calibration data
    ret_val = i2c_smbus_read_i2c_block_data(bme280_device.bme280_i2c_client, CALIB_ADDR, CALIB_DATA_PT_LEN, &bme280_device.calib_data[0]);
    if(ret_val != CALIB_DATA_PT_LEN)
    {
        printk(KERN_ERR "Error reading calib data = %d\n", ret_val);
    }
}

// Read the pressure
static long unsigned int bme280_pressure_read(void)
{
    long long signed int var1, var2, P;
    long signed int adc_P = 0;
    unsigned short dig_P1_val;
    signed short dig_P2_val, dig_P3_val, dig_P4_val, dig_P5_val, dig_P6_val, dig_P7_val, dig_P8_val, dig_P9_val;

    // pres_msb - [19:12], pres_lsb - [11:4], pres_xlsb, [3:0]
    adc_P |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, PRESSURE_REG_ADDR) << 12);
    adc_P |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, PRESSURE_REG_ADDR + 1) << 4);
    adc_P |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, PRESSURE_REG_ADDR + 2) >> 4);

    // Concatenate bytes read from calibration registers to get calibration digits
    // Reference: GitHub repo of Bosch Sensortec
    dig_P1_val = (((bme280_device.calib_data[(dig_P1 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P1 << 1)]));
    dig_P2_val = (((bme280_device.calib_data[(dig_P2 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P2 << 1)]));
    dig_P3_val = (((bme280_device.calib_data[(dig_P3 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P3 << 1)]));
    dig_P4_val = (((bme280_device.calib_data[(dig_P4 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P4 << 1)]));
    dig_P5_val = (((bme280_device.calib_data[(dig_P5 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P5 << 1)]));
    dig_P6_val = (((bme280_device.calib_data[(dig_P6 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P6 << 1)]));
    dig_P7_val = (((bme280_device.calib_data[(dig_P7 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P7 << 1)]));
    dig_P8_val = (((bme280_device.calib_data[(dig_P8 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P8 << 1)]));
    dig_P9_val = (((bme280_device.calib_data[(dig_P9 << 1) + 1]) << 8) | (bme280_device.calib_data[(dig_P9 << 1)]));


    // Reference: BME280 datasheet
    var1 = ((long long signed int)tFine) - P_CALC;
    var2 = var1 * var1 * ((long long signed int)dig_P6_val);
    var2 = var2 + ((var1 * (long long signed int)dig_P5_val) << 17);
    var2 = var2 + (((long long signed int)dig_P4_val) << 35);
    var1 = ((var1 * var1 * (long long signed int)dig_P3_val) >> 8) + ((var1 * (long long signed int)dig_P2_val) << 12);
    var1 = (((((long long signed int)1) << 47) + var1)) * ((long long signed int)dig_P1_val) >> 33;

    // Check for zero before dividing
    if (var1 == 0)
    {
        printk(KERN_ERR "Value of tFine = %lu and var1 = %lld", tFine, var1);
        return -1;
    }
    
    P = P_CALC_2 - adc_P;
    P = (((P << 31) - var2) * 3125);
    do_div(P, var1);
    var1 = (((long long signed int)dig_P9_val) * (P >> 13) * (P >> 13)) >> 25;
    var2 = (((long long signed int)dig_P8_val) * P) >> 19;
    P = ((P + var1 + var2) >> 8) + (((long long signed int)dig_P7_val) << 4);

    return (long unsigned int)P;

}


static long signed int bme280_temp_read(void)
{
    long signed int adc_T = 0, var1, var2, T;
    unsigned short dig_T1_val;
    signed short dig_T2_val, dig_T3_val;

    // temp_msb - [19:12], temp_lsb - [11:4], temp_xlsb, [3:0]
    adc_T |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, TEMP_REG_ADDR) << 12);
    adc_T |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, TEMP_REG_ADDR + 1) << 4);
    adc_T |= (i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, TEMP_REG_ADDR + 2) >> 4);

    dig_T1_val = (((bme280_device.calib_data[dig_T1 + 1]) << 8) | (bme280_device.calib_data[dig_T1]));
    dig_T2_val = (((bme280_device.calib_data[(1 << dig_T2) + 1]) << 8) | (bme280_device.calib_data[(1 << dig_T2)]));
    dig_T3_val = (((bme280_device.calib_data[(1 << dig_T3) + 1]) << 8) | (bme280_device.calib_data[(1 << dig_T3)]));

    // Compensation for possible errors in sensor data
    // Reference for logic: BME280 Datasheet
    var1 = (((adc_T >> 3) - ((long signed int) dig_T1_val << 1)) * ((long signed int) dig_T2_val)) >> 11;
    var2 = (((((adc_T >> 4) - ((long signed int) dig_T1_val)) * ((adc_T >> 4) - ((long signed int) dig_T1_val))) >> 12) *
        ((long signed int) dig_T3_val)) >> 14;

    tFine = var1 + var2;

    printk(KERN_DEBUG "Value of tFine = %ld\n", tFine);
    T = (tFine * 5 + 128) >> 8;
    return T;
}


// Read the values from the sensor for temperature and pressure
ssize_t bme280_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    long signed int bme280_temperature_val;
    long unsigned int bme280_pressure_val;
    ssize_t num_bytes_read = MEASUREMENT_LEN;
    ssize_t retval = 0;
    int rmw_val;
    char measurements[MEASUREMENT_LEN];

    // Force a reading
    rmw_val = i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, BME280_CTRL_MEAS_REG_ADDR);
    retval = i2c_smbus_write_byte_data(bme280_device.bme280_i2c_client, BME280_CTRL_MEAS_REG_ADDR, (rmw_val | (1 << MODE_LSB)));

    if(retval < 0)
    {
        printk(KERN_ERR "Coudn't write data to force reading Result = %ld\n", retval);
        return -1;
    }

    // Wait while a measurement is in progress
    while(!(i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, BME280_STATUS_REG_ADDR) & MEASUREMENT_IN_PROGRESS));

    bme280_temperature_val = bme280_temp_read();


    // Read Pressure value
    bme280_pressure_val = bme280_pressure_read();
    if(bme280_pressure_val == -1)
    {
        printk(KERN_ERR "Coudn't read pressure value. Error = %ld\n", bme280_pressure_val);
        return 0;
    }

    snprintf(measurements, sizeof(measurements), "%ld %lu", bme280_temperature_val, bme280_pressure_val);


    // Copy the entry from the specified offset to the user-provided buffer
    if(copy_to_user(buf, measurements, MEASUREMENT_LEN))
    {
        retval = -EFAULT;
        goto exit_gracefully;
    }

    PDEBUG("Number of bytes read = %ld\n", num_bytes_read);

    retval = num_bytes_read;

exit_gracefully:
    
    return retval;
}


struct file_operations bme280_fops = {
    .owner =            THIS_MODULE,
    .read =             bme280_read,
    .open =             bme280_open,
    .release =          bme280_release,
};


static int bme280_setup_cdev(struct bme280_dev *dev)
{
    int err, devno = MKDEV(bme280_major, bme280_minor);

    cdev_init(&dev->cdev, &bme280_fops);
    dev -> cdev.owner = THIS_MODULE;
    dev -> cdev.ops = &bme280_fops;
    err = cdev_add (&dev -> cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding bme280 cdev", err);
    }
    return err;
}


static uint8_t bme280_init_sensor(void)
{
    int retval = 0;
    int result = 0;
    uint8_t rmw_val = 0;

    int8_t chip_id = i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, BME280_CHIP_ID_REG_ADDR);

    if(chip_id != BME280_CHIP_ID)
    {
        printk(KERN_ERR "Incorrect chip ID. Expected = %d, recieved = %d\n", BME280_CHIP_ID, chip_id);
        retval = -1;
        goto exit_sensor_init;
    }

    // Table 7 in datasheet (suggested settings for weather monitoring)
    // turn off IIR filter
    rmw_val = i2c_smbus_read_byte_data(bme280_device.bme280_i2c_client, BME280_CONFIG_REG_ADDR);
    result = i2c_smbus_write_byte_data(bme280_device.bme280_i2c_client, BME280_CONFIG_REG_ADDR, (rmw_val & ~IIR_FILTER_BIT_MASK));
    if(result < 0)
    {
        printk(KERN_ERR "Coudn't write data to turn off IIR filter. Result = %d\n", result);
        retval = -1;
        goto exit_sensor_init;
    }

    // Set oversampling rate
    // Temp x 1
    // Pressure x 1
    // Mode = SLEEP initially
    rmw_val = 0;
    rmw_val |= ((1 << OSRS_T_LSB) | (1 << OSRS_P_LSB));
    rmw_val &= ~SLEEP_MASK;
    result = i2c_smbus_write_byte_data(bme280_device.bme280_i2c_client, BME280_CTRL_MEAS_REG_ADDR, rmw_val);

    if(result < 0)
    {
        printk(KERN_ERR "Coudn't write data to set oversampling rate. Result = %d\n", result);
        retval = -1;
        goto exit_sensor_init;
    }
    

    // Get calibration data from NVM
    get_calibration_data();

exit_sensor_init:
    return retval;
}


int bme280_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, bme280_minor, 1,
            "bme280");
    bme280_major = MAJOR(dev);

    if (result < 0) {
        printk(KERN_ERR "Can't get major %d\n", bme280_major);
        return result;
    }
    
    // Get the I2C adapter (master handle)
    bme280_device.bme280_i2c_adapter = i2c_get_adapter(1); //todo: Check 
    if(bme280_device.bme280_i2c_adapter == NULL)
    {
        printk(KERN_ERR "Failed to get I2C adapter\n");
        result = -ENODEV;
        goto safe_exit;
    }


    // Create an I2C client structure
    bme280_device.bme280_i2c_client = i2c_new_client_device(bme280_device.bme280_i2c_adapter, &bme280_i2c_board);
    if (bme280_device.bme280_i2c_client == NULL) 
    {
        printk(KERN_ERR "Failed to register I2C device\n");
        result = -ENODEV;
        goto safe_exit;
    }

    result = i2c_add_driver(&bme280_i2c_driver);

    if(result)
    {
        printk(KERN_ERR "Error %d adding i2c driver", result);
        goto safe_exit;
    }

    result = bme280_setup_cdev(&bme280_device);

    if(result) {
        unregister_chrdev_region(dev, 1);
    }

    // Intialize the registers of the sensor
    bme280_init_sensor();
    PDEBUG("Initialized\n");
    goto only_exit;

safe_exit:
    unregister_chrdev_region(dev, 1);
only_exit:
    if (bme280_device.bme280_i2c_adapter != NULL)
    {
        i2c_put_adapter(bme280_device.bme280_i2c_adapter);
    }
    return result;

}


void bme280_cleanup_module(void)
{
    dev_t devno;

    // Remove the I2C client from the I2C subsystem 
    i2c_unregister_device(bme280_device.bme280_i2c_client);

    // Delete the driver 
    i2c_del_driver(&bme280_i2c_driver);

    devno = MKDEV(bme280_major, bme280_minor);

    cdev_del(&bme280_device.cdev);

    unregister_chrdev_region(devno, 1);
}


module_init(bme280_init_module);
module_exit(bme280_cleanup_module);
