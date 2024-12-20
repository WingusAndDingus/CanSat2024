/*
 * BMP388.c
 *
 *  Leandro Sanchez
 */


#include "BMP388.h"
#include <math.h>

void fill_calibration_params(struct BMP388_calib_data *calib_data, uint8_t *buffer) {
	// Assign buffer values to NVM_PARAMs
	        calib_data->NVM_PAR_T1 = (uint16_t)((buffer[1] << 8)|buffer[0]);
	        calib_data->NVM_PAR_T2 = (uint16_t)((buffer[3] << 8)|buffer[2]);
	        calib_data->NVM_PAR_T3 = (int8_t)(buffer[4]);

	        calib_data->NVM_PAR_P1 =    (int16_t)((buffer[6] << 8)|buffer[5]);
	        calib_data->NVM_PAR_P2 =    (int16_t)((buffer[8] << 8)|buffer[7]);
	        calib_data->NVM_PAR_P3 =    (int8_t)(buffer[9]);
	        calib_data->NVM_PAR_P4 =    (int8_t)(buffer[10]);
	        calib_data->NVM_PAR_P5 =    (uint16_t)((buffer[12] << 8)|buffer[11]);
	        calib_data->NVM_PAR_P6 =    (uint16_t)((buffer[14] << 8)|buffer[13]);
	        calib_data->NVM_PAR_P7 =    (int8_t)(buffer[15]);
	        calib_data->NVM_PAR_P8 =    (int8_t)(buffer[16]);
	        calib_data->NVM_PAR_P9 =    (int16_t)((buffer[18] << 8)|buffer[17]);
	        calib_data->NVM_PAR_P10 =   (int8_t)(buffer[19]);
	        calib_data->NVM_PAR_P11 =   (int8_t)(buffer[20]);
}

