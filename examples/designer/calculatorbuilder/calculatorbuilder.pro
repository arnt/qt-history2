SDK = $(QTDIR)/tools/designer/src/lib

# Designer SDK
INCLUDEPATH += \
    $$SDK/sdk \
    $$SDK/uilib

contains(CONFIG, "debug") {
    LIBS += -lQtDesigner_debug
} else {
    LIBS += -lQtDesigner
}
   
HEADERS     = calculatorform.h
QT          += xml
SOURCES     = calculatorform.cpp \
              main.cpp
RESOURCES   = calculatorbuilder.qrc
