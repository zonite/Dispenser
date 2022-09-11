
#include "dispenser.h"

static int dispenser_post_event(enum eventtype type, const char *name, volatile void *data) {
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

static void dispenser_door_event(struct dispenser_gpiod* dev, char closed)
{
    if (dev != cDispenser.p_sDoor) {
        printk("Dispenser: Door event, GPIOD != p_sDoor, 0x%p != 0x%p\n", dev, cDispenser.p_sDoor);
    }

    if (closed) {
        //Door closed!
        printk("Door event: Closed %i -> %i\n", !closed, closed);
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
    } else {
        //Door opened!
        printk("Door event: Opened %i -> %i\n", !closed, closed);
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);
    }
    dispenser_post_event(DOOR, "Door event", &pDispenser_mmap->unit.door);
}

static void dispenser_button_event(struct dispenser_gpiod* dev, char pressed)
{
    if (dev != cDispenser.p_sButton) {
        printk("Dispenser: Button event, GPIOD != p_sButton, 0x%p != 0x%p\n", dev, cDispenser.p_sButton);
    }

    if (pressed) {
        //button pressed
        printk("Button event: Pressed = %i.\n", pressed);

        dispenser_gpiod_set(cDispenser.p_sLed, !(*cDispenser.p_sLed->value));

        dispenser_post_event(BUTTON, "Button pressed", &pDispenser_mmap->unit.button);
    } else {
        //button depressed
        printk("Button event: Unressed, %i.\n", pressed);
    }
}

static void dispenser_charge_event(struct dispenser_gpiod* dev, char charging)
{
    if (dev != cDispenser.p_sCharge) {
        printk("Dispenser: Charge event, GPIOD != p_sCharge, 0x%p != 0x%p\n", dev, cDispenser.p_sCharge);
    }

    if (charging) {
        //Charging
        cDispenser.p_sDoor->timeout = DOOR_TIMEOUT;

        dispenser_post_event(CHARGE, "Dispenser charging", &pDispenser_mmap->unit.charging);
    } else {
        //Not charging
        cDispenser.p_sDoor->timeout = LIGHT_TIMEOUT;

        dispenser_post_event(CHARGE, "Dispenser not charging", &pDispenser_mmap->unit.charging);
    }
}

static void dispenser_light_event(struct dispenser_gpiod* dev, char on)
{
    if (dev != cDispenser.p_sLed) {
        printk("Dispenser: Light event, GPIOD != p_sLed, 0x%p != 0x%p\n", dev, cDispenser.p_sLed);
    }

    if (on) {
        //Light switch on... Should not happen.
        dispenser_gpiod_set(cDispenser.p_sLed, 1);
        dispenser_post_event(LED, "Light on.\n", &pDispenser_mmap->unit.light);
    } else {
        //Light switch off
        dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
        dispenser_post_event(LED, "Light off.\n", &pDispenser_mmap->unit.light);
    }
}

static void dispenser_up_event(struct dispenser_gpiod* dev, char new_val)
{
    struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

    if (!slot) {
        printk("Dispenser: Fail, parent == NULL!\n");
        return;
    }

    if (new_val) { //Locked up
        if (slot->state->state == CLOSING ||
                (slot->state->state == FAILED && slot->state->down == 0 && slot->state->release == 0)) {
            slot->state->state = CLOSED;
        } else {
            slot->state->state = FAILED;
        }
    } else { //Released
        if (slot->state->state == RELEASE ||
                (slot->state->state == FAILED && slot->state->down == 0 && slot->state->release == 1)) {
            slot->state->state = OPENING;
        } else {
            slot->state->state = FAILED;
        }
    }
}

static void dispenser_down_event(struct dispenser_gpiod* dev, char new_val)
{
    struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

    if (!slot) {
        printk("Dispenser: Fail, parent == NULL!\n");
        return;
    }

    if (new_val) { //Locked down
        if (slot->state->state == OPENING ||
                (slot->state->state == FAILED && slot->state->up == 0)) {
            slot->state->state = OPEN;
        } else {
            slot->state->state = FAILED;
        }
        //If open next->inittiate
        if (slot->next)
            dispenser_unit_release_slot(slot->next);
    } else { //Closing
        if (slot->state->state == OPEN ||
                (slot->state->state == FAILED && slot->state->up == 0 && slot->state->release == 0)) {
            slot->state->state = CLOSING;
        } else {
            slot->state->state = FAILED;
        }
    }
}

static void dispenser_release_event(struct dispenser_gpiod* dev, char new_val)
{
    struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

    if (!slot) {
        printk("Dispenser: Fail, parent == NULL!\n");
        return;
    }

    if (new_val) { //Release event
        if (slot->state->state == CLOSED ||
                (slot->state->state == FAILED && slot->state->up == 1)) {
            slot->state->state = RELEASE;
        } else {
            slot->state->state = FAILED;
        }
        dispenser_gpiod_set(dev, 1);
    } else { //Release timeout
        if (slot->state->state == OPEN) {
            //Success
        } else if (slot->state->state == FAILED && slot->state->up == 0 && slot->state->down == 1) {
            //Failed, but now open
            slot->state->state = OPEN;
        } else {
            //Failed
            slot->state->state = FAILED;
        }
    }
}

