# Project ID used by some IDEs
GUID	 = {9a9583c7-de87-4ad8-9746-0d553ef8462d}
TEMPLATE = lib
TARGET	 = qsqltds

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/tds/qsql_tds.h \
		  ../../../../src/sql/drivers/shared/qsql_result.h

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/tds/qsql_tds.cpp \
		  ../../../../src/sql/drivers/shared/qsql_result.cpp

unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*sybdb.* ) {
	    LIBS 	*= -lsybdb
	}
}

win32 {
	OBJECTS_DIR		= obj
	!win32-borland:LIBS 	*= NTWDBLIB.LIB
	win32-borland:LIBS 	*= $(BCB)/lib/PSDK/NTWDBLIB.LIB
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:ntwdblib.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dntwdblib.dll
#	}
}

QTDIR_build:REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
