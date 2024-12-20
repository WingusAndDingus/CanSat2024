/*
 * BMP388.h
 *
 *  Created on: Apr 27, 2024
 *      Author: Leandro Sanchez
 */

#ifndef INC_BMP388_H_
#define INC_BMP388_H_

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


#define DEVICE_ADDR (0x77 << 1)
// Data Registers
#define ERR_REG 0x02
#define STATUS 0x03             // Data Status Register
#define PRESSURE_DATA 0X04      // LSB - 0x06
#define TEMPERATURE_DATA 0x07   // LSB -0x09
   //
// Registers for Setup
#define CHIP_ID 0x00
#define PWR_CTRL 0x1B   // Communication I2C Setup
#define OSR 0x1C        // Oversampling Setup
#define ODR 0x1D
#define CMD_BMP 0x7E
#define CONFIG 0x1F

//Settings
#define PWR_SETTINGS 0b00110011
#define OSR_SETTINGS 0b00000011
#define IIR_FILTER (0x02 << 1)
#define SOFT_RESET 0xB6
#define ODR_SETTING (0x02 << 0)


#define CALIBRATION_REG 0x31


typedef struct BMP388_calib_data {
    uint16_t NVM_PAR_T1;
    uint16_t NVM_PAR_T2;
    int8_t   NVM_PAR_T3;

    int16_t  NVM_PAR_P1;
    int16_t  NVM_PAR_P2;
    int8_t   NVM_PAR_P3;
    int8_t   NVM_PAR_P4;
    uint16_t NVM_PAR_P5;
    uint16_t NVM_PAR_P6;
    int8_t   NVM_PAR_P7;
    int8_t   NVM_PAR_P8;
    int16_t  NVM_PAR_P9;
    int8_t   NVM_PAR_P10;
    int8_t   NVM_PAR_P11;

    void (*fill_calibration_params)(struct BMP388_calib_data *, uint8_t *);
} BMP388_calib;

typedef struct BMP388_data {
    float PAR_T1;
    float PAR_T2;
    float PAR_T3;

    float PAR_P1;
    float PAR_P2;
    float PAR_P3;
    float PAR_P4;
    float PAR_P5;
    float PAR_P6;
    float PAR_P7;
    float PAR_P8;
    float PAR_P9;
    float PAR_P10;
    float PAR_P11;

    float T_LIN;
    float P_LIN;

    void (*set_coeff)(struct BMP388_data *, BMP388_calib *);
} BMP388_data;

typedef struct BMP388_sensor {
    BMP388_calib raw_calib;
    BMP388_data  data;

    void (*get_compensated)(struct BMP388_sensor *, I2C_HandleTypeDef*);
    void (*get_pressure_compensated)(struct BMP388_sensor *, uint8_t []);
    void  (*calibrate_sensor)(struct BMP388_sensor *, I2C_HandleTypeDef*);
    void  (*configure_sensor)(struct BMP388_sensor *, I2C_HandleTypeDef*);
} BMP388_sensor;

void fill_calibration_params(struct BMP388_calib_data *calib_data, uint8_t *buffer);

void set_coeff(struct BMP388_data *data, BMP388_calib *calib_data);

void get_compensated(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c);

void get_pressure_compensated(struct BMP388_sensor *sensor, uint8_t buffer []);

void calibrate_sensor(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c);

void configure_sensor(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c);

BMP388_sensor *BMP388_sensor_create() ;

//int main() {
//    // Example usage
//    BMP388_sensor *sensor = BMP388_sensor_create();
//    // Use the sensor functions...
//    free(sensor); // Don't forget to free the allocated memory
//    return 0;
//}


#endif /* INC_BMP388_H_ */
