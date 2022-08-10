/*
   Dispenser

 */
#ifndef DISPENSER_H
#define DISPENSER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

#define DRIVER_AUTHOR "Matti Nykyri <matti@nykyri.eu>"
#define DRIVER_DESC "Pet food dispenser"
#define SUCCESS 0
#define FAIL -1
#define DEVICE_NAME "dispenser"
#define BUF_LEN 100

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
//MODULE_SUPPORTED_DEVICE("dispenser");

/* Global static variables */
static struct gpio_device *p_sLed;
static struct gpio_device *p_sButton;
static struct gpio_device *p_sDoor;
static struct gpio_device *p_sCharge;

struct dispenser_config {
    struct device *dev;
    int charge;
    int button;
    int door;
    int led;
};

struct io {
    bool init;
    char io_num;
    char value;
    int (*callback)(int irq, void * ident);
};

static void *pDispenser;
static struct dispenser_config cConfig;

static int init_chardev(void);
static void cleanup_chardev(void);
static void init_param(void);

/* Platform */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);
static struct platform_driver dispenser_driver;

/* GPIO */
struct gpio_device {
    struct gpio_desc *gpio;
    unsigned long timeout;
};

static struct gpio_device* gpio_device_open(struct device *dev, const char *name, enum gpiod_flags flags);
static void gpio_device_close(struct gpio_device *pgpio);
static void gpio_device_set(struct gpio_device *pgpio, char value);

/*
static void gpio_init(struct io *io);
static void gpio_clear(struct io *io);
static int gpio_setup(struct io *io, char io_port, char direction, char value, irq_handler_t handler, unsigned long flags);
static int gpio_set(struct io *io, char val);
*/




#endif
