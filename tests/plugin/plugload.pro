TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qactionfactory.h \
		  qactionplugin.h \
		  qactioninterface.h \
		  plugmainwindow.h

SOURCES		= main.cpp \
		  qactionfactory.cpp \
		  qactionplugin.cpp \
		  ../../tools/designer/shared/widgetplugin.cpp \
		  plugmainwindow.cpp
