QT += script
CONFIG += uitools
RESOURCES += calculator.qrc
SOURCES += main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/script/calculator
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS calculator.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/script/calculator
INSTALLS += target sources
