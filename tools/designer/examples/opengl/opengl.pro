GUID 		= {b4a7e213-f42b-4372-aa35-363aacf7cb49}
TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= glwidget.h
SOURCES		= main.cpp \
		  glwidget.cpp
DESTDIR		= ../../../../plugins/designer
INCLUDEPATH     += ../../interfaces

TARGET		= glwidget
target.path += $$plugins.path/designer
INSTALLS 	+= target
