#include <linux/of_device.h>

#include "dispenser.h"

void init_unit(struct device *dev) {
    struct device_node *unit = of_find_node_by_name(NULL, DEVICE_UNIT);
    struct device_node *unit2 = of_find_node_by_path(DEVICE_PATH);
    struct device_node *unit3 = dev->of_node;
    struct device *test = device_find_child_by_name(dev, DEVICE_UNIT);

    struct device_node *col = of_get_next_child(unit3, NULL);
    struct device_node *unit4 = of_get_child_by_name(unit3, DEVICE_UNIT);

    struct device_node *slot = of_get_next_child(unit4, NULL);
    struct platform_device *slot_dev = of_find_device_by_node(slot);

    printk("Found unit device_node 0x%p and unit device 0x%p, by path 0x%p, by struct 0x%p\n", unit, test, unit2, unit3);
    printk("Name unit3 %s, col %s, unit4 0x%p = %s\n", unit3->name, col->name, unit4, unit4->name);

    printk("Slot 0x%p %s, device 0x%p", slot, slot->name, slot_dev);

    if (slot_dev && !device_property_present(&slot_dev->dev, "up-gpio")) {
        printk("Dispenser - probe error! up-gpio not found\n");
    } else {
        printk("GPIO property up found");

    }

    return;
}
