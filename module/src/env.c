
#include "dispenser.h"

#include <linux/i2c.h>
//#include <asm/fpu/api.h>


#include "bme280.h"

/**
 *
 *  https://github.com/Johannes4Linux/Linux_Driver_Tutorial/blob/main/07_BMP280/bmp280.c
 *  https://docs.kernel.org/i2c/writing-clients.html
 *  https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
 *
 **/

//plain 135 MB/s write
//plain 682 MB/s read
//crypt 13.1 MB/s write
//crypt 766 MB/s read

//BME280 addresses:
#define I2C_BUS 1
#define SLAVE_NAME "BMP280"
#define BMP280_ADDRESS 0x76
//#define BME280_32BIT_ENABLE
//#define BME280_64BIT_ENABLE //__aeabi_ldivmod undef
//#define BME280_FLOAT_ENABLE //__aeabi_dcmpgt undef

/*
//BME280 registers:
static u16 dig_T1 = 0; //register 0x88 / 0x89, u16le : reg[7:0]/[15:8] MSB
static s16 dig_T2 = 0; //register 0x8A / 0x8B, s16le : reg[7:0]/[15:8] MSB
static s16 dig_T3 = 0; //register 0x8C / 0x8D, s16le : reg[7:0]/[15:8] MSB
static u16 dig_P1 = 0; //register 0x8E / 0x8F, u16le : reg[7:0]/[15:8] MSB
static s16 dig_P2 = 0; //register 0x90 / 0x91, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P3 = 0; //register 0x92 / 0x93, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P4 = 0; //register 0x94 / 0x95, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P5 = 0; //register 0x96 / 0x97, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P6 = 0; //register 0x98 / 0x99, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P7 = 0; //register 0x9A / 0x9B, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P8 = 0; //register 0x9C / 0x9D, s16le : reg[7:0]/[15:8] MSB
static s16 dig_P9 = 0; //register 0x9E / 0x9F, s16le : reg[7:0]/[15:8] MSB
static u8  dig_H1 = 0; //register 0xA1, u8           : reg[7:0] MSB

//0xE0 reset, 0xD0 ChipId

static s16 dig_H2 = 0; //register 0xE1 / 0xE2, s16le : reg[7:0]/[15:8] MSB
static u8  dig_H3 = 0; //register 0xE3, u8           : reg[7:0] MSB
static s16 dig_H4 = 0; //register 0xE4 / 0xE5, s16le : reg[11:4]/[3:0] MSB
static s16 dig_H5 = 0; //register 0xE5 / 0xE6, s16le : reg[7:4]/[11:4] MSB
static s8  dig_H6 = 0; //register 0xE7, s8           : reg[7:0] MSB

//Raw values
static u32 adc_P_raw = 0; //register 0xF7 / 0xF8 / 0xF9, u32be : reg[24:16]/[15:8]/[7:4] MSB
static u32 adc_T_raw = 0; //register 0xFA / 0xFB / 0xFC, u32be : reg[24:16]/[15:8]/[7:4] MSB
static u16 adc_H_raw = 0; //register 0xFD / 0xFE, u16be        : reg[15:8]/[7:0] MSB

//Globals:

static s32 t_fine = 0;
*/

