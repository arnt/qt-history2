TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= rc2ui.h
SOURCES		= main.cpp rc2ui.cpp
DESTDIR		= ../../../../plugins/designer
TARGET		= rcplugin
INCLUDEPATH	+= ../../interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/designer
INSTALLS	+= target
