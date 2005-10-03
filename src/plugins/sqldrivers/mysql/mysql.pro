TARGET	 = qsqlmysql

HEADERS		= ../../../sql/drivers/mysql/qsql_mysql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/mysql/qsql_mysql.cpp

unix:!contains(LIBS, .*mysqlclient.*):!contains(LIBS, .*mysqld.*) {
    use_libmysqlclient_r:LIBS     *= -lmysqlclient_r
    !use_libmysqlclient_r:LIBS    *= -lmysqlclient
}

win32:!contains(LIBS, .*mysql.*):!contains(LIBS, .*mysqld.*) {
    LIBS     *= -llibmysql    
}


include(../qsqldriverbase.pri)
