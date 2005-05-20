TEMPLATE = app
TARGET	 = openglax

CONFIG	+= qt warn_off qaxserver

QT += opengl

HEADERS	 = glbox.h \
	   globjwin.h
SOURCES	 = glbox.cpp \
	   globjwin.cpp \
	   main.cpp
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc

# install
target.path = $$[QT_INSTALL_DATA]/examples/activeqt/opengl
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS opengl.pro
sources.path = $$[QT_INSTALL_DATA]/examples/activeqt/opengl
INSTALLS += target sources
