REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on release
HEADERS		= glalpha.h \
		  glstencil.h \
		  gldouble.h \
		  gldepth.h \
		  glcontrolwidget.h
SOURCES		= glalpha.cpp \
		  glstencil.cpp \
		  gldouble.cpp \
		  gldepth.cpp \
		  glcontrolwidget.cpp \
		  main.cpp
TARGET		= gltest
DEPENDPATH	= ../include
INTERFACES	= gltest.ui
