TEMPLATE	= lib
CONFIG		+= qt plugin

HEADERS		= ../../../../src/sql/drivers/db2/qsql_db2.h 

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/db2/qsql_db2.cpp

unix {
	OBJECTS_DIR	= .obj
	!contains( LIBS, .*db2.* ) {
	    LIBS 	*= -ldb2
	}
}

REQUIRES	= sql

TARGET		= qsqldb2
DESTDIR		= ../../../sqldrivers


target.path	+= $$plugins.path/sqldrivers
INSTALLS	+= target
