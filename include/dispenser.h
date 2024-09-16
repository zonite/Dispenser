#ifndef DISPENSER_H
#define DISPENSER_H

#include <linux/kernel.h>
#include <linux/types.h>

#define ORGANIZATION "MNY"
#define DOMAIN "nykyri.eu"
#define APPNAME "Dispenser"

#define DAEMON_NAME "Dispenser Daemon"
#define DAEMON_VER "0.1"

#define SEC_TO_MSEC(A) ((A) * 1000)
#define LIGHT_TIMEOUT SEC_TO_MSEC(5 * 60)
#define FAIL_TIMEOUT SEC_TO_MSEC(15)
#define DOOR_TIMEOUT SEC_TO_MSEC(45 * 60)
#define FILL_TIMEOUT SEC_TO_MSEC(4 * 60)
//#define LIGHT_TIMEOUT SEC_TO_MSEC(3)
//#define FAIL_TIMEOUT SEC_TO_MSEC(12)
//#define DOOR_TIMEOUT SEC_TO_MSEC(6)
#define INT_DEBOUNCE 40
#define POLL_INTERVAL SEC_TO_MSEC(60)

/**
 * Dispenser Generic Netlink Interface
 */

#define DISPENSER_GENL_NAME "dispenser_genl"
#define DISPENSER_GENL_GROUP 1

/**
 * Dispenser Daemon config
 */

#define DISPENSER_TCP_PORT 80

struct __bme280_calib_data
{
    /*< Calibration coefficient for the temperature sensor */
    __u16 dig_t1;

    /*< Calibration coefficient for the temperature sensor */
    __s16 dig_t2;

    /*< Calibration coefficient for the temperature sensor */
    __s16 dig_t3;

    /*< Calibration coefficient for the pressure sensor */
    __u16 dig_p1;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p2;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p3;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p4;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p5;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p6;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p7;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p8;

    /*< Calibration coefficient for the pressure sensor */
    __s16 dig_p9;

    /*< Calibration coefficient for the humidity sensor */
    __u8 dig_h1;

    /*< Calibration coefficient for the humidity sensor */
    __s16 dig_h2;

    /*< Calibration coefficient for the humidity sensor */
    __u8 dig_h3;

    /*< Calibration coefficient for the humidity sensor */
    __s16 dig_h4;

    /*< Calibration coefficient for the humidity sensor */
    __s16 dig_h5;

    /*< Calibration coefficient for the humidity sensor */
    __u8 dig_h6;

    /*< Variable to store the intermediate temperature coefficient */
    __s32 t_fine;
};


/**
 * @brief The DISPENSER_GENL_ATTRIBUTE enum Netlink message payload
 */

enum DISPENSER_GENL_ATTRIBUTE {
	DISPENSER_GENL_ATTR_UNSPEC,
	DISPENSER_GENL_MEM_COUNTER, //u32 attr
	DISPENSER_GENL_RELEASE_COUNT, //u8 attr
	DISPENSER_GENL_COL_NUM, //u8 attr
	DISPENSER_GENL_SLOT_NUM, //u8 attr
	DISPENSER_GENL_SLOT_STATUS, //bitfield up,down,release,+enum state (5bits) (settable)
	DISPENSER_GENL_SLOT_FAILED_UP, //u32 attr
	DISPENSER_GENL_SLOT_FAILED_DOWN, //u32 attr
	DISPENSER_GENL_UNIT_STATUS, //bitfield door,power,night,light (night+light settable)
	DISPENSER_GENL_TEMPERATURE, //u32 attr //raw temperature
	DISPENSER_GENL_PRESSURE, //u32 attr //raw pressure
	DISPENSER_GENL_HUMIDITY, //u32 attr //raw humidity
	DISPENSER_GENL_CALIBRATION0, //calibration data u64
	DISPENSER_GENL_CALIBRATION1, //calibration data s16
	DISPENSER_GENL_INITIALIZED, //calibration data u8
	__DISPENSER_GENL_ATTR_MAX,
} __attribute__ ((__packed__));

#define DISPENSER_GENL_ATTR_COUNT (__DISPENSER_GENL_ATTR_MAX)
#define DISPENSER_GENL_ATTR_MAX (__DISPENSER_GENL_ATTR_MAX - 1)

/**
 * @brief The DISPENSER_GENL_COMMAND enum Netlink commands (functions) executed by the receiver
**/

