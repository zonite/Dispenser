/* Interrupthandler */

#include <linux/interrupt.h>
#include <linux/jiffies.h>

#include "dispenser.h"

//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id) {
static irqreturn_t door_irq_handler(int irq, void *dev_id) {
    static unsigned long last = 0;

    if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
        printk("Door: GPIO debounce too early\n");

        return (irqreturn_t) IRQ_HANDLED;
    }
    last = jiffies;

    if (pDispenser_mmap && cDispenser.p_sDoor && cDispenser.p_sLed) {
        char old_door = *cDispenser.p_sDoor->value, new_door = gpio_device_get(cDispenser.p_sDoor);

        if (old_door != new_door) {
            //Door value changed
            if (new_door) {
                //Door closed!
                printk("Door interrupt: Closed %i -> %i\n", old_door, new_door);
                gpio_device_set_tmout(cDispenser.p_sLed, 0, 0);
            } else {
                //Door opened!
                printk("Door interrupt: Opened %i -> %i\n", old_door, new_door);
                gpio_device_set_tmout(cDispenser.p_sLed, 1, cDispenser.p_sDoor->timeout);
            }
        }
        //if (*cDispenser.p_sDoor->value) {
            //Closed

        //} else {
            //Open
        //}
        //gpio_device_set_tmout(cDispenser.p_sLed, !pDispenser_mmap->door, cDispenser.p_sDoor->timeout);
    } else {
        printk("Interrupt door, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t charge_irq_handler(int irq, void *dev_id) {
    static unsigned long last = 0;

    if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
        printk("Charging: GPIO debounce too early\n");

        return (irqreturn_t) IRQ_HANDLED;
    }
    last = jiffies;

    if (pDispenser_mmap && cDispenser.p_sCharge) {
        printk("Charging event charge=%i -> %i\n", pDispenser_mmap->charging, gpio_device_get(cDispenser.p_sCharge));

        post_event(CHARGE, "Charging event", &pDispenser_mmap->charging);
    } else {
        printk("Interrupt charge, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t button_irq_handler(int irq, void *dev_id) {
    char old_val, new_val;
    static unsigned long last = 0;

    if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
        printk("Button: GPIO debounce too early\n");

        return (irqreturn_t) IRQ_HANDLED;
    }
    last = jiffies;

    printk("Button interrupt: Begins.\n");

    if (pDispenser_mmap && cDispenser.p_sButton && cDispenser.p_sLed) {
        old_val = pDispenser_mmap->button;
        new_val = gpio_device_get(cDispenser.p_sButton);

        printk("Button interrupt: %s.\n", new_val ? "pressed" : "depressed");

        if (old_val != new_val) {
            //Button state change
            if (new_val) {
                //Button pressed
                printk("Button: Press event. Set new value %i -> %i\n", pDispenser_mmap->light, !pDispenser_mmap->light);
                gpio_device_set(cDispenser.p_sLed, !pDispenser_mmap->light);
            } else {
                //Button depressed
                printk("Button: Depress event.\n");
            }
        }
    } else {
        printk("Interrupt charge, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

#include "dispenser.h"

