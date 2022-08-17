#include <linux/of_device.h>

#include "dispenser.h"

void init_unit(struct device *dev) {
    struct device_node *unit = of_find_node_by_name(NULL, DEVICE_UNIT);
    struct device_node *unit2 = of_find_node_by_path(DEVICE_PATH);
    struct device_node *unit3 = dev->of_node;
    struct device *test = device_find_child_by_name(dev, DEVICE_UNIT);

    printk("Found unit device_node 0x%p and unit device 0x%p, by path 0x%p, by struct 0x%p\n", unit, test, unit2, unit3);

    return;
}
