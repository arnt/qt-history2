TARGET	 = qsqltds

HEADERS		= ../../../sql/drivers/tds/qsql_tds.h

SOURCES		= main.cpp \
		  ../../../sql/drivers/tds/qsql_tds.cpp

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

include(../common.pri)
