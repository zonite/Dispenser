
#include "dispenser.h"

int post_event(enum eventtype type, const char *name, void *data) {
    printk("post event %s\n", name);

    return 1;
}

void door_event (char closed) {
    if (closed) {
        //Door closed!
        printk("Door event: Closed %i -> %i\n", !closed, closed);
        gpio_device_set_tmout(cDispenser.p_sLed, 0, 0);
    } else {
        //Door opened!
        printk("Door event: Opened %i -> %i\n", !closed, closed);
        gpio_device_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);
    }
    post_event(DOOR, "Door event", &pDispenser_mmap->door);
}
