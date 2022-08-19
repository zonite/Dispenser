/*
   Dispenser

 */
#ifndef DISPENSER_PRIVATE_H
#define DISPENSER_PRIVATE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <linux/gpio/consumer.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include <dispenser.h>

#include "dt-bindings/dispenser.h"

#define DRIVER_AUTHOR "Matti Nykyri <matti@nykyri.eu>"
#define DRIVER_DESC "Pet food dispenser"
#define SUCCESS 0
#define FAIL -1
#define DEVICE_NAME "dispenser"
#define DEVICE_CLASS "agriculture"
#define BUF_LEN 100
#define DEBOUNCE 300
#define DEVICE_UNIT "dispense_unit"
#define DEVICE_PATH "/dispenser/dispense_unit"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
//MODULE_SUPPORTED_DEVICE("dispenser");

/* Global Private structures */

struct dispenser_private {
    struct device *dev;
    /*
    int charge;
    int button;
    int door;
    int led;
    */
    unsigned int iFailTimeout;
    struct platform_driver dispenser_driver;
    struct gpio_switch *p_sLed;
    struct gpio_switch *p_sButton;
    struct gpio_switch *p_sDoor;
    struct gpio_switch *p_sCharge;
};

/*
struct io {
    bool init;
    char io_num;
    char value;
    int (*callback)(int irq, void * ident);
};
*/

/* Global static variables */

static struct dispenser_mmap *pDispenser_mmap;
static struct dispenser_private cDispenser;

/* Global Functions */
static int init_chardev(void);
static void cleanup_chardev(void);
static void init_param(void);

/* Platform */
static int dt_probe(struct platform_device *pdev);
static int dt_probe_dispenser(struct platform_device *pdev);
static int dt_probe_column(struct platform_device *pdev);
static int dt_probe_slot(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);
//static struct platform_driver dispenser_driver;

/* GPIO */
struct gpio_switch {
    struct gpio_desc *gpio;
    char *value;
    unsigned int timeout; //millisec
    struct timer_list timer;
    int irq_num;
    irq_handler_t irq_handler;
};

static struct gpio_switch* gpio_device_open(struct device *dev, const char *name, enum gpiod_flags flags, irq_handler_t irq_handler, char *value);
static void gpio_device_close(struct gpio_switch *pgpio);
static char gpio_device_get(struct gpio_switch *pgpio);
static void gpio_device_set(struct gpio_switch *pgpio, char value);
static void gpio_device_set_tmout(struct gpio_switch *pgpio, char value, unsigned int tmout);
static void gpio_timer_callback(struct timer_list *timer);

/* Interupt */
static irqreturn_t door_irq_handler(int irq, void *dev_id);
static irqreturn_t button_irq_handler(int irq, void *dev_id);
static irqreturn_t charge_irq_handler(int irq, void *dev_id);
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id);
//static irq_handler_t door_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/* Events */
enum eventtype {
    CHARGE,
    BUTTON,
    DOOR,
    LED,
    DISPENSER
};

int post_event(enum eventtype type, const char *name, void *data);

/* Unit */
void init_unit(struct device *dev);


/*
static void gpio_init(struct io *io);
static void gpio_clear(struct io *io);
static int gpio_setup(struct io *io, char io_port, char direction, char value, irq_handler_t handler, unsigned long flags);
static int gpio_set(struct io *io, char val);
*/




#endif
