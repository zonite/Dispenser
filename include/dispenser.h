#ifndef DISPENSER_H
#define DISPENSER_H

#define SEC_TO_MSEC(A) ((A) * 1000)
#define LIGHT_TIMEOUT SEC_TO_MSEC(5 * 60)
#define FAIL_TIMEOUT SEC_TO_MSEC(15)
#define DOOR_TIMEOUT SEC_TO_MSEC(45 * 60)

struct dispenser_ioctl {
    int cmd;
    char *name;
};

struct dispenser_mmap {
    char charging;
    char button;
    char light;
    char door;
};

#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)
#define GREETER  _IOW('a', 'c', struct dispenser_ioc *)

#endif // DISPENSER_H
