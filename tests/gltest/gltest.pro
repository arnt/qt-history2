REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on debug
HEADERS		= glalpha.h \ 
		  glstencil.h \
		  gldouble.h \
		  gldepth.h \
		  glaccum.h \
		  glcontrolwidget.h \
		  glinfo.h
SOURCES		= glalpha.cpp \
		  glstencil.cpp \
		  gldouble.cpp \
		  gldepth.cpp \
		  glaccum.cpp \
		  glcontrolwidget.cpp \
		  main.cpp 
unix:SOURCES	+= glinfo_x11.cpp
win:SOURCES	+= glinfo_win.h
TARGET		= gltest
DEPENDPATH	= ../include
INTERFACES	= gltest.ui
unix:DEFINES		+= QT_NO_XINERAMA QT_NO_XINPUT QT_NO_XRENDER
