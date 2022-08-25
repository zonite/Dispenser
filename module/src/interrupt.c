/* Interrupthandler */

#include <linux/interrupt.h>
#include <linux/jiffies.h>

#include "dispenser.h"

#define ALPHA 0.25

/*
struct ema {

}
*/

//double ema (double last, double now) {
//    return last + ALPHA * (now - last);
//}

//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id) {
static irqreturn_t door_irq_handler(int irq, void *dev_id) {
    static unsigned long last = 0;
    unsigned long flags;
    char old_door, new_door;

    if (cDispenser.p_sLed) {
        //Disable interrupts
        local_irq_save(flags);

        if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
            printk("Door: GPIO debounce too early\n");
            gpio_device_get_debounce(cDispenser.p_sDoor);
            last = jiffies;

            //Enable interrupts
            local_irq_restore(flags);

            return (irqreturn_t) IRQ_HANDLED;
        }
        last = jiffies;

        //irq_wake_thread(irq, dev_id);

        old_door = *cDispenser.p_sDoor->value;
        new_door = !old_door;
        //new_door = gpio_device_get(cDispenser.p_sDoor);

        door_event(new_door);

        //Enable interrupts
        local_irq_restore(flags);

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

    //if (pDispenser_mmap && cDispenser.p_sCharge) {
        if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
            printk("Charging: GPIO debounce too early\n");
            gpio_device_get(cDispenser.p_sCharge);

            return (irqreturn_t) IRQ_HANDLED;
        }
        last = jiffies;

        printk("Charging event charge=%i -> %i\n", pDispenser_mmap->charging, gpio_device_get(cDispenser.p_sCharge));

        post_event(CHARGE, "Charging event", &pDispenser_mmap->charging);
    //} else {
    //    printk("Interrupt charge, driver not ready\n");
    //}

    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t button_irq_handler(int irq, void *dev_id) {
    char old_val, new_val;
    static unsigned long last = 0;


    if (cDispenser.p_sLed) {
        if (last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
            printk("Button: GPIO debounce too early\n");
            gpio_device_get(cDispenser.p_sButton);

            return (irqreturn_t) IRQ_HANDLED;
        }
        last = jiffies;

        printk("Button interrupt: Begins.\n");

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
        printk("Interrupt button, driver not ready\n");
    }

    return (irqreturn_t) IRQ_HANDLED;
}

#include "dispenser.h"

