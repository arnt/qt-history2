TARGET	 = qsqlibase

HEADERS		= ../../../sql/drivers/ibase/qsql_ibase.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/ibase/qsql_ibase.cpp

unix {
	OBJECTS_DIR = .obj

	!contains( LIBS, .*gds.* ):!contains( LIBS, .*libfb.* ) {
	    LIBS    *= -lgds
	}
}
win32 {
	OBJECTS_DIR = obj
	!win32-borland:LIBS *= gds32_ms.lib
	win32-borland:LIBS  += gds32.lib
}

include(../common.pri)
