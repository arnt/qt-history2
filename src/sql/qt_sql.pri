# Qt sql module

sql {

	!table {
		message(table must be enabled for sql support)
		REQUIRES += table
	}

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
		    $$SQL_H/qsqltable.h \
		    $$SQL_H/qdatetimeedit.h

	SOURCES     += $$SQL_CPP/qsql.cpp \
		    $$SQL_CPP/qsqldatabase.cpp \
		    $$SQL_CPP/qsqlconnection.cpp \
		    $$SQL_CPP/qsqlfield.cpp \
		    $$SQL_CPP/qsqlform.cpp \
		    $$SQL_CPP/qsqlview.cpp \
		    $$SQL_CPP/qsqleditorfactory.cpp \
		    $$SQL_CPP/qsqldriver.cpp \
		    $$SQL_CPP/qsqldriverplugin.cpp \
		    $$SQL_CPP/qsqlerror.cpp \
		    $$SQL_CPP/qsqlresult.cpp \
		    $$SQL_CPP/qsqlindex.cpp \
		    $$SQL_CPP/qdatetimeedit.cpp \
		    $$SQL_CPP/qsqltable.cpp

	contains(sql-driver, postgres) {
		HEADERS += $$SQL_H/src/p$$SQL_CPP/qsql_psql.h
		SOURCES += $$SQL_CPP/src/p$$SQL_CPP/qsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			INCLUDEPATH += /usr/include/postgresql
			LIBS += -lpq
		}
	}

	contains(sql-driver, mysql) {
		HEADERS += $$SQL_H/src/my$$SQL_CPP/qsql_mysql.h
		SOURCES += $$SQL_CPP/src/my$$SQL_CPP/qsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			INCLUDEPATH += /usr/include/mysql
			LIBS += -lmysqlclient
		}
	}
	
	contains(sql-driver, odbc) {
		HEADERS += $$SQL_H/src/odbc/qsql_odbc.h
		SOURCES += $$SQL_CPP/src/odbc/qsql_odbc.cpp
		DEFINES += QT_SQL_ODBC
		unix {
			INCLUDEPATH += /usr/local/include
			LIBS += -lodbc
		}
	}

	contains(sql-driver, oci) {
		HEADERS += $$SQL_H/src/oci/qsql_oci.h
		SOURCES += $$SQL_CPP/src/oci/qsql_oci.cpp
		DEFINES += QT_SQL_OCI
		unix {
			INCLUDEPATH += /usr/local/include
			LIBS += -lclntsh
		}
	}

}
!sql:DEFINES    += QT_NO_SQL
