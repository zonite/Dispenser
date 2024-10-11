
#include "dispenser.h"

static int dispenser_post_event(enum eventtype type, const char *name, volatile void *data) {
    printk("post event %s\n", name);

    return 1;
}

static void dispenser_gpiod_event(struct dispenser_gpiod* dev, char new_val)
{
	int gpio_val = new_val;
	int dir = gpiod_get_direction(dev->gpiod);
	if (dir) { //0=output 1=input
		gpio_val = gpiod_get_value(dev->gpiod);
	}
	printk("GPIOD Event: set %px, %i => actual = %i, new_val = %i, dir = %i.", dev->value, *dev->value, gpio_val, new_val, dir);
	*dev->value = gpio_val;
	dev->event_handler(dev, gpio_val);
	//post_event
	//dispenser_post_event()
}

static void dispenser_door_event(struct dispenser_gpiod* dev, char closed)
{
	//static unsigned long opened = -1;

	if (dev != cDispenser.p_sDoor) {
		printk("Dispenser: Door event, GPIOD != p_sDoor, 0x%px != 0x%px\n", dev, cDispenser.p_sDoor);
	}

	if (closed) {
		//Door closed!
		printk("Door event: Closed %i -> %i. GPIO value = %i\n", !closed, closed, *dev->value);
		dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
		//if (jiffies > msecs_to_jiffies(opened + FILL_TIMEOUT))
		//	dispenser_unit_filled();
	} else {
		//Door opened!
		printk("Door event: Opened %i -> %i. GPIO value = %i\n", !closed, closed, *dev->value);
		//opened = jiffies;
		dispenser_gpiod_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);
		//dispenser_unit_locks_on();
	}
	if (cDispenser.initialized) __dispenser_genl_post_unit_status(NULL);
	dispenser_post_event(DOOR, "Door event", &pDispenser_mmap->unit.door);
}

static void dispenser_button_event(struct dispenser_gpiod* dev, char pressed)
{
    if (dev != cDispenser.p_sButton) {
	printk("Dispenser: Button event, GPIOD != p_sButton, 0x%px != 0x%px\n", dev, cDispenser.p_sButton);
    }

    if (pressed) {
	//button pressed
	printk("Button event: Pressed = %i. GPIO value = %i.\n", pressed, *dev->value);

	if (*cDispenser.p_sLed->value) {
		dispenser_unit_locks_on();
		dispenser_unit_filled();
	}

	dispenser_gpiod_set(cDispenser.p_sLed, !(*cDispenser.p_sLed->value));

	dispenser_post_event(BUTTON, "Button pressed", &pDispenser_mmap->unit.button);
    } else {
	//button depressed
	printk("Button event: Unressed, %i. GPIO value = %i.\n", pressed, *dev->value);
    }
    if (cDispenser.initialized) __dispenser_genl_post_unit_status(NULL);
}

static void dispenser_charge_event(struct dispenser_gpiod* dev, char charging)
{
    if (dev != cDispenser.p_sCharge) {
	printk("Dispenser: Charge event, GPIOD != p_sCharge, 0x%px != 0x%px\n", dev, cDispenser.p_sCharge);
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
    if (cDispenser.initialized) __dispenser_genl_post_unit_status(NULL);
}

static void dispenser_light_event(struct dispenser_gpiod* dev, char on)
{
    if (dev != cDispenser.p_sLed) {
	printk("Dispenser: Light event, GPIOD != p_sLed, 0x%px != 0x%px\n", dev, cDispenser.p_sLed);
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
    if (cDispenser.initialized) __dispenser_genl_post_unit_status(NULL);
}

static void dispenser_up_event(struct dispenser_gpiod* dev, char new_val)
{
	struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

	if (!slot) {
		printk("Dispenser: Fail, parent == NULL!\n");
		return;
	}

	enum slot_state old = slot->state->state;

	dispenser_slot_update(slot);
	if (slot->state->release && !slot->state->up) {
		//release == 1 && up == 0 -> release = 0
		//dispenser_gpiod_set_tmout(slot->release, 0, 0);
		dispenser_update_slot_status(slot->state); //update release bit
	}

	enum slot_state new = slot->state->state;
	printk("Dispenser: %s, up event new val = %i, old = %x, up=%i, dn=%i, rel=%i, new = %x, up=%i, dn=%i, rel=%i",
	       dev->gpiod->name, new_val,
	       old, (old >> 2) & 1, (old >> 1) & 1, old & 1,
	       new, (new >> 2) & 1, (new >> 1) & 1, new & 1);

	if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
}

static void dispenser_down_event(struct dispenser_gpiod* dev, char new_val)
{
	struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

	if (!slot) {
		printk("Dispenser: Fail, parent == NULL!\n");
		return;
	}

	enum slot_state old = slot->state->state;

	dispenser_slot_update(slot);
	if (slot->state->down) {
		if (slot->state->release) {
			//release == 1 && up == 0 -> release = 0
			dispenser_gpiod_set_tmout(slot->release, 0, 0);
			dispenser_update_slot_status(slot->state); //update release bit
		}

		if (slot->next && slot->next->pendingRelease)
			dispenser_unit_release_slot(slot->next, 0, 0);

		dispenser_release_event(dev, 0); //Down. Stop timer.
		slot->full = 0;
	}

	enum slot_state new = slot->state->state;
	printk("Dispenser: %s, down event new val = %i, old = %x, up=%i, dn=%i, rel=%i, new = %x, up=%i, dn=%i, rel=%i",
	       dev->gpiod->name, new_val,
	       old, (old >> 2) & 1, (old >> 1) & 1, old & 1,
	       new, (new >> 2) & 1, (new >> 1) & 1, new & 1);

	if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
}

static void dispenser_release_event(struct dispenser_gpiod* dev, char new_val)
{
    struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

    if (!slot) {
	printk("Dispenser: Fail, parent == NULL!\n");
	return;
    }

    enum slot_state old = slot->state->state;

    dispenser_slot_update(slot);
    if (new_val) {
	    if (slot->release_delayed) {
		    //pending release
		    dispenser_gpiod_set(dev, 0); //start lock timeout
		    dispenser_update_slot_status(slot->state); //update release bit
	    } else {
		    //immediate release
		    slot->pendingRelease = 0;
		    dispenser_gpiod_set(dev, 1); //Open the lock
		    dispenser_update_slot_status(slot->state); //update release bit
	    }
    } else {
	    //release timeout
	    if (slot->release_delayed) {
		    //pending release
		    slot->pendingRelease = 0;
		    slot->release_delayed = 0;
		    dispenser_gpiod_set(dev, 1); //start lock timeout
		    dispenser_update_slot_status(slot->state); //update release bit
	    } else if (slot->state->release && slot->state->up && !slot->state->down) {
		    //release failed! re-release
		    printk("Dispenser: Release %s failed, re-release!\n", dev->gpiod->name);
		    slot->pendingRelease = 0;
		    dispenser_gpiod_set(dev, 1);
	    } else {
		    //release success!
		    slot->pendingRelease = 0;
		    dispenser_gpiod_set_tmout(dev, 0, 0);
		    printk("Dispenser: Release %s success.\n", dev->gpiod->name);
	    }
    }

    enum slot_state new = slot->state->state;
    printk("Dispenser: %s, down event new val = %i, old = %x, up=%i, dn=%i, rel=%i, new = %x, up=%i, dn=%i, rel=%i",
           dev->gpiod->name, new_val,
           old, (old >> 2) & 1, (old >> 1) & 1, old & 1,
           new, (new >> 2) & 1, (new >> 1) & 1, new & 1);

    if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
}

