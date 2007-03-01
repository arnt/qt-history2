TEMPLATE	= lib
CONFIG	       += plugin
HEADERS		= simplestyle.h \
		  simplestyleplugin.h
SOURCES		= simplestyle.cpp \
		  simplestyleplugin.cpp
TARGET		= simplestyleplugin
DESTDIR         = ../stylewindow/styles/

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/styleplugin/stylewindow/styles
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugin.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/tools/styleplugin/plugin
INSTALLS += target sources
