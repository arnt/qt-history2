SDK = $(QTDIR)/tools/designer/src/lib

# Designer SDK
INCLUDEPATH += \
    $$SDK/extension \
    $$SDK/sdk \
    $$SDK/uilib

contains(CONFIG, "debug") {
    LIBS += -lQtDesigner_debug
} else {
    LIBS += -lQtDesigner
}

DESTDIR     = $(QTDIR)/plugins/designer   
HEADERS     = worldtimeclock.h \
              plugin.h
QT          += xml
SOURCES     = worldtimeclock.cpp \
              plugin.cpp
TEMPLATE    = lib
