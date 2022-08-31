
#include "dispenser.h"

static int dispenser_post_event(enum eventtype type, const char *name, volatile void *data) {
    printk("post event %s\n", name);

    return 1;
}

static void dispenser_gpiod_event(struct dispenser_gpiod* dev, char new_val)
{
    *dev->value = new_val;
    dev->event_handler(new_val);
    //post_event
    //dispenser_post_event()
}

static void dispenser_door_event(char closed)
{
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

static void dispenser_button_event(char pressed)
{
    if (pressed) {
        //button pressed
        printk("Button event: Pressed = %i.\n", pressed);

        dispenser_gpiod_set(cDispenser.p_sLed, !(*cDispenser.p_sLed->value));

        dispenser_post_event(BUTTON, "Button pressed", &pDispenser_mmap->button);
    } else {
        //button depressed
        printk("Button event: Unressed, %i.\n", pressed);
    }
}

static void dispenser_charge_event(char charging)
{
    if (charging) {
        //Charging
        cDispenser.p_sDoor->timeout = DOOR_TIMEOUT;

        dispenser_post_event(CHARGE, "Dispenser charging", &pDispenser_mmap->charging);
    } else {
        //Not charging
        cDispenser.p_sDoor->timeout = LIGHT_TIMEOUT;

        dispenser_post_event(CHARGE, "Dispenser not charging", &pDispenser_mmap->charging);
    }
}

static void dispenser_light_event(char on)
{
    if (on) {
        //Light switch on... Should not happen.
        dispenser_gpiod_set(cDispenser.p_sLed, 1);
        dispenser_post_event(LED, "Light on.\n", &pDispenser_mmap->light);
    } else {
        //Light switch off
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
        dispenser_post_event(LED, "Light off.\n", &pDispenser_mmap->light);
    }
}
