TEMPLATE	= lib
CONFIG		+= qt plugin
HEADERS		= ../../../../src/sql/drivers/psql/qsql_psql.h 
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/psql/qsql_psql.cpp 
unix {
	OBJECTS_DIR	= .obj
	isEmpty(LIBS) {
		LIBS    += -lpq
	}
}

win32 {
	OBJECTS_DIR	= obj
	isEmpty(LIBS) {
		LIBS	+= libpqdll.lib
	}
}

REQUIRES	= sql

TARGET		= qsqlpsql
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
