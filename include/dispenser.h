#ifndef DISPENSER_H
#define DISPENSER_H

#include <linux/types.h>

#define SEC_TO_MSEC(A) ((A) * 1000)
//#define LIGHT_TIMEOUT SEC_TO_MSEC(5 * 60)
//#define FAIL_TIMEOUT SEC_TO_MSEC(15)
//#define DOOR_TIMEOUT SEC_TO_MSEC(45 * 60)
#define LIGHT_TIMEOUT SEC_TO_MSEC(3)
#define FAIL_TIMEOUT SEC_TO_MSEC(12)
#define DOOR_TIMEOUT SEC_TO_MSEC(6)

struct dispenser_ioctl {
    int cmd;
    char *name;
};

struct dispenser_mmap {
    char charging;
    char button;
    char light;
    char door;
    char cols;
    __u16 cols_offset;
};

struct dispenser_column {
    char slots;
    __u16 slots_offset;
};

struct dispenser_slot {
    char state;
    char up;
    char down;
    char release;
};

#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)
#define GREETER  _IOW('a', 'c', struct dispenser_ioc *)
#define DISPENSERIOCTL _IO(0xB4, 0x20)

#endif // DISPENSER_H
