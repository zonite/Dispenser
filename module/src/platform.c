#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
#include <linux/property.h>

#include "dispenser.h"

/*
static struct of_device_id dispenser_driver_ids[] = {
    {
        .compatible = "hortensis,dispenser",
    }, { ** NULL termination ** }
};
*/

MODULE_DEVICE_TABLE(of, dispenser_driver_ids);
//MODULE_DEVICE_TABLE(of, ((struct of_device_id[])cDispenser.dispenser_driver.driver.of_match_table));

static struct of_device_id match = {
    .compatible = COMPAT_COL,
    .name = "col0",
};

/*
static cDispenser.dispenser_driver = {
    .probe = dt_probe,
    .remove = dt_remove,
    .driver = {
        .name = "dispenser",
        .of_match_table = dispenser_driver_ids,
    },
};
*/
/*
static struct platform_driver dispenser_driver = {
    .probe = dt_probe,
    .remove = dt_remove,
    .driver = {
        .name = "dispenser",
        .of_match_table = dispenser_driver_ids,
    },
};
*/

/*
static int dt_register(void)
{
    //cDispenser.dispenser_driver = dispenser_driver;
    cDispenser.dispenser_driver = (struct platform_driver){
        .probe = dt_probe,
        .remove = dt_remove,
        .driver = {
            .name = "dispenser",
            .of_match_table = dispenser_driver_ids,
        },
    };

    return platform_driver_register(&cDispenser.dispenser_driver);
}
*/

static int dt_probe(struct platform_device *pdev)
{
    int ret;
    const char *compatible = NULL;
    struct device *dev = &pdev->dev;

    if (!device_property_present(dev, "compatible")) {
        printk("Dispenser - probe error! Compatible not found\n");
        return FAIL;
    }

    ret = device_property_read_string(dev, "compatible", &compatible);
    if (ret) {
        printk("Dispenser: Could not read 'compatible'\n");
        return FAIL;
    }

    if (!strcasecmp(compatible, COMPAT)) {
        printk("Process %s\n", compatible);
        return dt_probe_dispenser(pdev);
    } else if (!strcasecmp(compatible, COMPAT_COL)) {
        printk("Process %s\n", compatible);
        return dt_probe_column(pdev);
    } else if (!strcasecmp(compatible, COMPAT_SLOT)) {
        printk("Process %s\n", compatible);
        return dt_probe_slot(pdev);
    }
    return FAIL;
}

static int dt_probe_dispenser(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label;
    int ret;
    unsigned int value;

    printk("Dispenser probing device tree.\n");

    if (!device_property_present(dev, "label")) {
        printk("Dispenser - probe error! Label not found\n");
        return FAIL;
    }

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
    if (!device_property_present(dev, "unit")) {
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

    cDispenser.p_sLed = gpio_device_open(dev, "led", GPIOD_OUT_LOW, (irq_handler_t )NULL, &pDispenser_mmap->light, NULL);
    if (!cDispenser.p_sLed) {
        printk("Dispenser: Led failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    cDispenser.p_sButton = gpio_device_open(dev, "button", GPIOD_IN, button_irq_handler, &pDispenser_mmap->button, NULL);
    if (!cDispenser.p_sButton) {
        printk("Dispenser: Button failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    cDispenser.p_sCharge = gpio_device_open(dev, "charge", GPIOD_IN, charge_irq_handler, &pDispenser_mmap->charging, NULL);
    if (!cDispenser.p_sCharge) {
        printk("Dispenser: Charge failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    cDispenser.p_sDoor = gpio_device_open(dev, "door", GPIOD_IN, door_irq_handler, &pDispenser_mmap->door, gpio_timer_door);
    if (!cDispenser.p_sDoor) {
        printk("Dispenser: Door failed\n");
        dt_remove(pdev);
        return FAIL;
    }

    /* Init local unit */
    init_unit(dev);

    //Init TMOUT:
    cDispenser.p_sDoor->timeout = DOOR_TIMEOUT;
    cDispenser.p_sLed->timeout = LIGHT_TIMEOUT;

    //device_fin

    if (of_platform_populate(dev->of_node, &match, NULL, dev)) {
        printk("Device tree population failed! +n");
    }

    printk("Dispenser: Loaded device tree for: %s\n", label);

    return 0;
}

static int dt_probe_column(struct platform_device *pdev)
{
    return FAIL;
}

static int dt_probe_slot(struct platform_device *pdev)
{
    return FAIL;
}

static int dt_remove(struct platform_device *pdev)
{
    int ret;
    const char *compatible = NULL;
    struct device *dev = &pdev->dev;

    printk("Dispenser: Removing device tree\n");

    ret = device_property_read_string(dev, "compatible", &compatible);
    if (ret) {
        printk("Dispenser: Could not read 'compatible'\n");
        return FAIL;
    }

    if (!strcasecmp(compatible, COMPAT)) {
        printk("Process %s\n", compatible);
        if (cDispenser.p_sLed)
            gpio_device_close(cDispenser.p_sLed);
        if (cDispenser.p_sDoor)
            gpio_device_close(cDispenser.p_sDoor);
        if (cDispenser.p_sCharge)
            gpio_device_close(cDispenser.p_sCharge);
        if (cDispenser.p_sButton)
            gpio_device_close(cDispenser.p_sButton);
        cDispenser.p_sLed = NULL;
        cDispenser.p_sDoor = NULL;
        cDispenser.p_sCharge = NULL;
        cDispenser.p_sButton = NULL;

        of_platform_depopulate(dev);
    }

    return 0;
}