void set_coeff(struct BMP388_data *data, BMP388_calib *calib_data) {
			data->PAR_T1 = calib_data->NVM_PAR_T1/pow(2.0,-8.0);
			data->PAR_T2 = calib_data->NVM_PAR_T2/pow(2.0,30.0);
			data->PAR_T3 = calib_data->NVM_PAR_T3/pow(2.0,48.0);

			data->PAR_P1 = (calib_data->NVM_PAR_P1-pow(2.0, 14.0))/pow(2.0,20.0);
			data->PAR_P2 = (calib_data->NVM_PAR_P2-pow(2.0, 14.0))/pow(2.0,29.0);
			data->PAR_P3 = (calib_data->NVM_PAR_P3)/pow(2.0,32.0);
			data->PAR_P4 = (calib_data->NVM_PAR_P4)/pow(2.0,37.0);
			data->PAR_P5 = (calib_data->NVM_PAR_P5)/pow(2.0,-3.0);
			data->PAR_P6 = (calib_data->NVM_PAR_P6)/pow(2.0,6.0);
			data->PAR_P7 = (calib_data->NVM_PAR_P7)/pow(2.0,8.0);
			data->PAR_P8 = (calib_data->NVM_PAR_P8)/pow(2.0,15.0);
			data->PAR_P9 = (calib_data->NVM_PAR_P9)/pow(2.0,48.0);
			data->PAR_P10 = (calib_data->NVM_PAR_P10)/pow(2.0,48.0);
			data->PAR_P11 = (calib_data->NVM_PAR_P11)/pow(2.0,65.0);
}
void get_compensated(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c) {
	 // Collect data  from registers
			uint8_t check = (0b11 << 5);
			uint8_t ready = 0;
			while ((ready & check) != check){
				HAL_I2C_Mem_Read((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, STATUS, sizeof(uint8_t), &ready, 1, 100);
			}
			uint8_t temp[6];
			HAL_I2C_Mem_Read((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, PRESSURE_DATA, sizeof(uint8_t), (uint8_t *) temp, sizeof(temp), 100);
	        uint32_t uncomp_temp = ((temp[5] << 16) | (temp[4] << 8) | temp[3]);

	        float partial_data1;
	        float partial_data2;

	        partial_data1 = (float)(uncomp_temp - sensor->data.PAR_T1);
	        partial_data2 = (float)(partial_data1 * sensor->data.PAR_T2);
	        /*Update the compensated temperature in data structure since this is needed for
	        pressure calulation*/
	        sensor->data.T_LIN = partial_data2 + (partial_data1 * partial_data1) * sensor->data.PAR_T3;
	        sensor->get_pressure_compensated(sensor, temp);

}

void get_pressure_compensated(struct BMP388_sensor *sensor, uint8_t buffer []) {
	// Collect Raw Value from Sensor
	        uint32_t uncomp_press = ((buffer[2] << 16) | (buffer[1] << 8) | buffer[0]);

	        /*Variable to store the compensated pressure*/
	        /*Temporary variables used for compensation*/

	        float partial_data1;
	        float partial_data2;
	        float partial_data3;
	        float partial_data4;
	        float partial_out1;
	        float partial_out2;
	        /*Calibration Data */

	        partial_data1 = sensor->data.PAR_P6 * sensor->data.T_LIN;
	        partial_data2 = sensor->data.PAR_P7 * (sensor->data.T_LIN * sensor->data.T_LIN);
	        partial_data3 = sensor->data.PAR_P8 * (sensor->data.T_LIN * sensor->data.T_LIN * sensor->data.T_LIN);
	        partial_out1  = sensor->data.PAR_P5 + partial_data1 + partial_data2 + partial_data3;

	        partial_data1 = sensor->data.PAR_P2 * sensor->data.T_LIN;
	        partial_data2 = sensor->data.PAR_P3 * (sensor->data.T_LIN * sensor->data.T_LIN);
	        partial_data3 = sensor->data.PAR_P4 * (sensor->data.T_LIN * sensor->data.T_LIN * sensor->data.T_LIN);
	        partial_out2  = (float)uncomp_press *  (sensor->data.PAR_P1 + partial_data1 + partial_data2 + partial_data3);

	        partial_data1 = (float)uncomp_press * (float)uncomp_press;
	        partial_data2 = sensor->data.PAR_P9 + sensor->data.PAR_P10 * sensor->data.T_LIN;
	        partial_data3 = partial_data1 * partial_data2;
	        partial_data4 = partial_data3 + ((float)uncomp_press * (float)uncomp_press * (float)uncomp_press) * sensor->data.PAR_P11;
	        sensor->data.P_LIN = partial_out1 + partial_out2 + partial_data4;

}

void calibrate_sensor(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c) {
	uint8_t buffer [21];
	HAL_I2C_Mem_Read((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, CALIBRATION_REG, sizeof(uint8_t), (uint8_t *) buffer, sizeof(buffer), 1000);

	sensor->raw_calib.fill_calibration_params(&sensor->raw_calib, (uint8_t *)buffer);
	sensor->data.set_coeff(&sensor->data, &sensor->raw_calib);

}

void configure_sensor(struct BMP388_sensor *sensor, I2C_HandleTypeDef* hi2c) {
    // Implementation of configure_sensor...
	uint8_t sf  = SOFT_RESET;
	uint8_t osr = OSR_SETTINGS;
	uint8_t pwr = PWR_SETTINGS;
	uint8_t iir = IIR_FILTER;
	uint8_t odr = ODR_SETTING;

	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, CMD_BMP, sizeof(uint8_t), &sf,sizeof(uint8_t), 100);
	HAL_Delay(250);

	sensor->calibrate_sensor(sensor, (I2C_HandleTypeDef*)hi2c);
	HAL_Delay(250);

	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, CONFIG, 1, &iir,1, 100);
	HAL_Delay(250);

	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, ODR, 1, &odr,1, 100);
	HAL_Delay(250);


	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, OSR, 1, &osr,1, 100);
	HAL_Delay(250);


	HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR, PWR_CTRL, 1, &pwr,1, 100);
	HAL_Delay(250);


}

BMP388_sensor *BMP388_sensor_create() {
    BMP388_sensor *sensor = (BMP388_sensor *)malloc(sizeof(BMP388_sensor));
    if (sensor) {
        sensor->raw_calib.fill_calibration_params = fill_calibration_params;
        sensor->data.set_coeff = set_coeff;
        sensor->get_compensated = get_compensated;
        sensor->get_pressure_compensated = get_pressure_compensated;
        sensor->calibrate_sensor = calibrate_sensor;
        sensor->configure_sensor = configure_sensor;
    }
    return sensor;
}
