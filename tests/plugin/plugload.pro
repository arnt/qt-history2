TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qplugin.h \
		  qactionfactory.h \
		  qwidgetfactory.h \
		  qdefaultplugin.h \
		  plugmainwindow.h
		  
SOURCES		= main.cpp \
		  qplugin.cpp \
		  qactionfactory.cpp \
		  qwidgetfactory.cpp \
		  qdefaultplugin.cpp \
		  plugmainwindow.cpp

unix:LIBS	+= -ldl
