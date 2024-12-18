TARGET = QtSerialBus
MODULE = serialbus
QT = core-private network serialport
CONFIG += c++11

QMAKE_DOCS = $$PWD/doc/qtserialbus.qdocconf

MODULE_PLUGIN_TYPES = \
    canbus

load(qt_module)

PUBLIC_HEADERS += \
    qcanbusdevice.h \
    qcanbusfactory.h \
    qcanbusframe.h \
    qcanbus.h \
    qserialbusglobal.h \
    qmodbusserver.h \
    qmodbusdevice.h \
    qmodbusdataunit.h \
    qmodbusclient.h \
    qmodbusreply.h \
    qmodbusrtuserialmaster.h \
    qmodbustcpclient.h \
    qmodbustcpserver.h \
    qmodbusrtuserialslave.h \
    qmodbuspdu.h

PRIVATE_HEADERS += \
    qcanbusdevice_p.h \
    qmodbusserver_p.h \
    qmodbusclient_p.h \
    qmodbusdevice_p.h \
    qmodbusrtuserialmaster_p.h \
    qmodbustcpclient_p.h \
    qmodbustcpserver_p.h \
    qmodbusrtuserialslave_p.h \
    qmodbus_symbols_p.h \
    qmodbuscommevent_p.h \
    qmodbusadu_p.h

SOURCES += \
    qcanbusdevice.cpp \
    qcanbus.cpp \
    qcanbusfactory.cpp \
    qcanbusframe.cpp \
    qmodbusserver.cpp \
    qmodbusdevice.cpp \
    qmodbusdataunit.cpp \
    qmodbusclient.cpp \
    qmodbusreply.cpp \
    qmodbusrtuserialmaster.cpp \
    qmodbustcpclient.cpp \
    qmodbustcpserver.cpp \
    qmodbusrtuserialslave.cpp \
    qmodbuspdu.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
