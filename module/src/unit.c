#include <linux/of_device.h>

#include "dispenser.h"

void init_unit(void) {
    struct device_node *unit = of_find_node_by_name(NULL, "unit");

    printk("Found unit node %p\n", unit);

    return;
}
