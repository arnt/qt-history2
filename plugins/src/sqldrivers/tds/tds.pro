TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../src/sql/drivers/tds/qsql_tds.h \
		  ../../../../src/sql/drivers/shared/qsql_result.h

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/tds/qsql_tds.cpp \
		  ../../../../src/sql/drivers/shared/qsql_result.cpp

unix {
	OBJECTS_DIR	= .obj
	isEmpty(LIBS) {
		LIBS += -lsybdb
	}
}
win32 {
	OBJECTS_DIR	= obj
	isEmpty(LIBS) {
		LIBS += NTWDBLIB.LIB
	}
}

REQUIRES	= sql

TARGET		= qsqltds
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
