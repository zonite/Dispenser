/* Interrupthandler */

#include <linux/interrupt.h>

//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id) {
static irqreturn_t door_irq_handler(int irq, void *dev_id) {
    printk("interrupt");
    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t charge_irq_handler(int irq, void *dev_id) {
    printk("interrupt");
    return (irqreturn_t) IRQ_HANDLED;
}

static irqreturn_t button_irq_handler(int irq, void *dev_id) {
    printk("interrupt");
    return (irqreturn_t) IRQ_HANDLED;
}

#include "dispenser.h"

