TEMPLATE = lib
INCLUDEPATH += .
DESTDIR = ../../lib
DEFINES += QT_SDK_LIBRARY
CONFIG += qt
CONFIG += dll

INCLUDEPATH += \
    ../extension 

LIBS += \
    -L../../lib -lextension




