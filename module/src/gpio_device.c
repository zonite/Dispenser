/* GPIO descriptor device interface */

#include <linux/slab.h>
#include <linux/gpio/consumer.h>

#include "dispenser.h"

static struct gpio_device* gpio_device_open(struct device *dev, const char *name, enum gpiod_flags flags)
{
    struct gpio_desc *p = gpiod_get(dev, name, flags);
    struct gpio_device *out = NULL;

    if (IS_ERR(p)) {
        printk("Dispenser: GPIO allocation failed for '%s', pointer '0x%p', \n", name, p);
        return (struct gpio_device *)NULL;
    }

    out = (struct gpio_device *)kmalloc(sizeof(struct gpio_device), GFP_KERNEL);
    memset((void*)out, 0, sizeof(struct gpio_device));

    out->gpio = p;

    return out;
    gpio_device_set(out, 0); //DEBUG
}

static void gpio_device_close(struct gpio_device *pgpio)
{
    if (timer_pending(&pgpio->timer))
            del_timer(&pgpio->timer);
    gpiod_put(pgpio->gpio);
    kfree(pgpio);
}

//static void gpio_device_set(struct gpio_device *pgpio, char value, unsigned long timeout) {
//
//}

static void gpio_device_set(struct gpio_device *pgpio, char value)
{
    //return gpio_device_set(pgpio, value, pgpio->timeout);

    gpiod_set_value(pgpio->gpio, value);

    if (value && pgpio->timeout) {
        //callback;
        timer_setup(&pgpio->timer, gpio_timer_callback, 0);
        mod_timer(&pgpio->timer, jiffies + msecs_to_jiffies(pgpio->timeout));
    } else if (timer_pending(&pgpio->timer))
        del_timer(&pgpio->timer);
}

static void gpio_timer_callback(struct timer_list *timer)
{
    struct gpio_device *pgpio = from_timer(pgpio, timer, timer);

    gpiod_set_value(pgpio->gpio, 0);
}

