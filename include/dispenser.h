#ifndef DISPENSER_H
#define DISPENSER_H

#include <linux/types.h>

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

/**
 * @brief The DISPENSER_GENL_ATTRIBUTE enum Netlink message payload
 */

enum DISPENSER_GENL_ATTRIBUTE {
	DISPENSER_GENL_UNSPEC,
	DISPENSER_SLOT_STATE,
	__DISPENSER_GENL_ATTR_END,
};

enum DISPENSER_GENL_COMMAND {
	__DISPENSER_GENL_CMD_END,
};

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
	volatile char charging;
	volatile char button;
	volatile char light;
	volatile char door;
	char cols;
	char slots;
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

#endif // DISPENSER_H
