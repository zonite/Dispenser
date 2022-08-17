/*
   Dispenser

 */
#include "dispenser.h"

#include <linux/of_device.h>

/* Global static variables */
static struct of_device_id dispenser_driver_ids[] = {
    {
        .compatible = "hortensis,dispenser",
    }, { /* NULL termination */ }
};

//static struct dispenser_private cDispenser = { 0 };
static struct dispenser_private cDispenser = {
    .dispenser_driver = {
        .probe = dt_probe,
        .remove = dt_remove,
        .driver = {
            .name = "dispenser",
            .of_match_table = dispenser_driver_ids,
        },
    0 },
};

static struct dispenser_mmap *pDispenser_mmap = NULL;

/*
static struct dispenser_private cDispenser = {
    .dispenser_driver = {
        .probe = dt_probe,
        .remove = dt_remove,
        .driver = {
            .name = "dispenser",
            .of_match_table = (struct of_device_id[]) {
                {
                    .compatible = "hortensis,dispenser",
                }, { }
            },
        },
    0 },
};
*/

/*
static struct gpio_device *p_sLed = NULL;
static struct gpio_device *p_sDoor = NULL;
static struct gpio_device *p_sButton = NULL;
static struct gpio_device *p_sCharge = NULL;
*/

/* Dev files */
#include "interface.c"
#include "chardev.c"
//#include "gpio.c"
#include "gpio_device.c"
#include "interrupt.c"
#include "param.c"
#include "platform.c"
#include "event.c"

