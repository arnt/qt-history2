TEMPLATE	= lib
CONFIG		+= qt plugin
HEADERS		= ../../../../src/sql/drivers/sqlite/qsql_sqlite.h 
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/sqlite/qsql_sqlite.cpp 

unix {
	OBJECTS_DIR = .obj
	
	!contains( LIBS, .*sqlite.* ) {
	    LIBS    *= -lsqlite
	}
}
win32 {
	OBJECTS_DIR = obj
	LIBS	*= sqlite.lib
}

QTDIR_build:REQUIRES	= sql

TARGET		= qsqlite
DESTDIR		= ../../../sqldrivers


target.path += $$plugins.path/sqldrivers
INSTALLS += target
