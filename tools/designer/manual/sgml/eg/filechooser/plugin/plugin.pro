TEMPLATE	= lib
CONFIG		= qt warn_on release plugin
HEADERS		= plugin.h ../widget/filechooser.h
SOURCES		= plugin.cpp ../widget/filechooser.cpp
INTERFACES	=
DESTDIR		= $(QTDIR)/plugins
INCLUDEPATH     += $(QTDIR)/tools/designer/interfaces
TARGET		= filechooser

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
