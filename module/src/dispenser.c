/*
   Dispenser

 */
#include "dispenser.h"

/* Global static variables */
static struct gpio_device *p_sLed = NULL;
static struct gpio_device *p_sDoor = NULL;
static struct gpio_device *p_sButton = NULL;
static struct gpio_device *p_sCharge = NULL;

/* Dev files */
#include "interface.c"
#include "chardev.c"
//#include "gpio.c"
#include "gpio_device.c"
#include "interrupt.c"
#include "param.c"
#include "platform.c"

