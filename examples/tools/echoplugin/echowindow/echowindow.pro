HEADERS    = echowindow.h \
             echointerface.h
SOURCES    = echowindow.cpp \
             main.cpp

TARGET     = ../echoplugin

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin/echowindow
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS echowindow.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/echoplugin/echowindow
INSTALLS += target sources
