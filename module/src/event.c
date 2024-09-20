
#include "dispenser.h"

static int dispenser_post_event(enum eventtype type, const char *name, volatile void *data) {
    printk("post event %s\n", name);

    return 1;
}

static void dispenser_gpiod_event(struct dispenser_gpiod* dev, char new_val)
{
	printk("GPIOD Event: set %pX, %i => %i.", dev->value, *dev->value, new_val);
	*dev->value = new_val;
	dev->event_handler(dev, new_val);
	//post_event
	//dispenser_post_event()
}

static void dispenser_door_event(struct dispenser_gpiod* dev, char closed)
{
	static unsigned long opened = -1;

	if (dev != cDispenser.p_sDoor) {
		printk("Dispenser: Door event, GPIOD != p_sDoor, 0x%pX != 0x%pX\n", dev, cDispenser.p_sDoor);
	}

	if (closed) {
		//Door closed!
		printk("Door event: Closed %i -> %i. GPIO value = %i\n", !closed, closed, *dev->value);
		dispenser_gpiod_set_tmout(cDispenser.p_sLed, 0, 0);
		if (jiffies > msecs_to_jiffies(opened + FILL_TIMEOUT))
			dispenser_unit_filled();
	} else {
		//Door opened!
		printk("Door event: Opened %i -> %i. GPIO value = %i\n", !closed, closed, *dev->value);
		opened = jiffies;
		dispenser_gpiod_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);

	}
	if (cDispenser.initialized) __dispenser_genl_post_unit_status(NULL);
	dispenser_post_event(DOOR, "Door event", &pDispenser_mmap->unit.door);
}

static void dispenser_button_event(struct dispenser_gpiod* dev, char pressed)
{
    if (dev != cDispenser.p_sButton) {
	printk("Dispenser: Button event, GPIOD != p_sButton, 0x%pX != 0x%pX\n", dev, cDispenser.p_sButton);
    }

    if (pressed) {
	//button pressed
	printk("Button event: Pressed = %i. GPIO value = %i.\n", pressed, *dev->value);

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
	printk("Dispenser: Charge event, GPIOD != p_sCharge, 0x%pX != 0x%pX\n", dev, cDispenser.p_sCharge);
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
	printk("Dispenser: Light event, GPIOD != p_sLed, 0x%pX != 0x%pX\n", dev, cDispenser.p_sLed);
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

	if (new_val) { //Locked up
		if (slot->state->state == CLOSING ||
		   (slot->state->state == FAILED && slot->state->down == 0 && slot->state->release == 0)) {
			slot->state->state = CLOSED;
			slot->full = 1;
			printk("Dispenser: %s closed.\n", dev->gpiod->name);
		} else {
			slot->state->state = FAILED;
			printk("Dispenser: %s failed up: up = %i, down = %i, release = %i\n", dev->gpiod->name, slot->state->up, slot->state->down, slot->state->release);
		}
	} else { //Released
		slot->full = 0;
		if (slot->state->state == RELEASE ||
		   (slot->state->state == FAILED && slot->state->down == 0 && slot->state->release == 1)) {
			slot->state->state = OPENING;
			printk("Dispenser: %s opening.\n", dev->gpiod->name);
		} else {
			slot->state->state = FAILED;
			printk("Dispenser: %s failed up release: up = %i, down = %i, release = %i\n", dev->gpiod->name, slot->state->up, slot->state->down, slot->state->release);
		}
	}
	if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
}

static void dispenser_down_event(struct dispenser_gpiod* dev, char new_val)
{
	struct dispenser_slot_list *slot = (struct dispenser_slot_list *)dev->parent;

	if (!slot) {
		printk("Dispenser: Fail, parent == NULL!\n");
		return;
	}

	if (new_val) { //Locked down
		slot->full = 0;
		if (slot->state->state == OPENING || slot->state->state == CLOSING ||
		                (slot->state->state == FAILED && slot->state->up == 0)) {
			slot->state->state = OPEN;
			dispenser_release_event(dev, 0); //Down. Stop timer.
			printk("Dispenser: %s open.\n", dev->gpiod->name);
		} else {
			slot->state->state = FAILED;
			printk("Dispenser: %s failed down: up = %i, down = %i, release = %i\n", dev->gpiod->name, slot->state->up, slot->state->down, slot->state->release);
		}
		//If open next->inittiate
		if (slot->next && slot->next->release_delayed)
			dispenser_unit_release_slot(slot->next, 1, 0);
	} else { //Closing
		if (slot->state->state == OPEN ||
		                (slot->state->state == FAILED && slot->state->up == 0 && slot->state->release == 0)) {
			slot->state->state = CLOSING;
			printk("Dispenser: %s closing.\n", dev->gpiod->name);
		} else {
			slot->state->state = FAILED;
			printk("Dispenser: %s failed closing: up = %i, down = %i, release = %i\n", dev->gpiod->name, slot->state->up, slot->state->down, slot->state->release);
		}
	}
	if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
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
		printk("Dispenser: %s release.\n", dev->gpiod->name);
	} else {
		slot->state->state = FAILED;
		printk("Dispenser: %s failed release: up = %i, down = %i, release = %i\n", dev->gpiod->name, slot->state->up, slot->state->down, slot->state->release);
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
		printk("Dispenser: Release %s failed, re-release!\n", dev->gpiod->name);
		dispenser_gpiod_set(dev, 1);
		return;
	}
	dispenser_gpiod_set_tmout(dev, 0, 0);
	printk("Dispenser: Release %s success.\n", dev->gpiod->name);
	if (slot->initialized) __dispenser_genl_post_slot_status(slot, NULL);
    }
}