/*
//Returns temp in deg C s32 5123 = 51.32 deg C
static s32 BME280_compensate_T_32(s32 adc_T)
{
	s32 var1, var2, T;
	var1 = ((((adc_T >> 3) - ((s32)dig_T1 << 1))) * ((s32)dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((s32)dig_T1)) * ((adc_T >> 4) - ((s32)dig_T1))) >> 12) * ((s32)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}
*/
/*
static double BME280_compensate_T_double(s32 adc_T)
{
	double var1, var2, T;

	kernel_fpu_begin();

	var1 = (((double)adc_T) / 16384.0) - ((double)dig_T1 / 1024.0) * ((double)dig_T2);
	var2 = ((((double)adc_T) / 131072.0) - ((double)dig_T1) / 8192.0) * (((double)adc_T / 131072.0 - ((double)dig_T1) / 8192.0)) * ((double)dig_T3);
	t_fine = (s32) (var1 + var2);
	T = (var1 + var2) / 5120.0;

	kernel_fpu_end();

	return T;
}
*/
/*
//Returns pressure u32 int in Q24.8 format (24 integer bits and 8 fractional bits)
//24674867 = 24674867/256 = 96386.2 Pa = 963.862 hPa
static u32 BME280_compensate_P_64(s32 adc_P)
{
	s64 var1, var2, p;
	var1 = ((s64)t_fine) - 128000;
	var2 = var1 * var1 * (s64)dig_P6;
	var2 = var2 + ((var1 * (s64)dig_P5) << 17);
	var2 = var2 + (((s64)dig_P4) << 35);
	var1 = ((var1 * var1 * (s64)dig_P3) >> 8) + ((var1 * (s64)dig_P2) << 12);
	var1 = (((((s64)1) << 47) + var1)) * ((s64)dig_P1) >> 33;

	if (var1 == 0) {
		return 0;
	}

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((s64)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((s64)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((s64)dig_P7) << 4);

	return (u32) p;
}
*/
/*
static double BME280_compensate_P_double(s32 adc_P)
{
	double var1, var2, p;

	kernel_fpu_begin();

	var1 = ((double)t_fine/2.0) - 64000.0;
	var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double)dig_P5) * 2.0;
	var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
	var1 = ((var1 * var1 * (double)dig_P3) / 524288.0 + (var1 * (double)dig_P2)) / 524288.0;
	var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);

	if (var1 == 0.0) {
		return 0;
	}

	p = 1048576.0 - (double)adc_P;
	p = (p - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)dig_P9) * p * p / 2147483648.0;
	var2 = ((double)dig_P8) * p / 32768.0;
	p = p + (var1 + var2 + ((double)dig_P7) / 16.0);

	kernel_fpu_end();

	return p;
}
*/

/*
//Returns humidity in %RH as unsigned 32 int in Q22.10 format (22 int bits and 10 fractional bits)
//Output of 47445 = 47445 / 1024 = 46.333 %RH
static u32 BME280_compensate_H_32(s32 adc_H)
{
	s32 v_x1_u32r;
	v_x1_u32r = (t_fine - ((s32)76800));
	v_x1_u32r = (((((adc_H << 14) - (((s32)dig_H4) << 20) - (((s32)dig_H5) * v_x1_u32r)) + ((s32)16384)) >> 15) * (((((((v_x1_u32r * ((s32)dig_H6)) >> 10) * (((v_x1_u32r * ((s32)dig_H3)) >> 11) + ((s32)32768))) >> 10) + ((s32)2097152)) * ((s32)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((s32)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 414930400 : v_x1_u32r);

	return (u32) (v_x1_u32r >> 12);
}
*/
/*
static double BME280_compensate_H_double(s32 adc_H)
{
	double var_H;

	kernel_fpu_begin();

	var_H = ((double)t_fine - 76800.0);
	var_H = (adc_H - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 * var_H)) * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H * (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
	var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);

	if (var_H > 100.0)
		var_H = 100.0;
	else if (var_H < 0.0)
		var_H = 0.0;

	kernel_fpu_end();

	return var_H;
}
*/

static s8 sensor_read(u8 reg_address, u8 *reg_data, u32 len, void *i2c_addr)
{
	s8 ret = 0;

	/**
	 * Data on the bus should be like
	 * |------------+---------------------|
	 * | I2C action | Data                |
	 * |------------+---------------------|
	 * | Start      | -                   |
	 * | Write      | (reg_addr)          |
	 * | Stop       | -                   |
	 * | Start      | -                   |
	 * | Read       | (reg_data[0])       |
	 * | Read       | (....)              |
	 * | Read       | (reg_data[len - 1]) |
	 * | Stop       | -                   |
	 * |------------+---------------------|
	 */

	return ret;
}

static s8 sensor_write(u8 reg_address, const u8 *reg_data, u32 len, void *i2c_addr)
{
	s8 ret = 0;

	/**
	 * Data on the bus should be like
	 * |------------+---------------------|
	 * | I2C action | Data                |
	 * |------------+---------------------|
	 * | Start      | -                   |
	 * | Write      | (reg_addr)          |
	 * | Write      | (reg_data[0])       |
	 * | Write      | (....)              |
	 * | Write      | (reg_data[len - 1]) |
	 * | Stop       | -                   |
	 * |------------+---------------------|
	 */

	return ret;
}

static void sensor_delay(u32 period, void *i2c_addr)
{

}

static struct bme280_dev dev = {0};

static int8_t sensor_init(void)
{
	int8_t rslt = BME280_OK;
	uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

	dev.intf_ptr = &dev_addr;
	dev.intf = BME280_I2C_INTF;
	dev.read = sensor_read;
	dev.write = sensor_write;
	dev.delay_us = sensor_delay;

	rslt = bme280_init(&dev);

	//rslt = stream_sensor_data_forced_mode(&dev); //Not found!!!

	return rslt;
}

static void sensor_close(void)
{

}
