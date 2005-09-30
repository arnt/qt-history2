TARGET	 = qsqlpsql

HEADERS		= ../../../sql/drivers/psql/qsql_psql.h
SOURCES		= main.cpp \
		  ../../../sql/drivers/psql/qsql_psql.cpp

unix:!contains( LIBS, .*pq.* ):LIBS	*= -lpq
	
win32:!contains(LIBS, .*pq.* ) {
    LIBS    *= -lpq       
    LIBS    *= -lws2_32 -ladvapi32
}

include(../qsqldriverbase.pri)
