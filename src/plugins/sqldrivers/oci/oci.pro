TARGET	 = qsqloci

HEADERS		= ../../../sql/drivers/oci/qsql_oci.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/oci/qsql_oci.cpp

win32 {
	LIBS	*= -loci
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:oci.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /doci.dll
#	}
}
unix {
	!contains( LIBS, .*clnts.* ) {
	    LIBS	*= -lclntsh
	}
}
macx {
        QMAKE_LFLAGS += -Wl,-flat_namespace,-U,_environ
}

include(../qsqldriverbase.pri)
