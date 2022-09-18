#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
//#include <linux/gpio/machine.h>
#include <drivers/gpio/gpiolib-of.h>

#include "dispenser.h"

//#define GPIO_LOOKUP_FLAGS_DEFAULT  ((0 << 0)| (0 << 3))

static int dispenser_unit_init(struct device *dev) {
    int i = 0, n = 0;
    //, k = -1;
    //unsigned char col = 0;
    //struct device_node *unit = of_get_child_by_name(dev->of_node, DEVICE_UNIT);
    struct dispenser_slot_list *slot_list = NULL;
    struct dispenser_col_list *col_list = NULL, *col_iterator;
    unsigned char *slots = NULL, *cols = NULL;
    //struct gpio_descs *up = NULL, *down = NULL, *release = NULL;

    i = device_property_count_u8(dev, "cols");
    if (device_property_count_u8(dev, "slots") != i) {
        printk("Dispenser: Incorrect count slots\n");
        return FAIL;
    }

    if (device_property_count_u32(dev, "up-gpio") != i) {
        printk("Dispenser: Incorrect count up-gpio\n");
        return FAIL;
    }

    if (device_property_count_u32(dev, "down-gpio") != i) {
        printk("Dispenser: Incorrect count down-gpio\n");
        return FAIL;
    }

    if (device_property_count_u32(dev, "release-gpio") != i) {
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
    col_list->first = slot_list;
    col_iterator = col_list;
    cDispenser.col_count = 1;
    cDispenser.cols = col_list;

    for (n = 0; n < i; ++n ) {
        //slots[n].up = gpiod_get_index(dev, "up", n, GPIOD_IN);
        if (col_iterator->col_name != cols[n]) {
            //Different column:
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

                    col_iterator->next->col_name = cols[n];
                    col_iterator->next->col_id = ++(col_iterator->col_id);
                    col_iterator->next->first = &slot_list[n];
                    ++cDispenser.col_count;
                }
                col_iterator = col_iterator->next;
            } while (col_iterator->col_name != cols[n]);
        } else {
            //Same column:
            if (n)
                slot_list[n].slot_id = ++(slot_list[n - 1].slot_id);
        }

        ++col_iterator->slot_count;

        //slot_list[n].col = cols[n];
        slot_list[n].slot_name = slots[n];
        slot_list[n].column = col_iterator;
        slot_list[n].slot_num = n;

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

        if (cols[n] == cols[(n + 1) % n]) {
            slot_list[n].next = &slot_list[n + 1];
            slot_list[n + 1].prev = &slot_list[n];
        }
        //slot_list[n].
    }

    slot_list[0].prev = NULL;
    slot_list[i - 1].next = NULL;

    cDispenser.slot_count = i;

    kfree(slots);
    kfree(cols);

    return SUCCESS;
}

static void dispenser_unit_mmap_set(void)
{
    struct dispenser_col_list *c = cDispenser.cols;

    pDispenser_mmap->unit.cols = cDispenser.col_count;
    pDispenser_mmap->unit.slots = cDispenser.slot_count;

    if (cDispenser.p_sLed)
        cDispenser.p_sLed->value = &pDispenser_mmap->unit.light;
    if (cDispenser.p_sDoor)
        cDispenser.p_sDoor->value = &pDispenser_mmap->unit.door;
    if (cDispenser.p_sCharge)
        cDispenser.p_sCharge->value = &pDispenser_mmap->unit.charging;
    if (cDispenser.p_sButton)
        cDispenser.p_sButton->value = &pDispenser_mmap->unit.button;

    while (c) {
        struct dispenser_slot_list *s = c->first;
        struct dispenser_mmap_column *col_mmap = &pDispenser_mmap[MMAP_COL(c->col_id)].column;

        col_mmap->col_id = c->col_id;
        col_mmap->slot_count = c->slot_count;

        while (s) {
            s->state = &pDispenser_mmap[MMAP_SLOT(s->slot_num)].slot;
            s->up->value = &s->state->up;
            s->down->value = &s->state->down;
            s->release->value = &s->state->release;

            s = s->next;
        }

        c = c->next;
    }
}

static void dispenser_unit_mmap_reset(void)
{
    static struct dispenser_mmap_slot state = { 0 };
    static char n = 0;
    struct dispenser_col_list *c = cDispenser.cols;

    if (cDispenser.p_sLed)
        cDispenser.p_sLed->value = &n;
    if (cDispenser.p_sDoor)
        cDispenser.p_sDoor->value = &n;
    if (cDispenser.p_sCharge)
        cDispenser.p_sCharge->value = &n;
    if (cDispenser.p_sButton)
        cDispenser.p_sButton->value = &n;

    while (c) {
        struct dispenser_slot_list *s = c->first;

        while (s) {
            s->state = &state;
            s->up->value = &s->state->up;
            s->down->value = &s->state->down;
            s->release->value = &s->state->release;

            s = s->next;
        }

        c = c->next;
    }
}

static void dispenser_unit_close()
{
    struct dispenser_col_list *c = cDispenser.cols, *t;
    struct dispenser_slot_list *slots = c->first;

    while (c) {
        struct dispenser_slot_list *s = c->first;

        while (s) {
            dispenser_gpiod_close(s->up);
            dispenser_gpiod_close(s->down);
            dispenser_gpiod_close(s->release);

            s = s->next;
        }

        t = c->next;
        kfree(c);
        c = t;
    }

    kfree(slots);
}

static void dispenser_unit_release_slot(struct dispenser_slot_list *slot)
{
    //dispenser_gpiod_set(slot->release, 1);
    dispenser_release_event(slot->release, 1);
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
