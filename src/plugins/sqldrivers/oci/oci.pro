TEMPLATE = lib
TARGET	 = qsqloci

CONFIG  += plugin
QCONFIG  = core sql
DESTDIR	 = $$QT_BUILD_TREE/plugins/sqldrivers

HEADERS		= ../../../sql/drivers/oci/qsql_oci.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/oci/qsql_oci.cpp

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

QTDIR_build:REQUIRES	= sql

target.path += $$plugins.path/sqldrivers
INSTALLS += target
