TEMPLATE = app
TARGET	 = simpleax

CONFIG	+= qt warn_off qaxserver

SOURCES	 = main.cpp
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc

# install
target.path = $$[QT_INSTALL_DATA]/examples/activeqt/simple
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS simple.pro
sources.path = $$[QT_INSTALL_DATA]/examples/activeqt/simple
INSTALLS += target sources
