#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
//#include <linux/gpio/machine.h>
#include <drivers/gpio/gpiolib-of.h>

#include "dispenser.h"

//#define GPIO_LOOKUP_FLAGS_DEFAULT  ((0 << 0)| (0 << 3))

static char UP_TEXT[] = "Dispenser[  ].up";
static char DOWN_TEXT[] = "Dispenser[  ].down";
static char RELEASE_TEXT[] = "Dispenser[  ].release";

static int dispenser_unit_init(struct device *dev) {
    int i = 0, n = 0;
    //, k = -1;
    //unsigned char col = 0;
    //struct device_node *unit = of_get_child_by_name(dev->of_node, DEVICE_UNIT);
    struct dispenser_slot_list *slot_list = NULL, *s = NULL;
    struct dispenser_col_list *col_list = NULL, *col_iterator;
    unsigned char *slots = NULL, *cols = NULL;
    //struct gpio_descs *up = NULL, *down = NULL, *release = NULL;

    i = device_property_count_u8(dev, "cols");
    if (device_property_count_u8(dev, "slots") != i) {
	printk("Dispenser: Incorrect count slots\n");
	return FAIL;
    }

    //i = i * 3; //gpios have 3 per property

    if (device_property_count_u32(dev, "up-gpio") != i * 3) {
	printk("Dispenser: Incorrect count up-gpio %i != %i\n", device_property_count_u32(dev, "up-gpio"), i);
	return FAIL;
    }

    if (device_property_count_u32(dev, "down-gpio") != i * 3) {
	printk("Dispenser: Incorrect count down-gpio\n");
	return FAIL;
    }

    if (device_property_count_u32(dev, "release-gpio") != i * 3) {
	printk("Dispenser: Incorrect count release-gpio\n");
	return FAIL;
    }

    slot_list = (struct dispenser_slot_list *)kzalloc(sizeof(struct dispenser_slot_list) * i, GFP_KERNEL);
    if (!slot_list) {
	printk("Mem allocation failed: slot_list.\n");
	return FAIL;
    }

    slots = (unsigned char *)kmalloc(sizeof(unsigned char) * i, GFP_KERNEL);
    if (!slots) {
	printk("Mem allocation failed: slots.\n");
	kfree(slot_list);
	return FAIL;
    }

    col_list = (struct dispenser_col_list *)kzalloc(sizeof(struct dispenser_col_list), GFP_KERNEL);
    if (!col_list) {
	printk("Mem allocation failed: slot_list.\n");
	kfree(slot_list);
	kfree(slots);
	return FAIL;
    }
    printk("0 column 0x%p\n", col_list);

    cols = (unsigned char *)kzalloc(sizeof(unsigned char) * i, GFP_KERNEL);
    if (!cols) {
	printk("Mem allocation failed: slots.\n");
	kfree(col_list);
	kfree(slot_list);
	kfree(slots);
	return FAIL;
    }

    if (device_property_read_u8_array(dev, "slots", slots, i)) {
	printk("Array read failed: slots.\n");
	kfree(col_list);
	kfree(slot_list);
	kfree(slots);
	kfree(cols);
	return FAIL;
    }

    if (device_property_read_u8_array(dev, "cols", cols, i)) {
	printk("Array read failed: cols.\n");
	kfree(col_list);
	kfree(slot_list);
	kfree(slots);
	kfree(cols);
	return FAIL;
    }

    col_list->col_name = cols[0];
    //col_list->first = slot_list;
    col_iterator = col_list;
    cDispenser.col_count = 1;
    cDispenser.cols = col_list;

    for (n = 0; n < i; ++n ) {
	//slots[n].up = gpiod_get_index(dev, "up", n, GPIOD_IN);
	if (col_iterator->col_name != cols[n]) {
		//Different column:
		printk("Different column, name = %i != cols[%i] = %i, id = %i, slots = %i\n", col_iterator->col_name, n, cols[n], col_iterator->col_id, col_iterator->slot_count);
	    col_iterator = col_list; //Reset to first column
	    do { //Iterate columns
		if (!col_iterator->next) { //col_iterator == NULL : no column found! End of list!
			col_iterator->next = (struct dispenser_col_list *)kzalloc(sizeof(struct dispenser_col_list), GFP_KERNEL);
		    col_iterator->next->prev = col_iterator; //Double linking

		    if (!col_iterator->next) {
			printk("Mem allocation failed: slot_list.\n");
			dispenser_unit_close();
			kfree(slots);
			kfree(cols);
			return FAIL;
		    }

		    printk("%i column 0x%p\n", col_iterator->col_id + 1, col_iterator->next);

		    col_iterator->next->col_name = cols[n];
		    col_iterator->next->col_id = col_iterator->col_id + 1;
		    ++cDispenser.col_count;

		    printk("Finished column name = %i, id = %i, slots = %i\n", col_iterator->col_name, col_iterator->col_id, col_iterator->slot_count);
		}
		col_iterator = col_iterator->next;
		printk("New column name = %i, id = %i, slots = %i\n", col_iterator->col_name, col_iterator->col_id, col_iterator->slot_count);
	    } while (col_iterator->col_name != cols[n]);
	}

	//Slot list linking:
	if (col_iterator->first) {
		s = col_iterator->first;

	    while (s->next) {
		s = s->next;
		if (s == s->next) {
			printk("Error: ring linking! 0x%p\n", s);
		    s->next = NULL;
		}
	    }

	    //s points to last slot.
	    s->next = &slot_list[n];
	    slot_list[n].prev = s;
	    slot_list[n].slot_id = s->slot_id + 1;
	} else {
		col_iterator->first = &slot_list[n];
	}

	++col_iterator->slot_count;

	//slot_list[n].col = cols[n];
	slot_list[n].slot_name = slots[n];
	slot_list[n].column = col_iterator;
	slot_list[n].slot_num = n;

	UP_TEXT[10] = '0' + col_iterator->col_id;
	UP_TEXT[11] = '0' + slot_list[n].slot_id;
	DOWN_TEXT[10] = '0' + col_iterator->col_id;
	DOWN_TEXT[11] = '0' + slot_list[n].slot_id;
	RELEASE_TEXT[10] = '0' + col_iterator->col_id;
	RELEASE_TEXT[11] = '0' + slot_list[n].slot_id;

	slot_list[n].up = dispenser_gpiod_open_index(dev, "up", n, GPIOD_IN);
	slot_list[n].down = dispenser_gpiod_open_index(dev, "down", n, GPIOD_IN);
	slot_list[n].release = dispenser_gpiod_open_index(dev, "release", n, GPIOD_OUT_LOW);

	if (!slot_list[n].up || !slot_list[n].down || !slot_list[n].release) {
		printk("GPIOD allocation failed.\n");

	    dispenser_unit_close();
	    kfree(slots);
	    kfree(cols);

	    return FAIL;
	}

	slot_list[n].up->timeout = cDispenser.iFailTimeout;
	slot_list[n].up->event_handler = dispenser_up_event;
	slot_list[n].up->parent = &slot_list[n];

	slot_list[n].down->timeout = cDispenser.iFailTimeout;
	slot_list[n].down->event_handler = dispenser_down_event;
	slot_list[n].down->parent = &slot_list[n];

	slot_list[n].release->timeout = cDispenser.iFailTimeout;
	slot_list[n].release->event_handler = dispenser_release_event;
	slot_list[n].release->parent = &slot_list[n];

	dispenser_gpiod_rename(slot_list[n].up, UP_TEXT);
	dispenser_gpiod_rename(slot_list[n].down, DOWN_TEXT);
	dispenser_gpiod_rename(slot_list[n].release, RELEASE_TEXT);

	//if (cols[n] == cols[(n + 1) % n]) {
	//    slot_list[n].next = &slot_list[n + 1];
	//    slot_list[n + 1].prev = &slot_list[n];
	//}
	//slot_list[n].
	s = &slot_list[n];
	printk("New slot[%i] pos %i%i (0x%p): name = %i, id = %i, slot_num = %i, prev 0x%p, next 0x%p\n", n, s->column->col_id, s->slot_id, s, s->slot_name, s->slot_id, s->slot_num, s->prev, s->next);
    }

    slot_list[0].prev = NULL;
    slot_list[i - 1].next = NULL;

    cDispenser.slot_count = i;

    kfree(slots);
    kfree(cols);

    printk("Unit Finished: col 0 = 0x%p, col 1 = 0x%p, prev = 0x%p, col_ite 0x%p (%i, %i), col_ite_prev 0x%p, col_ite_next 0x%p\n", cDispenser.cols, cDispenser.cols->next, cDispenser.cols->prev, col_iterator, col_iterator->col_id, col_iterator->col_name, col_iterator->prev, col_iterator->next);

    return SUCCESS;
}

