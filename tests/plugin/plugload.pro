TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qplugin.h \
		  qactionfactory.h \
		  qwidgetfactory.h \
		  qwidgetplugin.h \
		  qwidgetinterface.h \
		  plugmainwindow.h
		  
SOURCES		= main.cpp \
		  qplugin.cpp \
		  qactionfactory.cpp \
		  qwidgetfactory.cpp \
		  qwidgetplugin.cpp \
		  plugmainwindow.cpp

unix:LIBS	+= -ldl
