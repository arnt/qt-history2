
QT = core script
SOURCES += main.cpp

contains(QT_CONFIG, qdbus) {
    SOURCES += qdbusbinding.cpp
    HEADERS += qdbusbinding.h
    DEFINES += WITH_DBUS
    CONFIG += qdbus
}

