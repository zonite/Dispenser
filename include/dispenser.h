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
	DISPENSER_GENL_CALIBRATION, //calibration data
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
	DISPENSER_GENL_CMD_SLOT_STATUS, //u8 col, u8 slot, u8 state attr
	DISPENSER_GENL_CMD_UNIT_STATUS, //u8 col, u8 slot, u8 state attr
	DISPENSER_GENL_CMD_ENVIRONMENT, //u32 attr //raw temperature
	DISPENSER_GENL_CMD_CALIBRATION, //calibration data
	DISPENSER_GENL_CMD_DUMP, //Dumps the mmap-area
	__DISPENSER_GENL_CMD_MAX,
} __attribute__ ((__packed__));

//#define DISPENSER_GENL_CMD_ENUM (__DISPENSER_GENL_CMD_MAX)
#define DISPENSER_GENL_CMD_MAX (__DISPENSER_GENL_CMD_MAX - 1)

enum slot_state {
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
	char night;
	char ncols;
	char nslots;
};

struct dispenser_mmap_column {
	char col_id;
	char slot_count;
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
inline unsigned char dispenser_pack_unit_status(struct dispenser_mmap_unit *unit)
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
inline unsigned char dispenser_pack_slot_status(struct dispenser_mmap_slot *state, unsigned char full)
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
	}
	if (full) {
		*full = (status >> 4) & 1;
	}
}


#endif // DISPENSER_H