enum DISPENSER_GENL_COMMAND {
	DISPENSER_GENL_CMD_UNSPEC,
	DISPENSER_GENL_CMD_RELEASE, //action by attributes
	DISPENSER_GENL_CMD_SLOT_STATUS, //u8 col, u8 slot, u8 state attr, u32 counter
	DISPENSER_GENL_CMD_COL_STATUS, //u8 col, u8 slot, u32 counter
	DISPENSER_GENL_CMD_UNIT_STATUS, //u8 col, u8 slot, u8 state attr, u32 counter
	DISPENSER_GENL_CMD_ENVIRONMENT, //u32 temp, u32 press, u32 humid, u32 counter //raw temperature
	DISPENSER_GENL_CMD_TEMPERATURE_CALIBRATION, //u32 C0, u32 counter //calibration data: u16 t1, s16 t2, s16 t3
	DISPENSER_GENL_CMD_PRESSURE_CALIBRATION, //u32 C0-1, u32 counter //calibration data: u16 p1, s16 p2-9
	DISPENSER_GENL_CMD_HUMIDITY_CALIBRATION, //u32 C0, u32 counter //calibration data: u8 h1, s16 h2, u8 h3, s16 h4-5, u8 h6
//	DISPENSER_GENL_CMD_DUMP, //Dumps the mmap-area
	__DISPENSER_GENL_CMD_MAX,
} __attribute__ ((__packed__));

//#define DISPENSER_GENL_CMD_ENUM (__DISPENSER_GENL_CMD_MAX)
#define DISPENSER_GENL_CMD_MAX (__DISPENSER_GENL_CMD_MAX - 1)

//For alarms
enum weekdays {
	NONE = 0,
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 4,
	THURSDAY = 8,
	FRIDAY = 16,
	SATURDAY = 32,
	SUNDAY = 64,
	EVERYDAY = 127,
}__attribute__ ((__packed__));
static_assert( sizeof(enum weekdays) == 1 );

//Maximum 4 bits! = 0-7 = 8 states
enum slot_state {
	UNKNOWN,
	FAILED,
	CLOSED,
	RELEASE,
	OPENING,
	OPEN,
	CLOSING
} __attribute__ ((__packed__));
static_assert( sizeof(enum slot_state) == 1 );

enum iocl_cmd {
	RELEASE_ALL,
	RELEASE_SLOT,
	RELEASE_COLUMN,
	RELEASE_UNIT,
	ALL_CLOSED,
	FAILED_SLOT
} __attribute__ ((__packed__));
static_assert( sizeof(enum slot_state) == 1 );

struct dispenser_slot_position {
	char col;
	char slot;
};

struct dispenser_release {
	char col;
	char count;
	char force;
};

union dispenser_ioctl_parameter {
	struct dispenser_slot_position slot;
	struct dispenser_release release;
};

struct dispenser_ioctl {
	enum iocl_cmd cmd;
	union dispenser_ioctl_parameter param;
};


struct dispenser_mmap_unit {
	volatile unsigned int counter;
	volatile char charging;
	volatile char button;
	volatile char light;
	volatile char door;
	volatile __s32 temperature;
	volatile __u32 pressure;
	volatile __u32 humidity;
#ifndef BME280_DEFS_H_
	volatile struct __bme280_calib_data calib_data;
#else
	volatile struct bme280_calib_data calib_data;
#endif
	signed char night;
	signed char ncols;
	signed char nslots;
	signed char initialized;
};

struct dispenser_mmap_column {
	signed char col_id;
	signed char slot_count;
};

struct dispenser_mmap_slot {
	volatile enum slot_state state;
	volatile char up;
	volatile char down;
	volatile char release;
	volatile int up_failed;
	volatile int down_failed;
};

union dispenser_mmap {
	struct dispenser_mmap_unit unit;
	struct dispenser_mmap_column column;
	struct dispenser_mmap_slot slot;
};

#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)
#define GREETER  _IOW('a', 'c', struct dispenser_ioc *)
//#define DISPENSERIOCTL _IO(0xB4, 0x20)

#define DISPENSER_CMD _IOW(0xB4, 0x20, struct dispenser_ioctl *)


//bitfield door,power,night,light (night+light settable)
inline unsigned char dispenser_pack_unit_status(const struct dispenser_mmap_unit *unit)
{
	unsigned char status = 0;
	status |= (unit->door & 1) << 3;
	status |= (unit->charging & 1) << 2;
	status |= (unit->night & 1) << 1;
	status |= (unit->light & 1);

	return status;
}

inline void dispenser_unpack_unit_status(unsigned char status, struct dispenser_mmap_unit *unit)
{
	if (unit) {
		unit->night = (status >> 1) & 1;
		unit->light = status & 1;
	}
}

//bitfield up,down,release,full,+enum state (4bits) (settable)
inline unsigned char dispenser_pack_slot_status(const struct dispenser_mmap_slot *state, unsigned char full)
{
	unsigned char status = 0;
	status |= (state->up & 1) << 7;
	status |= (state->down & 1) << 6;
	status |= (state->release & 1) << 5;
	status |= (full & 1) << 4;
	status |= (state->state & 0xf);

	return status;
}

inline void dispenser_unpack_slot_status(unsigned char status, struct dispenser_mmap_slot *state, unsigned char *full)
{
	if (state) {
		state->state = (enum slot_state) (status & 0xf);
#ifndef __KERNEL__
		state->up = (status >> 7) & 1;
		state->down = (status >> 6) & 1;
		state->release = (status >> 5) & 1;
#endif
	}
	if (full) {
		*full = (status >> 4) & 1;
	}
}


#endif // DISPENSER_H
