TEMPLATE = lib
INCLUDEPATH += .
DESTDIR = ../../lib
DEFINES += QT_SDK_LIBRARY
CONFIG += qt
CONFIG += dll

INCLUDEPATH += \
    ../extension \
    ../../shared

LIBS += \
    -L../../lib -lextension




