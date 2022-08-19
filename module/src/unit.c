#include <linux/of_device.h>
#include <linux/gpio/consumer.h>

#include "dispenser.h"

void init_unit(struct device *dev) {
    int i = 0;

    struct device_node *unit = of_get_child_by_name(dev->of_node, DEVICE_UNIT);
    if (unit) {
        int cols = of_get_child_count(unit);
        struct device_node *col = NULL;

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

            printk("Dispenser found %d slots in column %d\n", slots, colnum ? *colnum : -1);
            while((slot = of_get_next_child(col, slot))) {
                const int *slotnum;
                struct gpio_desc *gpio;

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
                slotnum = of_get_property(slot, "num", NULL);
                if (!slotnum) {
                    printk("No slotnum! Fail!\n");
                }
                gpio = gpiod_get_from_of_node(slot, "up", 0, GPIOD_IN, "up");
                if (IS_ERR(gpio)) {
                    printk("GPIO allocation failed! gpio == %li.\n", (long)gpio);
                    printk("Dispenser found %d slot in column %d\n", slotnum ? *slotnum : -1, colnum ? *colnum : -1);
                } else {
                    printk("Dispenser found %d slot in column %d, up = %p, hw_num = %d\n", slotnum ? *slotnum : -1, colnum ? *colnum : -1, gpio, desc_to_gpio(gpio));
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
