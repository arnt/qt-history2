TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qplugin.h \
		  qactionfactory.h \
		  qwidgetfactory.h \
		  qwidgetplugin.h \
		  qwidgetinterface.h \
		  qactionplugin.h \
		  qactioninterface.h \
		  plugmainwindow.h
		  
SOURCES		= main.cpp \
		  qplugin.cpp \
		  qactionfactory.cpp \
		  qwidgetfactory.cpp \
		  qwidgetplugin.cpp \
		  qactionplugin.cpp \
		  plugmainwindow.cpp

unix:LIBS	+= -ldl
