TEMPLATE = lib
TARGET	 = multipleax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = main.cpp
HEADERS	 = ax1.h ax2.h
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
DEF_FILE = ../../../extensions/activeqt/control/qaxserver.def

# install
target.path = $$[QT_INSTALL_EXAMPLES]/activeqt/multiple
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS multiple.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/activeqt/multiple
INSTALLS += target sources
