/*
 * ICM-42688.c
 *
 *  Leandro Sanchez
 */

#include "ICM-42688.h"


ICM_data *ICM_sensor_create() {
    ICM_data *sensor = (ICM_data *)malloc(sizeof(ICM_data));
    if (sensor) {
        sensor->collect_data = collect_data;
    }
    return sensor;
}

void setup_ICM (I2C_HandleTypeDef* hi2c){
		uint8_t gyro_set  = GYRO_SETTING;
		uint8_t accel_set = ACCEL_SETTING;
		uint8_t pwr_set   = PWR_SET_ICM;


		HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR_ICM, ACCEL_CONFIG0, 1, &accel_set,1, 100);
		HAL_Delay(250);

		HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR_ICM, GYRO_CONFIG0, 1, &gyro_set,1, 100);
		HAL_Delay(250);

		HAL_I2C_Mem_Write((I2C_HandleTypeDef*)hi2c, DEVICE_ADDR_ICM, PWR_MGMT0, 1, &pwr_set, 1, 100);
		HAL_Delay(250);

}

void collect_data (struct ICM_data *sensor, I2C_HandleTypeDef* hi2c){

	uint8_t temp_buffer[14];
	HAL_I2C_Mem_Read((I2C_HandleTypeDef*) hi2c, DEVICE_ADDR_ICM, DATA_START, 1, (uint8_t *)temp_buffer, sizeof(temp_buffer), 1000);

	sensor->TEMP 	= ((temp_buffer[1] << 8)|(temp_buffer[0]));
	sensor->ACCEL_X = ((temp_buffer[3] << 8)|(temp_buffer[2]));
	sensor->ACCEL_Y = ((temp_buffer[5] << 8)|(temp_buffer[4]));
	sensor->ACCEL_Z = ((temp_buffer[7] << 8)|(temp_buffer[6]));
	sensor->GYRO_X  = ((temp_buffer[9] << 8)|(temp_buffer[8]));
	sensor->GYRO_Y  = ((temp_buffer[11] << 8)|(temp_buffer[10]));
	sensor->GYRO_Z  = ((temp_buffer[13] << 8)|(temp_buffer[12]));


}
