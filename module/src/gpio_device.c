/* GPIO descriptor device interface */

#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include <drivers/gpio/gpiolib.h>

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

    out->gpio = p;
    out->value = value;
    gpio_device_get(out);

    if (flags == GPIOD_IN)
        gpiod_set_debounce(out->gpio, DEBOUNCE);

    if (irq_handler) {
        int irq = gpiod_to_irq(out->gpio);

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
    if (timer_pending(&pgpio->timer))
            del_timer(&pgpio->timer);
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
    gpio_device_set_tmout(pgpio, value, pgpio->timeout);
}

static void gpio_device_set_tmout(struct gpio_switch *pgpio, char value, unsigned int tmout)
{
    gpiod_set_value(pgpio->gpio, value);

    if (value && tmout) {
        //callback;
        timer_setup(&pgpio->timer, gpio_timer_callback, 0);
        mod_timer(&pgpio->timer, jiffies + msecs_to_jiffies(tmout));
    } else if (timer_pending(&pgpio->timer))
        del_timer(&pgpio->timer);
}

static char gpio_device_get(struct gpio_switch *pgpio)
{
    char new_val = gpiod_get_value(pgpio->gpio);

    if (new_val != *pgpio->value) {

        *pgpio->value = new_val;
    }

    return *pgpio->value;
}

static void gpio_timer_callback(struct timer_list *timer)
{
    struct gpio_switch *pgpio = from_timer(pgpio, timer, timer);

    gpiod_set_value(pgpio->gpio, 0);
}