static void dispenser_unit_mmap_set(void)
{
	struct dispenser_col_list *c = cDispenser.cols;

	pDispenser_mmap->unit.cols = cDispenser.col_count;
	pDispenser_mmap->unit.slots = cDispenser.slot_count;

	/*
	if (cDispenser.p_sLed)
		cDispenser.p_sLed->value = &pDispenser_mmap->unit.light;
	if (cDispenser.p_sDoor)
		cDispenser.p_sDoor->value = &pDispenser_mmap->unit.door;
	if (cDispenser.p_sCharge)
		cDispenser.p_sCharge->value = &pDispenser_mmap->unit.charging;
	if (cDispenser.p_sButton)
		cDispenser.p_sButton->value = &pDispenser_mmap->unit.button;
	*/

	dispenser_gpiod_set_pointer(cDispenser.p_sLed, &pDispenser_mmap->unit.light);
	dispenser_gpiod_set_pointer(cDispenser.p_sDoor, &pDispenser_mmap->unit.door);
	dispenser_gpiod_set_pointer(cDispenser.p_sCharge, &pDispenser_mmap->unit.charging);
	dispenser_gpiod_set_pointer(cDispenser.p_sButton, &pDispenser_mmap->unit.button);

	while (c) {
		struct dispenser_slot_list *s = c->first;
		struct dispenser_mmap_column *col_mmap = &pDispenser_mmap[MMAP_COL(c->col_id)].column;

		col_mmap->col_id = c->col_id;
		col_mmap->slot_count = c->slot_count;

		while (s) {
			s->state = &pDispenser_mmap[MMAP_SLOT(s->slot_num)].slot;
			//s->up->value = &s->state->up;
			//s->down->value = &s->state->down;
			//s->release->value = &s->state->release;
			dispenser_gpiod_set_pointer(s->up, &s->state->up);
			dispenser_gpiod_set_pointer(s->down, &s->state->down);
			dispenser_gpiod_set_pointer(s->release, &s->state->release);

			if (s->up->value && s->down->value)
				s->state->state = FAILED;
			else if (s->up->value)
				s->state->state = CLOSED;
			else if (s->down->value)
				s->state->state = OPEN;
			else
				s->state->state = CLOSING;

			if (s->state->state == OPEN)
				s->full = 1;

			s = s->next;
		}

		c = c->next;
	}
}

