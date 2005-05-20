CONFIG      += designer

HEADERS     = calculatorform.h
SOURCES     = calculatorform.cpp \
              main.cpp
RESOURCES   = calculatorbuilder.qrc

# install
target.path = $$[QT_INSTALL_DATA]/examples/designer/calculatorbuilder
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.ui *.pro
sources.path = $$[QT_INSTALL_DATA]/examples/designer/calculatorbuilder
INSTALLS += target sources
