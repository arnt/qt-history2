TEMPLATE	= lib
CONFIG		+= qt plugin
HEADERS		= ../../../../src/sql/drivers/ibase/qsql_ibase.h 
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/ibase/qsql_ibase.cpp \
		  ../../../../src/sql/drivers/cache/qsqlcachedresult.cpp

unix {
	OBJECTS_DIR = .obj
	
	!contains( LIBS, .*gds.* ) {
	    LIBS    *= -lgds
	}
}
win32 {
	OBJECTS_DIR = obj
#	LIBS	*= libmysql.lib
#	win32-msvc: { 
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibmysql.dll
#	}		
}

REQUIRES	= sql

TARGET		= qsqlibase
DESTDIR		= ../../../sqldrivers


target.path += $$plugins.path/sqldrivers
INSTALLS += target
