GUID 	 = {2a81f97c-4003-4399-b97d-904f6f607a27}
TEMPLATE = lib
TARGET	 = qsqlpsql

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/psql/qsql_psql.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/psql/qsql_psql.cpp
unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*pq.* ) {
	    LIBS	*= -lpq
	}
}

win32 {
	OBJECTS_DIR	= obj
	LIBS	*= libpqdll.lib
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libpq.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibpq.dll
#	}
}

QTDIR_build:REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
