TEMPLATE	= lib
CONFIG+= qt warn_on release plugin
HEADERS		= glwidget.h
SOURCES		= main.cpp \
		  glwidget.cpp
DESTDIR		= ../../../../plugins/designer
INCLUDEPATH     += ../../interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/designer
INSTALLS += target
