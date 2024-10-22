/* GPIO descriptor device interface */

#include <linux/slab.h>
#include <linux/gpio/consumer.h>
//#include <drivers/gpio/gpiolib.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

#include "dispenser.h"

//static struct dispenser_gpiod* dispenser_gpiod_open(struct device *dev, const char *name, enum gpiod_flags flags, irq_handler_t irq_handler, char *value, void (*timer_callback)(struct timer_list *))
static struct dispenser_gpiod *dispenser_gpiod_open_index(struct device *dev, const char *name, unsigned int i, enum gpiod_flags flags)
{
	struct gpio_desc *p = gpiod_get_index(dev, name, i, flags);
	struct dispenser_gpiod *out = NULL;
	int irq;

	if (IS_ERR(p)) {
		printk("Dispenser: GPIO allocation failed for '%s', pointer '0x%px', \n", name, p);
		return (struct dispenser_gpiod *)NULL;
	}

	out = (struct dispenser_gpiod *)kzalloc(sizeof(struct dispenser_gpiod), GFP_KERNEL);
	//out = (struct dispenser_gpiod *)kmalloc(sizeof(struct dispenser_gpiod), GFP_KERNEL);
	//memset((void*)out, 0, sizeof(struct dispenser_gpiod));
	if (!out) {
		printk("Mem allocation failed: %s\n", name);
		gpiod_put(p);
		return out;
	}

	out->event_handler = dispenser_null_event;
	out->gpiod = p;
	out->value = &out->value_priv;

	if (flags & GPIOD_FLAGS_BIT_DIR_OUT) {
		//Output
		printk("Dispenser GPIOD output %d\n", flags);

		*out->value = ((flags & GPIOD_FLAGS_BIT_DIR_VAL) != 0);
		if (gpiod_direction_output(p, *out->value)) {
			//GPIOD error
			printk("GPIOD error.\n");
			kfree(out);
			gpiod_put(p);
			return NULL;
		}

		timer_setup(&out->timer, dispenser_gpiod_out_tmr_callback, 0);

		out->irq_num = -1;
		*out->value = gpiod_get_value(p);
	} else {
		//Input
		printk("Dispenser GPIOD input %d\n", flags);

		if (gpiod_direction_input(p)) {
			//GPIOD error
			printk("GPIOD error.\n");
			kfree(out);
			gpiod_put(p);
			return NULL;
		}
		*out->value = gpiod_get_value(p);
		//dispenser_gpiod_get(out);

		timer_setup(&out->timer, dispenser_gpiod_tmr_callback, 0);

		irq = gpiod_to_irq(out->gpiod);
		printk("Setting irq_handler %i, %s, 0x%px\n", irq, name, out);
		if (request_irq(irq, dispenser_gpiod_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, name, out) == 0 ){
			//if (request_irq(irq, irq_handler, IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW, name, out) == 0 ){
			printk("Dispenser: Mapped IRQ nr. %d to gpiod %ld, %s\n", irq, p->flags, p->name);
			//out->irq_handler = irq_handler;
			out->irq_num = irq;
		} else {
			printk("Dispenser: Error requesting IRQ nr.: %d\n", irq);
			//out->irq_handler = NULL;
		}
	}


	//out->value = value;

	//if (flags == GPIOD_IN)
	//    gpiod_set_debounce(out->gpiod, DEBOUNCE);

	//out->timer_callback = dispenser_gpiod_tmr_callback;
	//timer_setup(&out->timer, out->timer_callback, 0);

	//if (flags == GPIOD_IN)
	//if (irq_handler) {
	//return out;

	//}

	return out;
	dispenser_gpiod_set(out, 0); //DEBUG
}

static void dispenser_gpiod_close(struct dispenser_gpiod *pgpiod)
{
	printk("Close %s GPIO 0x%px\n", pgpiod->gpiod->name, pgpiod);
	if (timer_pending(&pgpiod->timer)) {
		printk("Deleting pending timer!\n");
		del_timer(&pgpiod->timer);
	}
	if (pgpiod->irq_num > 0) {
		printk("Free irq %d, device %s\n", pgpiod->irq_num, pgpiod->gpiod->name);
		free_irq(pgpiod->irq_num, pgpiod);
		//pgpiod->irq_handler = NULL;
	}
	gpiod_put(pgpiod->gpiod);
	kfree(pgpiod);
}

static void dispenser_gpiod_rename(struct dispenser_gpiod *pgpiod, char *name)
{
	gpiod_set_consumer_name(pgpiod->gpiod, name);
	//pgpiod->gpiod->label = name;
}

//static void gpio_device_set(struct gpio_device *pgpio, char value, unsigned long timeout) {
//
//}

static void dispenser_gpiod_set(struct dispenser_gpiod *pgpiod, char value)
{
    //return gpio_device_set(pgpio, value, pgpio->timeout);
    if (pgpiod)
	dispenser_gpiod_set_tmout(pgpiod, value, pgpiod->timeout);
    else
	printk("GPIO set failed: GPIO NULL 0x%px\n", pgpiod);
}

