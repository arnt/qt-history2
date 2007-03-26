
QT = core gui script
win32: CONFIG += console
mac:CONFIG -= app_bundle

SOURCES += main.cpp

contains(QT_CONFIG, qdbus) {
    SOURCES += qdbusbinding.cpp
    HEADERS += qdbusbinding.h
    DEFINES += WITH_DBUS
    CONFIG += qdbus
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/script/qscript
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qscript.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/script/qscript
INSTALLS += target sources
