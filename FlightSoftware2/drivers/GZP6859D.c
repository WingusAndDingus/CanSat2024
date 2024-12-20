#include "GZP6859D.h"

//void GZP_INIT(){
//	//status = null;
//	//set GZP settings? defaults seem okay, might not need init function at all
//}

void GZP_SET_REG(I2C_HandleTypeDef* hi2c, uint8_t reg_addr, uint8_t input){
	//I2c_HandleTypeDef, device addr, addr to write to on device,
	//memaddsize, input, size, timeout

	//check if it works with status return val? add later todo
	//status = Hal_I2C_Mem_Write(&hi2c,GZP_I2C_ADDR,reg_addr,1,input,1,50);

	HAL_I2C_Mem_Write(&hi2c,GZP_I2C_ADDR,reg_addr,1,input,1,50);

	//if any issues with HAL or writing, return
//	if(status != HAL_OK){
//		return status;
//	}
//	return HAL_OK;
}

uint8_t GZP_READ_REG(I2C_HandleTypeDef* hi2c, uint8_t reg_addr){
	uint8_t output;
	HAL_I2C_Mem_Read(&hi2c,GZP_I2C_ADDR,reg_addr,1,&output,1,50);
	return output;
}

void GZP_READ_DATA(I2C_HandleTypeDef* hi2c, uint8_t* combined){
	uint8_t check = 1 << 3;

	GZP_SET_REG(hi2c, CMD, REQUEST_CMD);

	//wait for Sco bit to go to zero, means conversions are done and can be read
	while((GZP_READ_REG(hi2c, SYS_CFG) & check) != check);

	//read first 3 pressure, next 2 temp into same array
	//combined = (uint8_t*) malloc(5);
	HAL_I2C_Mem_Read(&hi2c,GZP_I2C_ADDR,START_ADDR | 0x01,1,&combined,5,50);
	//return out;
}

double GZP_READ_PRESSURE(uint8_t* combined){
	uint32_t pressure_ADC = combined[0] * 65536 + combined[1] * 256 + combined[2];
	uint32_t check_sign = 1 << 23;

	//convert ADC to kPa units:
	//bit 23 is 0, positive
	if((pressure_ADC & check_sign) != check_sign){
		return pressure_ADC / K_VAL;
	}

	return (pressure_ADC - (2^24)) / K_VAL;
}

double GZP_READ_TEMP(uint8_t* combined){
	uint32_t temp_ADC = combined[3] * 256 + combined[4];
	uint32_t check_sign = 1 << 15;

	//convert ADC to C units:
	//bit 15 is 0, positive
	if((temp_ADC & check_sign) != check_sign){
		return (double)(temp_ADC / 256); //from GZP6859D datasheet
	}else{
		return (double)((temp_ADC - (2^16)) / 256); //""
	}
}

double GZP_CALC_SPEED(I2C_HandleTypeDef* hi2c){
	uint8_t* combined[5];
	GZP_READ_DATA(hi2c,*combined);
	double R = 287; // J/kg/K
	double P = GZP_READ_PRESSURE(*combined);
	double T = GZP_READ_TEMP(*combined);
	free(combined);
	//air density = pressure / (temp * specific gas constant)
	// kg/m^3 = (Pa or N/m^2) / (K * J/kg/K)
	double p = P / (T * R);
	double velocity = (2 * P) / p;
	return sqrt(velocity); //sqrt(2P/p)
}