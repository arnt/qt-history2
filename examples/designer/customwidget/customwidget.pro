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
   
HEADERS     = analogclock.h \
              plugin.h
SOURCES     = analogclock.cpp \
              plugin.cpp
TEMPLATE    = lib
