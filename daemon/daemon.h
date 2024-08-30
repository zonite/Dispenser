#ifndef DAEMON_H
#define DAEMON_H

#define PAGE_SHIFT 13U
#define PAGE_SIZE (1UL << PAGE_SHIFT) //8192UL
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_ALIGN(len) (((len)+PAGE_SIZE-1) & ~(PAGE_SIZE-1))

#endif // DAEMON_H
