TARGET	 = qsqlmysql

HEADERS		= ../../../sql/drivers/mysql/qsql_mysql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/mysql/qsql_mysql.cpp

unix {
	OBJECTS_DIR = .obj

	!contains( LIBS, .*mysql.* ) {
	    LIBS    *= -lmysqlclient
	}
}
win32 {
	OBJECTS_DIR = obj
	LIBS	*= libmysql.lib
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibmysql.dll
#	}
}

include(../common.pri)
