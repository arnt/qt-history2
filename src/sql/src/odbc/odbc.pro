TEMPLATE	= lib
CONFIG		= qt warn_on debug plugin

HEADERS		= qsql_odbc.h 

SOURCES		= main.cpp \
		  qsql_odbc.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

unix:LIBS       += -lodbc

win32:LIBS  	+= odbc32.lib

TARGET		= qsqlodbc
DESTDIR		= ../../../../plugins

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
