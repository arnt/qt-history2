TEMPLATE	= lib
CONFIG		= qt warn_on release
WIN32:CONFIG   += dll
HEADERS		= glwidget.h \
		  $(QTDIR)/src/opengl/qgl.h
SOURCES		= main.cpp \
		  glwidget.cpp \
		  $(QTDIR)/src/opengl/qgl.cpp
win32:SOURCES  += $(QTDIR)/src/opengl/qgl_win.cpp
unix:SOURCEDS  += $(QTDIR)/src/opengl/qgl_unix.cpp
INTERFACES	=
DESTDIR		= $(QTDIR)/plugins
