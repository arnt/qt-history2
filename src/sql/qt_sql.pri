# Qt sql module

sql {

	!table {
		message(table must be enabled for sql support)
		REQUIRES += table
	}

	HEADERS     += $$SQL_H/qsqlquery.h \
		    $$SQL_H/qsqldatabase.h \
		    $$SQL_H/qsqlfield.h \
		    $$SQL_H/qsqlrecord.h \
		    $$SQL_H/qsqlcursor.h \
		    $$SQL_H/qsqlform.h \
		    $$SQL_H/qeditorfactory.h \
		    $$SQL_H/qsqleditorfactory.h \
		    $$SQL_H/qsqldriver.h \
		    $$SQL_H/qsqldriverinterface.h \
		    $$SQL_H/qsqlerror.h \
		    $$SQL_H/qsqlresult.h \
		    $$SQL_H/qsqlindex.h \
		    $$SQL_H/qsqltable.h \
		    $$SQL_H/qsqlpropertymap.h \
		    $$SQL_H/qdatetimeedit.h \
		    $$SQL_H/qsqlnavigator.h \
		    $$SQL_H/qsqldataview.h \
		    $$SQL_H/qsqldataform.h 

	SOURCES     += $$SQL_CPP/qsqlquery.cpp \
		    $$SQL_CPP/qsqldatabase.cpp \
		    $$SQL_CPP/qsqlfield.cpp \
		    $$SQL_CPP/qsqlrecord.cpp \
		    $$SQL_CPP/qsqlform.cpp \
		    $$SQL_CPP/qsqlcursor.cpp \
		    $$SQL_CPP/qeditorfactory.cpp \
		    $$SQL_CPP/qsqleditorfactory.cpp \
		    $$SQL_CPP/qsqldriver.cpp \
		    $$SQL_CPP/qsqlerror.cpp \
		    $$SQL_CPP/qsqlresult.cpp \
		    $$SQL_CPP/qsqlindex.cpp \
		    $$SQL_CPP/qdatetimeedit.cpp \
		    $$SQL_CPP/qsqlpropertymap.cpp \
		    $$SQL_CPP/qsqltable.cpp \
		    $$SQL_CPP/qsqlnavigator.cpp \
		    $$SQL_CPP/qsqldataform.cpp \
		    $$SQL_CPP/qsqldataview.cpp 

	contains(sql-driver, all ) {
		sql-driver += psql mysql odbc oci
	}			

	contains(sql-driver, psql) {
		HEADERS += $$SQL_H/src/p$$SQL_CPP/qsql_psql.h
		SOURCES += $$SQL_CPP/src/p$$SQL_CPP/qsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			LIBS += -lpq
		}
		win32 {
			LIBS += libpqdll.lib
		}
	}

	contains(sql-driver, mysql) {
		HEADERS += $$SQL_H/src/my$$SQL_CPP/qsql_mysql.h
		SOURCES += $$SQL_CPP/src/my$$SQL_CPP/qsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			LIBS += -lmysqlclient
		}
		win32 {
			LIBS += libmysql.lib
		}
	}
	
	contains(sql-driver, odbc) {
		HEADERS += $$SQL_H/src/odbc/qsql_odbc.h
		SOURCES += $$SQL_CPP/src/odbc/qsql_odbc.cpp
		DEFINES += QT_SQL_ODBC
		unix {
			LIBS += -lodbc
		}
		win32 {
			LIBS += odbc32.lib
		}
	}

	contains(sql-driver, oci) {
		HEADERS += $$SQL_H/src/oci/qsql_oci.h
		SOURCES += $$SQL_CPP/src/oci/qsql_oci.cpp
		DEFINES += QT_SQL_OCI
		unix {
			LIBS += -lclntsh
		}
		win32 {
			LIBS += oci.lib
		}
	}

}
