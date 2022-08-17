#include <linux/of_device.h>

#include "dispenser.h"

void init_unit(struct device *dev) {
    struct device_node *unit = of_find_node_by_name(NULL, DEVICE_UNIT);
    struct device *test = device_find_child_by_name(dev, DEVICE_UNIT);

    printk("Found unit device_node 0x%p and unit device 0x%p\n", unit, test);

    return;
}
