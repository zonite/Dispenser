#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
#include <linux/property.h>

#include "dispenser.h"

static struct of_device_id dispenser_driver_ids[] = {
    {
        .compatible = "hortensis,dispenser",
}, { /* NULL termination */ }
};

MODULE_DEVICE_TABLE(of, dispenser_driver_ids);

static struct platform_driver dispenser_driver = {
    .probe = dt_probe,
    .remove = dt_remove,
    .driver = {
        .name = "dispenser",
        .of_match_table = dispenser_driver_ids,
    },
};

static int dt_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label;
    int ret;
    unsigned int value;

    printk("Dispenser probing device tree.\n");

    /* Check correct label */
    if (!device_property_present(dev, "label")) {
        printk("Dispenser - probe error! Label not found\n");
        return -1;
    }
    if (!device_property_present(dev, "local")) {
        printk("Dispenser - probe error! Local not found\n");
        return -1;
    }

    // Read properties
    ret = device_property_read_string(dev, "label", &label);
    if (ret) {
        printk("Dispenser: Could not read 'label'\n");
        return -1;
    }
    ret = device_property_read_u32(dev, "value", &value);
    if (ret) {
        printk("Dispenser: Could not read 'value'\n");
        return -1;
    }
    printk("Dispenser: Loaded device tree for: %s\n", label);

    return 0;
}

static int dt_remove(struct platform_device *pdev)
{
    printk("Dispenser: Removing device tree\n");

    return 0;
}
