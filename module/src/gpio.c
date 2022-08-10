/* GPIO descriptor interface */

//#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#include "dispenser.h"

static struct gpio_desc *button = NULL;


/*
static void gpio_init(struct io *io)
{
    if (!io->init) {
        io->io_num = -1;
        io->init = true;
    }
}

static void gpio_clear(struct io *io)
{
    gpio_init(io);

    if (io->io_num >= 0) {
        gpio_free(io->io_num);
        io->io_num = -1;
    }
}

static int gpio_setup(struct io *io, char io_port, char direction, char value, irq_handler_t handler, unsigned long flags)
{
    int err;

    gpio_clear(io);

    /*
    if ((err = gpio_request(io_port, THIS_MODULE->name)) == 0) {
        io->io_num = io_port;
        //success
    } else {
        gpio_free(io_port);
        return err;
    }
    */
//}

/*
static int gpio_set(struct io *io, char val)
{

}
*/
