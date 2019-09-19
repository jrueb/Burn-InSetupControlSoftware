#-------------------------------------------------
#
# Project created by QtCreator 2018-07-25T11:10:00
#
#-------------------------------------------------

QT       += core gui widgets
QT       += network

TARGET = burnin
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
QMAKE_CXXFLAGS += -Werror=switch

OBJECTS_DIR = obj
MOC_DIR = obj
UI_HEADERS_DIR = obj
UI_SOURCES_DIR = obj
UI_DIR = obj

#unix:
LIBS += -L/usr/local/lib/ -llxi

SOURCES += \
    general/hwdescriptionparser.cpp \
    devices/ComHandler.cpp \
    devices/environment/JulaboFP50.cpp \
    devices/environment/thermorasp.cpp \
    devices/genericinstrumentclass.cpp \
    general/main.cpp \
    general/systemcontrollerclass.cpp \
    gui/mainwindow.cpp \
    devices/power/controlkeithleypower.cpp \
    devices/power/controlttipower.cpp \
    devices/power/powercontrolclass.cpp \
    devices/daq/daqmodule.cpp \
    gui/daqpage.cpp \
    gui/commandlistpage.cpp \
    general/commandprocessor.cpp \
    gui/commandmodifydialog.cpp \
    general/burnincommand.cpp \
    gui/commandsrundialog.cpp \
    gui/commanddisplayer.cpp \
    devices/environment/chiller.cpp \
    devices/environment/HuberPetiteFleur.cpp \
    general/logger.cpp \
    devices/power/kepco.cpp \
    devices/communication/communicator.cpp \
    devices/communication/lxicommunicator.cpp




FORMS += \
    gui/mainwindow.ui \
    gui/commandmodifydialog.ui \
    gui/commandsrundialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/
else: unix:!android: target.path = /opt/$${TARGET}/
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    settings/hardware_description_desy.xml

HEADERS += \
    general/hwdescriptionparser.h \
    devices/environment/thermorasp.h \
    devices/genericinstrumentclass.h \
    general/systemcontrollerclass.h \
    gui/mainwindow.h \
    devices/power/controlkeithleypower.h \
    devices/power/controlttipower.h \
    devices/power/powercontrolclass.h \
    devices/ComHandler.h \
    devices/environment/JulaboFP50.h \
    devices/daq/daqmodule.h \
    gui/daqpage.h \
    gui/commandlistpage.h \
    general/commandprocessor.h \
    gui/commandmodifydialog.h \
    general/burnincommand.h \
    gui/commandsrundialog.h \
    gui/commanddisplayer.h \
    devices/environment/chiller.h \
    devices/environment/HuberPetiteFleur.h \
    general/logger.h \
    devices/power/kepco.h \
    devices/communication/communicator.h \
    devices/communication/lxicommunicator.h