static void dispenser_unit_mmap_reset(void)
{
	static struct dispenser_mmap_slot state = { 0 };
	//static char n = 0;
	struct dispenser_col_list *c = cDispenser.cols;

	/*
	if (cDispenser.p_sLed)
		cDispenser.p_sLed->value = &n;
	if (cDispenser.p_sDoor)
		cDispenser.p_sDoor->value = &n;
	if (cDispenser.p_sCharge)
		cDispenser.p_sCharge->value = &n;
	if (cDispenser.p_sButton)
		cDispenser.p_sButton->value = &n;
	*/

	dispenser_gpiod_set_pointer(cDispenser.p_sLed, NULL);
	dispenser_gpiod_set_pointer(cDispenser.p_sDoor, NULL);
	dispenser_gpiod_set_pointer(cDispenser.p_sCharge, NULL);
	dispenser_gpiod_set_pointer(cDispenser.p_sButton, NULL);

	while (c) {
		struct dispenser_slot_list *s = c->first;

		while (s) {
			s->state = &state;

			/*
			s->up->value = &s->state->up;
			s->down->value = &s->state->down;
			s->release->value = &s->state->release;
			*/
			dispenser_gpiod_set_pointer(s->up, NULL);
			dispenser_gpiod_set_pointer(s->down, NULL);
			dispenser_gpiod_set_pointer(s->release, NULL);

			s = s->next;
		}

		c = c->next;
	}
}

