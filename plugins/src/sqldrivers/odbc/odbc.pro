TEMPLATE	= lib
CONFIG+= qt plugin

HEADERS		= ../../../../src/sql/drivers/odbc/qsql_odbc.h 

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/odbc/qsql_odbc.cpp

unix {
	OBJECTS_DIR	= .obj
	isEmpty(LIBS) {
		LIBS += -lodbc
	}
}

win32 {
	OBJECTS_DIR		= obj
	isEmpty(LIBS) {
		!win32-borland:LIBS	+= odbc32.lib
		win32-borland:LIBS	+= $(BCB)/lib/PSDK/odbc32.lib
	}
}

REQUIRES	= sql

TARGET		= qsqlodbc
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
