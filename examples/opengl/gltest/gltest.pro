REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on release
HEADERS		= glalpha.h \
		  glstencil.h \
		  glcontrolwidget.h
SOURCES		= glalpha.cpp \
		  glstencil.cpp \
		  glcontrolwidget.cpp \
		  main.cpp
TARGET		= gltest
DEPENDPATH	= ../include
INTERFACES	= gltest.ui
