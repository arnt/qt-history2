TEMPLATE	= lib
CONFIG+= qt warn_on release plugin
HEADERS		= ../../../../src/sql/drivers/oci/qsql_oci.h
SOURCES		= main.cpp \
		  ../../../../src/sql/drivers/oci/qsql_oci.cpp 

win32 {
	OBJECTS_DIR	= obj
	LIBS	*= oci.lib
#	win32-msvc: { 
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:oci.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /doci.dll
#	}
}
unix {
	OBJECTS_DIR	= .obj
	LIBS		*= -lclntsh
	LIBS            *= -lwtc8
}

REQUIRES	= sql

DESTDIR		= ../../../sqldrivers
TARGET		= qsqloci

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/sqldrivers
INSTALLS += target
