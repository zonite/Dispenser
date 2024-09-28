TEMPLATE = app


#QT       += core gui
#QT       += qml quick
QT       += core quickcontrols2 websockets

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 qmltypes

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_NAME = eu.nykyri.dispenser
QML_IMPORT_PATH = $$PWD/imports
QML_IMPORT_MAJOR_VERSION = 1

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RESOURCES += qml.qrc
#     controls_conf.qrc

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    manager.cpp \
    qdaemonlog.cpp \
    unitlist.cpp \
    unitmodel.cpp \
    websocketclient.cpp
#    mainwindow.cpp

INCLUDEPATH += \
    ../include \
    ../lib

LIBS += -L".$$PRO_FILE_PWD/../lib/" -lDispenser

#HEADERS += \
#    mainwindow.h

#FORMS += \
#    mainwindow.ui

TRANSLATIONS += \
    gui_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

HEADERS += \
        manager.h \
	unitlist.h \
	unitmodel.h \
	websocketclient.h
