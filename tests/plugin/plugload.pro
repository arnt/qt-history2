TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qplugin.h \
		  qpluginmanager.h \
		  qplugininterface.h \
		  qactionfactory.h \
		  qactionplugin.h \
		  qactioninterface.h \
		  qwidgetfactory.h \
		  qwidgetplugin.h \
		  qwidgetinterface.h \
		  plugmainwindow.h \
		  qcleanuphandler.h
		  
SOURCES		= main.cpp \
		  qplugin.cpp \
		  qactionfactory.cpp \
		  qactionplugin.cpp \
		  qwidgetfactory.cpp \
		  qwidgetplugin.cpp \
		  plugmainwindow.cpp

unix:LIBS	+= -ldl
