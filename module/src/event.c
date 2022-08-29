
#include "dispenser.h"

static int dispenser_post_event(enum eventtype type, const char *name, void *data) {
    printk("post event %s\n", name);

    return 1;
}

static void dispenser_gpiod_event(struct dispenser_gpiod* dev, char new_val)
{
    *dev->value = new_val;
    dev->event_handler(dev, new_val);
    //post_event
    //dispenser_post_event()
}

static void door_event (char closed) {
    if (closed) {
        //Door closed!
        printk("Door event: Closed %i -> %i\n", !closed, closed);
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
    } else {
        //Door opened!
        printk("Door event: Opened %i -> %i\n", !closed, closed);
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);
    }
    dispenser_post_event(DOOR, "Door event", &pDispenser_mmap->door);
}
