TARGET	 = qsqlmysql

HEADERS		= ../../../sql/drivers/mysql/qsql_mysql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/mysql/qsql_mysql.cpp

unix {
	!contains( LIBS, .*mysql* ) {
	    use_libmysqlclient_r {
		LIBS *= -lmysqlclient_r
	    }
	    !use_libmysqlclient_r {		
		LIBS    *= -lmysqlclient
	    }
	}
}
win32 {
        !contains(LIBS, .*mysql*) {
	    use_libmysqlclient_r {
		LIBS *= libmysql_r.lib
	    }
	    !use_libmysqlclient_r {		
		LIBS    *= libmysql.lib
	    }
        }
#	win32-msvc: {
#		LIBS *= delayimp.lib
#		QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#	}
#	win32-borland: {
#		QMAKE_LFLAGS += /dlibmysql.dll
#	}
}

include(../qsqldriverbase.pri)
