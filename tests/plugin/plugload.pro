TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qactionfactory.h \
		  qactionplugin.h \
		  qactioninterface.h \
		  qwidgetfactory.h \
		  qwidgetplugin.h \
		  qwidgetinterface.h \
		  plugmainwindow.h

SOURCES		= main.cpp \
		  qactionfactory.cpp \
		  qactionplugin.cpp \
		  qwidgetfactory.cpp \
		  qwidgetplugin.cpp \
		  plugmainwindow.cpp
