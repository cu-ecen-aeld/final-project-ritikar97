/**
 * @file    bme280.h
 * @brief   Functions and data related to the BME280 driver implementation
 *
 * @author  Ritika Ramchandani
 * @date    2023-04-12
 *
 */

#ifndef BME280_DRIVER_H_
#define BME280_DRIVER_H_

#define BME280_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef BME280_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "bme280: " fmt, ## args)
#include <linux/types.h>
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#include <stddef.h> 
#include <stdint.h> 
#include <stdbool.h>
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#include <linux/i2c.h>

#define CALIB_DATA_PT_LEN (24)
#define LONG_SIGNED_INT_NUM (1)
#define MEASUREMENT_LEN (25)

// Only for Temperature and Pressure
enum calib_data_digits
{
    dig_T1 = 0,
    dig_T2,
    dig_T3,
    dig_P1,
    dig_P2,
    dig_P3,
    dig_P4,
    dig_P5,
    dig_P6,
    dig_P7,
    dig_P8,
    dig_P9
};


struct bme280_dev
{
    struct cdev cdev;     /* Char device structure */
    struct i2c_adapter *bme280_i2c_adapter;
    struct i2c_client *bme280_i2c_client;
    uint8_t calib_data[CALIB_DATA_PT_LEN];
};


#endif /* BME280_DRIVER_H_ */
