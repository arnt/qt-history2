# Qt sql module
# Recognized CONFIG switches for adding sql support:
#
#	sql		- enable sql module
#	sql_postgres	- link with postgres
#	sql_mysql	- link with mysql
#	sql_odbc	- link with odbc
#

sql_postgres:CONFIG += sql
sql_mysql:CONFIG += sql
sql_odbc:CONFIG += sql

!sql:DEFINES    += QT_NO_SQL

sql {
	win32:SQL_H	= ../include
	unix:SQL_H	= sql
	unix:DEPENDPATH += :$$SQL_H

	HEADERS     += $$SQL_H/qsql.h \
		    $$SQL_H/qsqlconnection.h \
		    $$SQL_H/qsqldatabase.h \
		    $$SQL_H/qsqlfield.h \
		    $$SQL_H/qsqlview.h \
		    $$SQL_H/qsqlform.h \
		    $$SQL_H/qsqleditorfactory.h \
		    $$SQL_H/qsqldriver.h \
		    $$SQL_H/qsqldriverinterface.h \
		    $$SQL_H/qsqldriverplugin.h \
		    $$SQL_H/qsqlerror.h \
		    $$SQL_H/qsqlresult.h \
		    $$SQL_H/qsqlindex.h \
		    $$SQL_H/qsqltable.h

	SOURCES     += sql/qsql.cpp \
		    sql/qsqldatabase.cpp \
		    sql/qsqlconnection.cpp \
		    sql/qsqlfield.cpp \
		    sql/qsqlform.cpp \
		    sql/qsqlview.cpp \
		    sql/qsqleditorfactory.cpp \
		    sql/qsqldriver.cpp \
		    sql/qsqldriverplugin.cpp \
		    sql/qsqlerror.cpp \
		    sql/qsqlresult.cpp \
		    sql/qsqlindex.cpp \
		    sql/qsqltable.cpp

	sql_postgres {
		HEADERS += $$SQL_H/src/psql/qsql_psql.h
		SOURCES += sql/src/psql/qsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			INCLUDEPATH += /usr/include/postgresql
			LIBS += -lpq
		}
	}

	sql_mysql {
		HEADERS += $$SQL_H/src/mysql/qsql_mysql.h
		SOURCES += sql/src/mysql/qsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			INCLUDEPATH += /usr/include/mysql
			LIBS += -lmysqlclient
		}
	}
	
	sql_odbc {
		HEADERS += $$SQL_H/src/odbc/qsql_odbc.h
		SOURCES += sql/src/odbc/qsql_odbc.cpp
		DEFINES += QT_SQL_ODBC
		unix {
			INCLUDEPATH += /usr/local/include
			LIBS += -lodbc
		}
	}
}

