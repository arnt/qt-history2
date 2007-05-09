HEADERS    = stylewindow.h
SOURCES    = stylewindow.cpp \
             main.cpp

TARGET     = ../styleplugin

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/styleplugin/stylewindow
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS stylewindow.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/styleplugin/stylewindow
INSTALLS += target sources
