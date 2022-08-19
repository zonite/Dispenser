/* Module parameters */

#include <linux/moduleparam.h>
#include "dispenser.h"

/*
static int p_Charge = -1;
static int p_Button = -1;
static int p_Door = -1;
static int p_LED = -1;
*/

static unsigned int p_FailTimeout = FAIL_TIMEOUT;
static unsigned int p_DoorTimeout = DOOR_TIMEOUT;
static unsigned int p_LightTimeout = LIGHT_TIMEOUT;


/* Module param */
module_param(p_FailTimeout, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_FailTimeout, "Fault timeout");

module_param(p_DoorTimeout, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_DoorTimeout, "Door timeout");

module_param(p_LightTimeout, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_LightTimeout, "Light timeout");
/*
module_param(p_Charge, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_Charge, "Charge detection input");

module_param(p_Button, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_Button, "Button press input");

module_param(p_Door, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_Door, "Door closed input");

module_param(p_LED, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(p_LED, "LED output");
*/

void init_param(void)
{
    printk("Parameter initalization.\n");
    cDispenser.p_sDoor->timeout = p_DoorTimeout;
    cDispenser.p_sLed->timeout = p_LightTimeout;
    cDispenser.iFailTimeout = p_FailTimeout;
    /*
    struct dispenser_config *config = pDispenser;
    
    config->charge = p_Charge;
    config->button = p_Button;
    config->door = p_Door;
    config->led = p_LED;
    */
}
