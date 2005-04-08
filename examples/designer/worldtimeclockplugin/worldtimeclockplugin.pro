SDK = $(QTDIR)/tools/designer/src/lib

# Designer SDK
INCLUDEPATH += \
    $$SDK/extension \
    $$SDK/sdk \
    $$SDK/uilib

CONFIG      += designer
QT          += xml
TEMPLATE    = lib

DESTDIR     = $(QTDIR)/plugins/designer   
HEADERS     = worldtimeclock.h \
              worldtimeclockplugin.h
SOURCES     = worldtimeclock.cpp \
              worldtimeclockplugin.cpp
