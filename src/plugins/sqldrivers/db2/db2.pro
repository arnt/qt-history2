TEMPLATE = lib
TARGET	 = qsqldb2

CONFIG	+= qt plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/sqldrivers

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

QTDIR_build:REQUIRES	= sql

target.path	+= $$plugins.path/sqldrivers
INSTALLS	+= target
