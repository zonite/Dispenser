/* Interrupthandler */

#include <linux/interrupt.h>

#include "dispenser.h"

//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id) {
static irqreturn_t door_irq_handler(int irq, void *dev_id) {
    if (pDispenser_mmap && cDispenser.p_sDoor && cDispenser.p_sLed) {
        printk("Door interrupt: %i -> %i\n", pDispenser_mmap->door, gpio_device_get(cDispenser.p_sDoor));
        //if (*cDispenser.p_sDoor->value) {
            //Closed

        //} else {
            //Open
        //}
        gpio_device_set_tmout(cDispenser.p_sLed, !pDispenser_mmap->door, cDispenser.p_sDoor->timeout);
    } else {
        printk("Interrupt door, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t charge_irq_handler(int irq, void *dev_id) {
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
    if (pDispenser_mmap && cDispenser.p_sButton && cDispenser.p_sLed) {
        old_val = pDispenser_mmap->button;
        new_val = gpio_device_get(cDispenser.p_sButton);

        printk("Button interrupt: %s.\n", new_val ? "pressed" : "depressed");

        if (old_val != new_val) {
            if (new_val)
                gpio_device_set(cDispenser.p_sLed, !pDispenser_mmap->light);
        }
    } else {
        printk("Interrupt charge, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

#include "dispenser.h"

