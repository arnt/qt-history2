TEMPLATE	= lib
CONFIG+= qt warn_on release plugin

HEADERS		= ../../../../src/sql/drivers/odbc/qsql_odbc.h 

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/odbc/qsql_odbc.cpp

unix:OBJECTS_DIR	= .obj

win32 {
	OBJECTS_DIR		= obj
	!win32-borland:LIBS	+= odbc32.lib
	win32-borland:LIBS	+= $(BCB)/lib/PSDK/odbc32.lib
}

!isEmpty(LIBS) {
	CONFIG += odbc_libs_and_headers
}
!isEmpty(INCLUDEPATH) {
	CONFIG += odbc_libs_and_headers
}

REQUIRES	= sql odbc_libs_and_headers

TARGET		= qsqlodbc
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
