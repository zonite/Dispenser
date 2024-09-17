QT -= gui

TEMPLATE = lib
DEFINES += LIB_LIBRARY

CONFIG += c++11 staticlib dynamic
TARGET = Dispenser

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        alarm.cpp \
    colitem.cpp \
#    dispenser.cpp \
        localinfo.cpp \
    slotitem.cpp \
        timer.cpp \
    unititem.cpp

HEADERS += \
        alarm.h \
    colitem.h \
    lib_global.h \
#    dispenser.h \
        localinfo.h \
    slotitem.h \
        timer.h \
    unititem.h

CONFIG += link_pkgconfig
CONFIG += debug

PKGCONFIG += Qt5Daemon

INCLUDEPATH += \
        ../include

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target