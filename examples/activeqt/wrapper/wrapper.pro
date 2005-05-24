TEMPLATE = lib
TARGET	 = wrapperax

CONFIG	+= qt warn_off qaxserver dll

SOURCES	 = main.cpp
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
DEF_FILE = ../../../extensions/activeqt/control/qaxserver.def

# install
target.path = $$[QT_INSTALL_EXAMPLES]/activeqt/wrapper
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS wrapper.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/activeqt/wrapper
INSTALLS += target sources
