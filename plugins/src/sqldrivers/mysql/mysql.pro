GUID 	 = {0beec0b5-2c6f-4420-8a98-b7555099c8ea}
TEMPLATE = lib
TARGET	 = qsqlmysql

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/mysql/qsql_mysql.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/mysql/qsql_mysql.cpp

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

QTDIR_build:REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
