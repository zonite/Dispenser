QT -= gui
QT += network websockets

CONFIG += c++11 console
CONFIG -= app_bundle

//QMAKE_CXXFLAGS += -g

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        kernelclient.cpp \
	main.cpp \
	websocketserver.cpp

TRANSLATIONS += \
    daemon_en_US.ts
#CONFIG += lrelease
CONFIG += debug
CONFIG += embed_translations
CONFIG += link_pkgconfig

PKGCONFIG += libnl-3.0 Qt5Daemon

INCLUDEPATH += \
    ../include
#    /usr/include/qt5/QtDaemon \
#    /usr/include/aarch64-linux-gnu/qt5/QtDaemon

LIBS += -lQt5Daemon

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
        kernelclient.h \
	websocketserver.h

RESOURCES += \
        daemon.qrc
