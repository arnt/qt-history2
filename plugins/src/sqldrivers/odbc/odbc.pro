TEMPLATE	= lib
CONFIG+= qt plugin

HEADERS		= ../../../../src/sql/drivers/odbc/qsql_odbc.h 

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/odbc/qsql_odbc.cpp

unix {
	OBJECTS_DIR	= .obj
	LIBS 	*= -lodbc
}

win32 {
	OBJECTS_DIR		= obj
	!win32-borland:LIBS	*= odbc32.lib
	win32-borland:LIBS	*= $(BCB)/lib/PSDK/odbc32.lib
}

REQUIRES	= sql

TARGET		= qsqlodbc
DESTDIR		= ../../../sqldrivers

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/sqldrivers
INSTALLS += target
