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

	!contains( LIBS, .*clnts.* ) {
	    LIBS	*= -lclntsh
	    LIBS	*= -lwtc8
	}
}
macx {
        LIBS -= -lwtc8
        QMAKE_LFLAGS += -Wl,-flat_namespace,-U,_environ
}

REQUIRES	= sql

DESTDIR		= ../../../sqldrivers
TARGET		= qsqloci


target.path += $$plugins.path/sqldrivers
INSTALLS += target
