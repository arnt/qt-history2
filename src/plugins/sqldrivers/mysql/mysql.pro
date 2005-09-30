TARGET	 = qsqlmysql

HEADERS		= ../../../sql/drivers/mysql/qsql_mysql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/mysql/qsql_mysql.cpp

unix {
    !contains( LIBS, .*mysqlclient* ) {
        use_libmysqlclient_r:LIBS *= -lmysqlclient_r	    
        !use_libmysqlclient_r:LIBS    *= -lmysqlclient
    }
}

win32 {
    !contains(LIBS, .*mysqlclient*) {
	use_libmysqlclient_r:LIBS   *= -lmysqlclient_r
	!use_libmysqlclient_r:LIBS  *= -lmysqlclient
    }
}

include(../qsqldriverbase.pri)
