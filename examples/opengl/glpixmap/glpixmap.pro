TEMPLATE	= app
TARGET		= glpixmap

CONFIG		+= qt warn_on release
QT         += opengl
!mac:unix:LIBS  += -lm
DEPENDPATH	= ../include

QTDIR_build:REQUIRES         = "contains(QT_CONFIG, opengl)"

HEADERS		= glbox.h \
		  globjwin.h
SOURCES		= glbox.cpp \
		  globjwin.cpp \
		  main.cpp