static void dispenser_unit_close()
{
    struct dispenser_col_list *c = cDispenser.cols, *t;
    struct dispenser_slot_list *slots = NULL, *u = NULL;

    printk("Closing unit, cols = %i, slots = %i\n", cDispenser.col_count, cDispenser.slot_count);

    cDispenser.slot_count = 0;
    cDispenser.col_count = 0;
    cDispenser.cols = NULL;

    if (c)
	slots = c->first;

    while (c) {
	struct dispenser_slot_list *s = c->first;
	c->first = NULL;

	printk("Close column name = %i, id = %i, slots = %i\n", c->col_name, c->col_id, c->slot_count);

	while (s) {
		printk("Close slot pos %i%i: name = %i, id = %i, slot_num = %i\n", s->column->col_id, s->slot_id, s->slot_name, s->slot_id, s->slot_num);

		if (s->up)
			dispenser_gpiod_close(s->up);
		if (s->down)
			dispenser_gpiod_close(s->down);
		if (s->release)
			dispenser_gpiod_close(s->release);

		u = s;
		s = s->next;
		u->next = NULL;
	}

	t = c->next;
	kfree(c);
	c = t;
    }

    if (slots)
	kfree(slots);
}

static char dispenser_unit_get_full(void)
{
	struct dispenser_col_list *c = cDispenser.cols;
	char count = 0;

	while (c) {
		count += dispenser_unit_get_full_column(c);
	}

	return count;
}

static char dispenser_unit_get_full_column(struct dispenser_col_list *c)
{
	struct dispenser_slot_list *s = c->first;
	char count = 0;

	while (s) {
		count += s->full;
	}

	return count;
}

static void dispenser_unit_release_count(char count, char force)
{
	struct dispenser_col_list **c = NULL, *ci = NULL;
	struct dispenser_slot_list **s = NULL;
	char count_avail = 0, *col_count = NULL;
	int i = 0;

	count_avail = dispenser_unit_get_full();

	if (count >= count_avail)
		return dispenser_unit_release_all(force);

	count_avail = 0;

	c = kzalloc(sizeof(c) * cDispenser.col_count, GFP_KERNEL);
	s = kzalloc(sizeof(s) * cDispenser.col_count, GFP_KERNEL);
	col_count = kzalloc(sizeof(char) * cDispenser.col_count, GFP_KERNEL);

	ci = cDispenser.cols;

	while (ci && i < cDispenser.col_count) {
		c[i] = ci;
		s[i] = ci->first;
		ci = ci->next;
		++i;
	}

	do {
		ci = NULL;
		for (i = 0; i < cDispenser.col_count; ++i) {
			if (s[i]) {
				col_count[i] += s[i]->full;
				count_avail += s[i]->full;
				s[i] = s[i]->next;
				ci = c[i];
			}
		}
	} while (count_avail < count && ci);

	for (i = 0; i < cDispenser.col_count; ++i) {
		dispenser_unit_release_column(c[i], col_count[i], force);
	}

	kfree(c);
	kfree(s);
	kfree(col_count);
}

static void dispenser_unit_release(char column, char slot)
{
	struct dispenser_slot_list *s = NULL;

	s = dispenser_unit_get(column, slot);

	if (s) {
		dispenser_unit_release_slot(s, 1, 1);
		printk("Dispenser: release column %i and slot %i\n", column, slot);
	}
}

static void dispenser_unit_release_all(char force)
{
	struct dispenser_col_list *c = cDispenser.cols;

	while (c) {
		dispenser_unit_release_column(c, c->slot_count, force);
		c = c->next;
	}
}

static struct dispenser_slot_list *dispenser_unit_get(char column, char slot)
{
	struct dispenser_col_list *c = NULL;

	c = dispenser_unit_get_column(column);

	if (c) {
		printk("Dispenser: column %i and slot %i found\n", column, slot);
		return dispenser_unit_get_slot(c, slot);
	}

	printk("Dispenser: column %i and slot %i not found\n", column, slot);

	return NULL;
}

