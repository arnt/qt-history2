TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= qsql_odbc.h 

SOURCES		= main.cpp \
		  qsql_odbc.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

isEmpty(LIBS) {
	message( "No ODBC libraries specified for linking.  See the Qt SQL Module documentation for information on building SQL driver plugins." )
}

TARGET		= qsqlodbc
DESTDIR		= ../../../../plugins/sqldrivers

target.path=$$plugins.path/sqldrivers
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/sqldrivers
INSTALLS += target
