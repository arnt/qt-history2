TARGET	 = qsqldb2

HEADERS		= ../../../sql/drivers/db2/qsql_db2.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/db2/qsql_db2.cpp

unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*db2.* ) {
	    LIBS 	*= -ldb2
	}
}
win32 {
	!contains( LIBS, .*db2.* ) {
		LIBS *= db2cli.lib
	}
}

include(../common.pri)
