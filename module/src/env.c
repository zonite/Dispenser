
#include "dispenser.h"

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
//#include <asm/fpu/api.h>

#include "bme280.h"

/**
 *
 *  https://github.com/Johannes4Linux/Linux_Driver_Tutorial/blob/main/07_BMP280/bmp280.c
 *  https://docs.kernel.org/i2c/writing-clients.html
 *  https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
 *
 **/

void env_update_fn(struct work_struct *work);

//DECLARE_WORK(workqueue, env_update_fn);
DECLARE_DELAYED_WORK(workqueue, env_update_fn);

//plain 135 MB/s write
//plain 682 MB/s read
//crypt 13.1 MB/s write
//crypt 766 MB/s read

//BME280 addresses:
//#define I2C_BUS 1
#define SLAVE_NAME "BMP280"
//#define BMP280_ADDRESS 0x76
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
	s32 ret = 0;
	struct i2c_client *sensor = i2c_addr;

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

	if (len > 1) {
		ret = i2c_smbus_read_i2c_block_data(sensor, reg_address, len, reg_data);
		// < 0 == error
		if ( ret < 0) {
			printk("i2c read block error %i.", ret);
		}
	} else {
		ret = i2c_smbus_read_byte_data(sensor, reg_address);

		if (ret < 0) {
			//error
			printk("i2c read byte error %i.", ret);
			return ret;
		}
		*reg_data = ret;
	}

	return BME280_INTF_RET_SUCCESS;
}

static s8 sensor_write(u8 reg_address, const u8 *reg_data, u32 len, void *i2c_addr)
{
	s32 ret = 0;
	struct i2c_client *sensor = i2c_addr;

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

	//BUG in BME280 driver... reg data longer than 1 is interleaved with addresses!

	if (len > 1) {
		printk("i2c write block not supported, %i.", len);
		return -1;

		ret = i2c_smbus_write_i2c_block_data(sensor, reg_address, len, reg_data);
		if ( ret < 0) {
			printk("i2c write block error %i.", ret);
		}
	} else {
		ret = i2c_smbus_write_byte_data(sensor, reg_address, *reg_data);

		if (ret < 0) {
			//error
			printk("i2c write byte error %i.", ret);
			return ret;
		}
	}

	return BME280_INTF_RET_SUCCESS;
}

static void sensor_delay(u32 period, void *i2c_addr)
{
	usleep_range(period, 2 * period);
}


//static struct bme280_dev dev = {0};
/*
static struct i2c_driver bme280_i2c_driver = {
	.driver = {
		.name = SLAVE_NAME,
		.owner = THIS_MODULE
	}
};
*/
//#define BMP280_ADDRESS 0x76

/*
static void sensor_tmr_callback(struct timer_list *timer)
{
	int powered = 1;
	if (pDispenser_mmap) {
		powered = pDispenser_mmap->unit.charging;
	}

	schedule_work(&workqueue);

	mod_timer(timer, jiffies + msecs_to_jiffies(powered ? 60000 : 600000));
}
*/

void env_update_fn(struct work_struct *work) {
	int powered = 1;

	if (!cDispenser.env) {
		printk("ENV disabled, but called.");
		return;
	}

	if (pDispenser_mmap) {
		powered = pDispenser_mmap->unit.charging;
	}

	printk("ENV update callback.");

	sensor_update(cDispenser.env);

	dispenser_environment_event(cDispenser.env->data.temperature,
	                            cDispenser.env->data.pressure,
	                            cDispenser.env->data.humidity);

	schedule_delayed_work(&workqueue, msecs_to_jiffies(powered ? 60000 : 15000));
	//schedule_delayed_work(&workqueue, msecs_to_jiffies(powered ? 60000 : 600000));
}

static int8_t sensor_update(struct env_data *env)
{
	//Reads data from sensor

	//bme280_get_sensor_data(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev);
	return bme280_get_sensor_data(BME280_ALL, &env->data, &env->dev);
}

