QT -= gui
QT += network websockets

CONFIG += c++11 console
CONFIG -= app_bundle

//QMAKE_CXXFLAGS += -g

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        buffer.cpp \
	kernelclient.cpp \
	main.cpp \
        simulator.cpp \
	websocketserver.cpp

TRANSLATIONS += \
    daemon_en_US.ts
#CONFIG += lrelease
CONFIG += debug
CONFIG += embed_translations
CONFIG += link_pkgconfig

#PKGCONFIG += libnl-3.0 Qt5Daemon
PKGCONFIG += Qt5Daemon

INCLUDEPATH += \
    ../include \
    ../lib
#    /usr/include/qt5/QtDaemon \
#    /usr/include/aarch64-linux-gnu/qt5/QtDaemon

LIBS += -L".$$PRO_FILE_PWD/../lib/" -L".$$PRO_FILE_PWD/../../SmtpClient-for-Qt/src/" -lQt5Daemon -lDispenser -lSmtpMime

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
        buffer.h \
	daemon.h \
	kernelclient.h \
        simulator.h \
	websocketserver.h

RESOURCES += \
        daemon.qrc

DISTFILES += \
        http/HLSnginx.conf \
	http/RPInginx.conf \
	http/player.html \
	skripts/camerastream.service \
	skripts/captureCamera.sh \
	skripts/createReport.sh \
        skripts/dispenser.service \
	skripts/reencode.sh \
	skripts/rpistream.sh \
	skripts/sendReport.sh \
	skripts/startRec.sh \
	skripts/stopRec.sh
