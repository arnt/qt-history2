REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on debug
HEADERS		= glalpha.h \
		  glstencil.h \
		  gldouble.h \
		  gldepth.h \
		  glaccum.h \
		  glcontrolwidget.h
SOURCES		= glalpha.cpp \
		  glstencil.cpp \
		  gldouble.cpp \
		  gldepth.cpp \
		  glaccum.cpp \
		  glcontrolwidget.cpp \
		  main.cpp
TARGET		= gltest
DEPENDPATH	= ../include
INTERFACES	= gltest.ui
