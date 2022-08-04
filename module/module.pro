CONFIG -= qt

TARGET = dispenser
TEMPLATE = lib

lupdate_only {
    SOURCES += \
        src/chardev.c \
        src/gpio.c \
        src/interface.c \
        src/interrupt.c \
        src/param.c \
        src/dispenser.c

    HEADERS += \
        src/dispenser.h
}

KERNEL_RELEASE = $$system(uname -r)

INCLUDEPATH += \
    /lib/modules/$${KERNEL_RELEASE}/source/include \
    /lib/modules/$${KERNEL_RELEASE}/source/include/linux \
    /lib/modules/$${KERNEL_RELEASE}/source/arch/x86/include \
    /lib/modules/$${KERNEL_RELEASE}/source/arm64/x86/include

DEFINES += \
    __KERNEL__ \
    KBUILD_MODNAME=\"\\\"\\\"\"

OTHER_FILES += \
    src/Makefile

DUMMY_FILES = .
makedriver.input = DUMMY_FILES
makedriver.output = src/$${TARGET}.ko
makedriver.commands = cd src; make; cd ..
makedriver.clean = src/*.ko src/*.o src/*.mod.c src/modules.order src/Module.symvers
makedriver.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += makedriver
PRE_TARGETDEPS += compiler_makedriver_make_all

DISTFILES += \
    src/rpioverlay.dts

HEADERS += \
    src/dt-bindings/dispenser.h