static struct dispenser_col_list *dispenser_unit_get_column(char column)
{
	struct dispenser_col_list *c = cDispenser.cols;
	int i = 0;

	while (c) {
		if (i == column) {
			printk("Dispenser: column %i found\n", column);
			return c;
		}
		++i;
		c = c->next;
	}

	printk("Dispenser: column %i not found\n", column);

	return NULL;
}

static struct dispenser_slot_list *dispenser_unit_get_slot(struct dispenser_col_list *colunm, char slot)
{
	struct dispenser_slot_list *s = colunm->first;
	int i = 0;

	while (s) {
		if (i == slot) {
			printk("Dispenser: slot %i found\n", slot);
			return s;
		}
		++i;
		s = s->next;
	}

	printk("Dispenser: slot %i not found\n", slot);

	return NULL;
}

static void dispenser_unit_release_column(struct dispenser_col_list *col, char slots, char force)
{
	struct dispenser_slot_list *slot = col->first;

	if (!col || !slot) {
		printk("Dispenser: Error release NULL, col = 0x%p, slot = 0x%p\n", col, slot);
		return;
	}

	dispenser_unit_release_slot(slot, slots, force);
}

static void dispenser_unit_release_slot(struct dispenser_slot_list *slot, char count, char force)
{
	//dispenser_gpiod_set(slot->release, 1);
	unsigned char prev_full = 0, next_count = count;

	if (slot->prev && !force)
		prev_full = slot->prev->full;

	if (slot->full)
		--next_count;

	if (prev_full) { //Delayed release
		slot->release_delayed = 1;
		force = 0;
	} else { //Immediate release
		slot->release_delayed = 0;
		dispenser_release_event(slot->release, 1);
	}

	if (next_count > 0 && slot->next) {
		dispenser_unit_release_slot(slot->next, next_count, force);
	}
}

static void dispenser_unit_filled(void)
{
	struct dispenser_slot_list *slots = cDispenser.cols->first, *s;
	int i = 0;

	for (i = 0; i < cDispenser.slot_count; ++i) {
		s = &slots[i];

		s->full = 1;
		s->state->state = CLOSED;

		if (s->state->up)
			s->state->up_failed = 0;
		else
			s->state->up_failed = 1;

		if (s->state->down)
			s->state->down_failed = 1;
		else
			s->state->down_failed = 0;
	}
}

static void dispenser_unit_slot_failed(struct dispenser_slot_list *s)
{
	s->state->state = FAILED;
}

