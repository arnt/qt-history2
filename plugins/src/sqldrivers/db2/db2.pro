TEMPLATE	= lib
CONFIG		+= qt plugin

HEADERS		= ../../../../src/sql/drivers/db2/qsql_db2.h 

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/db2/qsql_db2.cpp

unix {
	OBJECTS_DIR	= .obj
#	!contains( LIBS, .*odbc.* ) {
#	    LIBS 	*= -lodbc
#	}
}

#win32 {
#	OBJECTS_DIR		= obj
#	!win32-borland:LIBS	*= odbc32.lib
#	win32-borland:LIBS	*= $(BCB)/lib/PSDK/odbc32.lib
#}

REQUIRES	= sql

TARGET		= qsqldb2
DESTDIR		= ../../../sqldrivers


target.path	+= $$plugins.path/sqldrivers
INSTALLS	+= target
