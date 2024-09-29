QT -= gui

QT += websockets core

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
	monitor.cpp \
    slotitem.cpp \
        timer.cpp \
    unititem.cpp \
        websocketclient.cpp

HEADERS += \
        alarm.h \
    colitem.h \
    lib_global.h \
#    dispenser.h \
        localinfo.h \
	monitor.h \
    slotitem.h \
        timer.h \
    unititem.h \
        websocketclient.h

CONFIG += link_pkgconfig
CONFIG += debug

PKGCONFIG += Qt5Daemon

LIBS += -L".$$PRO_FILE_PWD/../../SmtpClient-for-Qt/src/" -lSmtpMime

INCLUDEPATH += \
        ../include \
	../../SmtpClient-for-Qt/src/

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
