TEMPLATE	= lib
CONFIG		+= qt plugin
HEADERS		= ../../../../src/sql/drivers/psql/qsql_psql.h 
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/psql/qsql_psql.cpp 
unix {
	OBJECTS_DIR	= .obj
	LIBS   	*= -lpq
}

win32 {
	OBJECTS_DIR	= obj
	LIBS	*= libpqdll.lib
#	win32-msvc: { 
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libpq.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibpq.dll
#	}		
}

REQUIRES	= sql

TARGET		= qsqlpsql
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
