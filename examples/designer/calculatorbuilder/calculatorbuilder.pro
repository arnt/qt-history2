CONFIG      += uitools

HEADERS     = calculatorform.h
RESOURCES   = calculatorbuilder.qrc
SOURCES     = calculatorform.cpp \
              main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/designer/calculatorbuilder
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.ui *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/designer/calculatorbuilder
INSTALLS += target sources
