TEMPLATE	= lib
CONFIG		+= qt plugin

HEADERS		= ../../../../src/sql/drivers/tds/qsql_tds.h \
		  ../../../../src/sql/drivers/shared/qsql_result.h

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/tds/qsql_tds.cpp \
		  ../../../../src/sql/drivers/shared/qsql_result.cpp

unix {
	OBJECTS_DIR	= .obj
	LIBS 	*= -lsybdb
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

REQUIRES	= sql

TARGET		= qsqltds
DESTDIR		= ../../../sqldrivers

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/sqldrivers
INSTALLS += target
