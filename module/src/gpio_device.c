/* GPIO descriptor device interface */

#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include <drivers/gpio/gpiolib.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

#include "dispenser.h"

static struct gpio_switch* gpio_device_open(struct device *dev, const char *name, enum gpiod_flags flags, irq_handler_t irq_handler, char *value)
{
    struct gpio_desc *p = gpiod_get(dev, name, flags);
    struct gpio_switch *out = NULL;

    if (IS_ERR(p)) {
        printk("Dispenser: GPIO allocation failed for '%s', pointer '0x%p', \n", name, p);
        return (struct gpio_switch *)NULL;
    }

    out = (struct gpio_switch *)kmalloc(sizeof(struct gpio_switch), GFP_KERNEL);
    memset((void*)out, 0, sizeof(struct gpio_switch));
    if (!out) {
        printk("Mem allocation failed: %s\n", name);
        gpiod_put(p);
        return out;
    }

    out->gpio = p;
    out->value = value;
    gpio_device_get(out);

    if (flags == GPIOD_IN)
        gpiod_set_debounce(out->gpio, DEBOUNCE);

    if (irq_handler) {
        int irq = gpiod_to_irq(out->gpio);
        printk("Setting irq_handler %i, %s, 0x%p\n", irq, name, out);
        //return out;

        if (request_irq(irq, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, name, out) == 0 ){
            printk("Dispenser: Mapped IRQ nr. %d to gpiod %ld, %s\n", irq, p->flags, p->name);
            out->irq_handler = irq_handler;
            out->irq_num = irq;
        } else {
            printk("Dispenser: Error requesting IRQ nr.: %d\n", irq);
            out->irq_handler = NULL;
            out->irq_num = -1;
        }
    }

    return out;
    gpio_device_set(out, 0); //DEBUG
}

static void gpio_device_close(struct gpio_switch *pgpio)
{
    printk("Close GPIO 0x%p\n", pgpio);
    if (timer_pending(&pgpio->timer)) {
        printk("Deleting pending timer!\n");
        del_timer(&pgpio->timer);
    }
    if (pgpio->irq_handler) {
        printk("Free irq %d, device %s\n", pgpio->irq_num, pgpio->gpio->name);
        free_irq(pgpio->irq_num, pgpio);
        pgpio->irq_handler = NULL;
    }
    gpiod_put(pgpio->gpio);
    kfree(pgpio);
}

//static void gpio_device_set(struct gpio_device *pgpio, char value, unsigned long timeout) {
//
//}

static void gpio_device_set(struct gpio_switch *pgpio, char value)
{
    //return gpio_device_set(pgpio, value, pgpio->timeout);
    if (pgpio)
        gpio_device_set_tmout(pgpio, value, pgpio->timeout);
}

static void gpio_device_set_tmout(struct gpio_switch *pgpio, char value, unsigned int tmout)
{
    if (!pgpio || !pgpio->gpio || !pgpio->value) {
        printk("GPIO set failed: GPIO NULL 0x%p\n", pgpio);
        return;
    }
    printk("gpio_device_set_tmout 0x%p, %hhi, %i", pgpio, value, tmout);

    gpiod_set_value(pgpio->gpio, value);
    *pgpio->value = value;

    if (value && tmout) {
        //callback;
        printk("Setup timeout %d to 0x%p.\n", tmout, pgpio);

        timer_setup(&pgpio->timer, gpio_timer_callback, 0);
        mod_timer(&pgpio->timer, jiffies + msecs_to_jiffies(tmout));

    } else if (timer_pending(&pgpio->timer)) {
        printk("Delete timer 0x%p.\n", pgpio);
        del_timer(&pgpio->timer);
    }
}

static char gpio_device_get(struct gpio_switch *pgpio)
{
    char new_val;

    if (!pgpio || !pgpio->gpio || !pgpio->value) {
        printk("GPIO set failed: GPIO NULL 0x%p\n", pgpio);
        return -1;
    }

    new_val = gpiod_get_value(pgpio->gpio);

    printk("GPIO: Get and update value %hhi -> %hhi.\n", *pgpio->value, new_val);

    if (new_val != *pgpio->value) {
        *pgpio->value = new_val;
        printk("Updated!\n");
    }

    return *pgpio->value;
}

static void gpio_timer_callback(struct timer_list *timer)
{
    struct gpio_switch *pgpio = from_timer(pgpio, timer, timer);
    printk("Timer callback on 0x%p.\n", pgpio);

    gpiod_set_value(pgpio->gpio, 0);
}

