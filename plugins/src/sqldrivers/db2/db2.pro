GUID 	 = {2fe4f5d3-70e6-426e-a0cb-857e80c22a32}
TEMPLATE = lib
TARGET	 = qsqldb2

CONFIG	+= qt plugin
DESTDIR	 = ../../../sqldrivers

HEADERS		= ../../../../src/sql/drivers/db2/qsql_db2.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/db2/qsql_db2.cpp

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

QTDIR_build:REQUIRES	= sql

target.path	+= $$plugins.path/sqldrivers
INSTALLS	+= target
