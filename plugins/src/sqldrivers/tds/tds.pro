TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../src/sql/drivers/tds/qsql_tds.h \
		  ../../../../src/sql/drivers/shared/qsql_result.h

SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/tds/qsql_tds.cpp \
		  ../../../../src/sql/drivers/shared/qsql_result.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

isEmpty(LIBS) {
	message( "No TDS libraries specified for linking.  See the Qt SQL Module documentation for information on building SQL driver plugins." )
}

TARGET		= qsqltds
DESTDIR		= ../../../sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
