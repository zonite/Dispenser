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
        return FAIL;
    }
    /* Check Button */
    if (!device_property_present(dev, "button-gpio")) {
        printk("Dispenser - probe error! Button not found\n");
        return FAIL;
    }
    /* Check LED */
    if (!device_property_present(dev, "led-gpio")) {
        printk("Dispenser - probe error! Led not found\n");
        return FAIL;
    }
    /*
    if (!device_property_present(dev, "local")) {
        printk("Dispenser - probe error! Local not found\n");
        return FAIL;
    }
    */

    // Read properties
    ret = device_property_read_string(dev, "label", &label);
    if (ret) {
        printk("Dispenser: Could not read 'label'\n");
        return FAIL;
    }
    ret = device_property_read_u32(dev, "value", &value);
    if (ret) {
        printk("Dispenser: Could not read 'value'\n");
        return FAIL;
    }

    p_sLed = gpio_device_open(dev, "led-gpio", GPIOD_OUT_LOW);
    if (!p_sLed) {
        printk("Dispenser: Led failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    p_sButton = gpio_device_open(dev, "button-gpio", GPIOD_IN);
    if (!p_sButton) {
        printk("Dispenser: Button failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    p_sCharge = gpio_device_open(dev, "charge-gpio", GPIOD_IN);
    if (!p_sCharge) {
        printk("Dispenser: Charge failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    p_sDoor = gpio_device_open(dev, "door-gpio", GPIOD_IN);
    if (!p_sDoor) {
        printk("Dispenser: Door failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    printk("Dispenser: Loaded device tree for: %s\n", label);

    return 0;
}

static int dt_remove(struct platform_device *pdev)
{
    printk("Dispenser: Removing device tree\n");

    if (p_sLed)
        gpio_device_close(p_sLed);
    if (p_sDoor)
        gpio_device_close(p_sDoor);
    if (p_sCharge)
        gpio_device_close(p_sCharge);
    if (p_sButton)
        gpio_device_close(p_sButton);
    p_sLed = NULL;
    p_sDoor = NULL;
    p_sCharge = NULL;
    p_sButton = NULL;

    return 0;
}
