TARGET	 = qsqlpsql

HEADERS		= ../../../sql/drivers/psql/qsql_psql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/psql/qsql_psql.cpp
unix {
	!contains( LIBS, .*pq.* ) {
	    LIBS	*= -lpq
	}
}

win32 {
        !contains(LIBS, .*pq.* ) {
            LIBS *= libpq.lib
        }
        LIBS    *= -lws2_32 -ladvapi32
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libpq.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibpq.dll
#	}
}

include(../qsqldriverbase.pri)