/*
{


    if (unit) {
	int cols = of_get_child_count(unit);
	struct device_node *col = NULL;

	//ret = device_property_count_u8(dev, "cols"); // count array elements


	printk("Dispenser found %d columns\n", cols);
	while((col = of_get_next_child(unit, col))) {
	    int slots;
	    const int *colnum;
	    struct device_node *slot = NULL;

	    printk("Processing child 0x%p in unit 0x%p\n", col, unit);

	    if (!col) {
		printk("Error, NULL col child pointer\n");
		break;
	    }

	    if (++i > 100) {
		printk("Iteration limit reached! i == %i, col = 0x%p\n", i, col);
		break;
	    }

	    slots = of_get_child_count(col);
	    colnum = of_get_property(col, "num", NULL);

	    if (!colnum) {
		printk("No colnum! Fail!\n");
	    }

	    printk("Dispenser found %d slots in column %d\n", slots, colnum ? be32_to_cpu(*colnum) : -1);
	    while((slot = of_get_next_child(col, slot))) {
		const int *slotnum;
		struct gpio_desc *gpio;
//                unsigned long lookupflags = GPIO_LOOKUP_FLAGS_DEFAULT;

		printk("Processing slot 0x%p in col 0x%p\n", slot, col);

		if (!slot) {
		    printk("Error, NULL slot child pointer\n");
		    break;
		}

		if (++i > 100) {
		    printk("Iteration limit reached! i == %i, col = 0x%p, slot == 0x%p\n", i, col, slot);
		    break;
		}

		slotnum = of_get_property(slot, "down-gpio", NULL);
		if (!slotnum) {
		    printk("No down-gpio! Fail!\n");
		}
		slotnum = of_get_property(slot, "num", NULL);//big_endian!
		if (!slotnum) {
		    printk("No slotnum! Fail!\n");
		}
		gpio = gpiod_get_from_of_node(slot, "up", 0, GPIOD_IN, "up-gpio");
		if (IS_ERR(gpio)) {
		    printk("GPIO allocation failed! gpio == %li.\n", (long)gpio);
		    printk("Dispenser found %d slot in column %d\n", slotnum ? be32_to_cpu(*slotnum) : -1, colnum ? be32_to_cpu(*colnum) : -1);
		} else {
		    printk("Dispenser found %d slot in column %d, up = %p, hw_num = %d\n", slotnum ? be32_to_cpu(*slotnum) : -1, colnum ? be32_to_cpu(*colnum) : -1, gpio, desc_to_gpio(gpio));
		    gpiod_put(gpio);
		}
		gpio = gpiod_get_index(dev, "up", k, GPIOD_IN);
		if (IS_ERR(gpio)) {
		    printk("Error gettin gpio %li, k = %i", (long)gpio, k);
		} else {
		    printk("Got gpio at index k = %i, 0x%p, hw = %i", k, gpio, desc_to_gpio(gpio));
		    gpiod_put(gpio);
		}
		gpio = gpiod_get_index(NULL, "up", ++k, GPIOD_IN);
		if (IS_ERR(gpio)) {
		    printk("Error gettin gpio %li, k = %i", (long)gpio, k);
		} else {
		    printk("Got gpio at index k = %i, 0x%p, hw = %i", k, gpio, desc_to_gpio(gpio));
		    gpiod_put(gpio);
		}

		gpio = of_find_gpio(dev, "up", ++k, &lookupflags);
		if (IS_ERR(gpio)) {
		    printk("Error gettin gpio %li, k = %i", (long)gpio, k);
		} else {
		    printk("Got gpio at index k = %i, 0x%p, hw = %i", k, gpio, desc_to_gpio(gpio));
		    gpiod_put(gpio);
		}

	    }

	    if (slot) {
		printk("slot not NULL!\n");
		of_node_put(slot);
		slot = NULL;
	    }
	}
	if (col) {
	    printk("col not NULL!\n");
	    of_node_put(col);
	    col = NULL;
	}

	of_node_put(unit);
	unit = NULL;
    } else {
	printk("Dispenser unit not found!\n");
    }
    return;
}
*/

/*
    struct device_node *unit = of_find_node_by_name(NULL, DEVICE_UNIT);
    struct device_node *unit2 = of_find_node_by_path(DEVICE_PATH);
    struct device_node *unit3 = dev->of_node;
    struct device *test = device_find_child_by_name(dev, DEVICE_UNIT);

    struct device_node *col = of_get_next_child(unit3, NULL);
    struct device_node *unit4 = of_get_child_by_name(unit3, DEVICE_UNIT);

    struct device_node *slot = of_get_next_child(unit4, NULL);
    struct platform_device *slot_dev = of_find_device_by_node(slot);
    of_get_child_count()

    if (!slot_dev) {
	slot_dev = of_platform_device_create(slot, "", dev);
    }

    printk("Found unit device_node 0x%p and unit device 0x%p, by path 0x%p, by struct 0x%p\n", unit, test, unit2, unit3);
    printk("Name unit3 %s, col %s, unit4 0x%p = %s\n", unit3->name, col->name, unit4, unit4->name);

    printk("Slot 0x%p %s, device 0x%p", slot, slot->name, slot_dev);

    if (slot_dev && !device_property_present(&slot_dev->dev, "up-gpio")) {
	printk("Dispenser - probe error! up-gpio not found\n");
    } else {
	printk("GPIO property up found");
    }

    if (slot_dev) {
	of_platform_device_destroy(&slot_dev->dev, NULL);
	platform_device_put(slot_dev);
    }

    if (slot)
	of_node_put(slot);
    if (unit4)
	of_node_put(unit4);
    if (col)
	of_node_put(col);
    if (test)
	of_platform_device_destroy(test, NULL);
    if (unit3)
	of_node_put(unit3);
    if (unit2)
	of_node_put(unit2);

    return;
*/
