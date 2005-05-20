TEMPLATE = lib
TARGET	 = hierarchyax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = objects.cpp main.cpp
HEADERS	 = objects.h
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
DEF_FILE = ../../../extensions/activeqt/control/qaxserver.def

# install
target.path = $$[QT_INSTALL_DATA]/examples/activeqt/hierarchy
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS hierarchy.pro
sources.path = $$[QT_INSTALL_DATA]/examples/activeqt/hierarchy
INSTALLS += target sources