static void dispenser_gpiod_set_tmout(struct dispenser_gpiod *pgpiod, char value, unsigned int tmout)
{
    if (!pgpiod) {
	printk("GPIO set failed: GPIO NULL 0x%px\n", pgpiod);
	return;
    }
    printk("gpio_device_set_tmout 0x%px, %hhi, %i\n", pgpiod, value, tmout);

    gpiod_set_value(pgpiod->gpiod, value);
    *pgpiod->value = value;


    dispenser_gpiod_reset_timer(pgpiod, tmout);
}

static void dispenser_gpiod_reset_timer(struct dispenser_gpiod *pgpiod, unsigned int tmout)
{
    if (!pgpiod) {
	printk("GPIO set failed: GPIO NULL 0x%px\n", pgpiod);
	return;
    }

    if (tmout) {
	//callback;
	printk("Setup timeout %d to %s.\n", tmout, pgpiod->gpiod->name);

	mod_timer(&pgpiod->timer, jiffies + msecs_to_jiffies(tmout));
    } else if (timer_pending(&pgpiod->timer)) {
	printk("Delete timer 0x%px.\n", pgpiod);
	del_timer(&pgpiod->timer);
    }
}

static int dispenser_gpiod_read_value(struct dispenser_gpiod *pgpiod)
{
	if (!pgpiod) {
	    printk("GPIO read failed: GPIO NULL 0x%px\n", pgpiod);
	    return 0;
	}

	int val = 0;

	val = gpiod_get_value(pgpiod->gpiod);

	if (val != *pgpiod->value) {
		printk("Dispenser: %s actual value differs from cache. UPDATING -> event!\n", pgpiod->gpiod->name);
		dispenser_gpiod_event(pgpiod, val);
		return 1;
	}

	return 0;
}

/*
static char dispenser_gpiod_get(struct dispenser_gpiod *pgpiod)
{
    char new_val;

    if (!pgpiod || !pgpiod->gpiod || !pgpiod->value) {
	printk("GPIO set failed: GPIO NULL 0x%px\n", pgpiod);
	return -1;
    }

    new_val = gpiod_get_value(pgpiod->gpiod);

    printk("GPIO: Get and update value %hhi -> %hhi.\n", *pgpiod->value, new_val);

    if (new_val != *pgpiod->value) {
	*pgpiod->value = new_val;
	printk("Updated!\n");
    }

    return *pgpiod->value;
}
*/
/*
static char dispenser_gpiod_get_debounce(struct dispenser_gpiod *pgpiod)
{
    if (!pgpiod || !pgpiod->gpiod) {
	printk("GPIO set failed: GPIO NULL 0x%px\n", pgpiod);
	return -1;
    }
    mod_timer(&pgpiod->timer, jiffies + msecs_to_jiffies(INT_DEBOUNCE));

    return gpiod_get_value(pgpiod->gpiod);
}
*/

static void dispenser_gpiod_out_tmr_callback(struct timer_list *timer)
{
    struct dispenser_gpiod *pgpiod = from_timer(pgpiod, timer, timer);
    printk("Timer callback on %s.\n", pgpiod->gpiod->name);

    if (pgpiod->value) {
	dispenser_gpiod_event(pgpiod, 0);
    }
}


static void dispenser_gpiod_tmr_callback(struct timer_list *timer)
{
	struct dispenser_gpiod *pgpiod = from_timer(pgpiod, timer, timer);
	printk("Timer callback on 0x%px.\n", pgpiod);

	if (pgpiod->last + msecs_to_jiffies(INT_DEBOUNCE) > jiffies) {
		printk("Door: GPIO debounce too early\n");
		//dispenser_gpiod_get_debounce(cDispenser.p_sDoor);
		dispenser_gpiod_reset_timer(pgpiod, INT_DEBOUNCE);
	} else {
		char new_val = gpiod_get_value(pgpiod->gpiod);

		if (new_val != *pgpiod->value) {
			dispenser_gpiod_event(pgpiod, new_val);
		}
		dispenser_gpiod_reset_timer(pgpiod, POLL_INTERVAL);
	}
	pgpiod->last = jiffies;

	//dispenser_gpiod_set(pgpiod, 0);
}

static void dispenser_gpiod_set_pointer(struct dispenser_gpiod *pgpiod, volatile char *p)
{
	if (!pgpiod)
		return;

	volatile char *old = pgpiod->value;

	//printk("dispenser_gpiod_set_pointer: if");
	if (p)
		pgpiod->value = p;
	else
		pgpiod->value = &pgpiod->value_priv;

	*pgpiod->value = gpiod_get_value(pgpiod->gpiod);

	//printk("dispenser_gpiod_set_pointer: sec if");
	if (old && *old != *pgpiod->value)
		printk("GPIOparent %px value changed during value pointer update %u => %u.", pgpiod->parent, *old, *pgpiod->value);
}

/*
static void gpio_timer_door(struct timer_list *timer)
{
    char old_door, new_door;
    struct dispenser_gpiod *pgpiod = from_timer(pgpiod, timer, timer);

    printk("Door timer callback on 0x%px.\n", pgpiod);

    old_door = *pgpiod->value;
    new_door = dispenser_gpiod_get(pgpiod);

    if (old_door != new_door)
	dispenser_door_event(new_door);
}
*/