static int8_t sensor_init(struct env_data *env)
{
	int8_t rslt = BME280_OK;
	uint8_t dev_addr = BME280_I2C_ADDR_PRIM;
	struct bme280_dev *dev = NULL;
	struct i2c_driver *bme280_i2c_driver = NULL;
	struct bme280_settings *settings = NULL;

	if (!env)
		return -1;

	//env->i2c_driver_data = bme280_i2c_driver;
	//env->i2c_driver_data.driver.;
	env->i2c_driver_data.driver.name = SLAVE_NAME;
	env->i2c_driver_data.driver.owner = THIS_MODULE;
	dev_addr = env->addr;
	bme280_i2c_driver = &env->i2c_driver_data;
	dev = &env->dev;

	struct i2c_board_info bme280_i2c_board_info = {
		.type = SLAVE_NAME,
		.addr = dev_addr
	};

	//static struct i2c_board_info bme280_i2c_board_info2 = {
	//	I2C_BOARD_INFO(SLAVE_NAME, BME280_I2C_ADDR_PRIM)};

	struct i2c_adapter *bme280_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
	struct i2c_client *bme280_i2c_client = NULL;

	if (bme280_i2c_adapter != NULL)
	{
		bme280_i2c_client = i2c_new_client_device(bme280_i2c_adapter, &bme280_i2c_board_info);
		if (bme280_i2c_client != NULL)
		{

			int add_i2c = i2c_add_driver(bme280_i2c_driver);
			if (add_i2c != -1)
			{
				rslt = 0;
			}
			else
			{
				pr_err("bme250: failed to add i2c driver \n");
			}
		}
		i2c_put_adapter(bme280_i2c_adapter);
	} else {
		//Failed!
		return -2;
	}

	dev->intf_ptr = bme280_i2c_client; //struct i2c_client *
	dev->intf = BME280_I2C_INTF;
	dev->read = sensor_read;
	dev->write = sensor_write;
	dev->delay_us = sensor_delay;
	settings = &dev->settings;

	rslt = bme280_init(dev);
	if (rslt != BME280_OK) {
		printk("BME280 sensor not initialized.");
		return rslt;
	}
	printk("BME280 sensor found and initialized.");

	// set ctrl_meas register at 0xF4
	// set normal mode (b11)
	settings->filter = 0; //(7 << 2) BME280_FILTER_MSK
	// set XX register at 0xF2
	settings->osr_h = 0x3; // set ctrl_hum (humidity oversampling)
	settings->osr_p = 0x5; // set pressure oversampling to max (b101)
	settings->osr_t = 0x5; // set temperature oversampling to max (b101)

	// set config register at 0xF5
	// set t_standby time 0xf5, 0x3 << 5 == 0x60 == 96
	settings->standby_time = 0x3; // set t_standby time

	//(BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL | BME280_STANDBY_SEL == BME280_ALL_SETTINGS_SEL)
	bme280_set_sensor_settings(BME280_ALL_SETTINGS_SEL, dev);
	//set_osr_press_temp_settings(BME280_ALL_SETTINGS_SEL);

	// set ctrl_meas register at 0xF4
	bme280_set_sensor_mode(BME280_NORMAL_MODE, dev); // set normal mode (b11)

	//bme280_set_regs(uint8_t *reg_addr, const uint8_t *reg_data, uint8_t len, struct bme280_dev *dev);
	//bme280_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, struct bme280_dev *dev);

	//rslt = stream_sensor_data_forced_mode(&dev); //Not found!!!

	//timer_setup(&env->timer, sensor_tmr_callback, 0);
	//mod_timer(&env->timer, jiffies + msecs_to_jiffies(5000)); //initial 5 sec delay
	schedule_delayed_work(&workqueue, msecs_to_jiffies(5000));
	//schedule_work(&workqueue);

	return rslt;
}

static void sensor_close(struct env_data *env)
{
	if (!env) {
		return;
	}

	//if (timer_pending(&env->timer)) {
	//	printk("Deleting ENV pending timer!\n");
	//	del_timer(&env->timer);
	//}

	//cancel_work_sync(&workqueue);
	cancel_delayed_work_sync(&workqueue);
	//flush_scheduled_work(); //deprecated!

	i2c_del_driver(&env->i2c_driver_data);
	i2c_unregister_device((struct i2c_client *)env->dev.intf_ptr);
	//i2c_del_driver(&bme280_i2c_driver);
	//i2c_unregister_device(bme280_i2c_client);
}
