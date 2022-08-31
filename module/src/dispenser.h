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
#include <drivers/gpio/gpiolib.h>
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
//#define DEBOUNCE 300
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
    struct dispenser_gpiod *p_sLed;
    struct dispenser_gpiod *p_sButton;
    struct dispenser_gpiod *p_sDoor;
    struct dispenser_gpiod *p_sCharge;
};

struct dispenser_col_list {
    int col;
    struct dispenser_col_list *prev;
    struct dispenser_col_list *next;
};

struct dispenser_slot_list {
    int col;
    int slot;
    struct dispenser_slot_state *state;
    struct dispenser_gpiod *up;
    struct dispenser_gpiod *down;
    struct dispenser_gpiod *release;
    struct dispenser_slot_list *prev;
    struct dispenser_slot_list *next;
    struct dispenser_col_list *column;
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
struct dispenser_gpiod {
    struct gpio_desc *gpiod;
    volatile char *value; //Pointer to the cached value of the descriptor
    char value_priv; //Default place for the value
    volatile unsigned long last; //jiffies of the last interrupt
    unsigned int timeout; //millisec
    struct timer_list timer;
//    void (*timer_callback)(struct timer_list *timer);
    int irq_num;
//    irq_handler_t irq_handler;
    void (*event_handler)(char value);
};

static inline void dispenser_gpiod_set_value_ptr(struct dispenser_gpiod *pgpiod, char *value)
{ pgpiod->value = value; }

//static struct dispenser_gpiod* dispenser_gpiod_open(struct device *dev, const char *name, enum gpiod_flags flags, irq_handler_t irq_handler, char *value, void (*timer_callback)(struct timer_list *));
static struct dispenser_gpiod* dispenser_gpiod_open_index(struct device *dev, const char *name, unsigned int i, enum gpiod_flags flags);
static void dispenser_gpiod_close(struct dispenser_gpiod *pgpiod);
//static char dispenser_gpiod_get(struct dispenser_gpiod *pgpiod);
//static char dispenser_gpiod_get_debounce(struct dispenser_gpiod *pgpiod);
static void dispenser_gpiod_set(struct dispenser_gpiod *pgpiod, char value);
static void dispenser_gpiod_reset_timer(struct dispenser_gpiod *pgpiod, unsigned int tmout);
static void dispenser_gpiod_set_tmout(struct dispenser_gpiod *pgpiod, char value, unsigned int tmout);
static void dispenser_gpiod_tmr_callback(struct timer_list *timer);
static void dispenser_gpiod_out_tmr_callback(struct timer_list *timer);
//static void dispenser_gpiod_timer_door(struct timer_list *timer);

static inline struct dispenser_gpiod *dispenser_gpiod_open(struct device *dev, const char *name, enum gpiod_flags flags)
{ return dispenser_gpiod_open_index(dev, name, 0, flags); }

/* Interupt */
static irqreturn_t dispenser_gpiod_irq_handler(int irq, void *dev_id);
//static irqreturn_t door_irq_handler(int irq, void *dev_id);
//static irqreturn_t button_irq_handler(int irq, void *dev_id);
//static irqreturn_t charge_irq_handler(int irq, void *dev_id);
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

static inline void dispenser_null_event(char new_val)
{ return; }

static int dispenser_post_event(enum eventtype type, const char *name, volatile void *data);
static void dispenser_door_event(char closed);
static void dispenser_button_event(char pressed);
static void dispenser_charge_event(char charging);
static void dispenser_light_event(char on);
static void dispenser_gpiod_event(struct dispenser_gpiod* dev, char new_val);

/* Unit */
void init_unit(struct device *dev);


/*
static void gpio_init(struct io *io);
static void gpio_clear(struct io *io);
static int gpio_setup(struct io *io, char io_port, char direction, char value, irq_handler_t handler, unsigned long flags);
static int gpio_set(struct io *io, char val);
*/




#endif
