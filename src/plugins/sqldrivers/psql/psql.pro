TARGET	 = qsqlpsql

HEADERS		= ../../../sql/drivers/psql/qsql_psql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/psql/qsql_psql.cpp
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

include(../common.pri)
