# Qt sql module

sql {

	!table {
		message(table must be enabled for sql support)
		REQUIRES += table
	}
	
	SQL_P	    = sql
	HEADERS     += $$SQL_H/qsql.h \
		    $$SQL_H/qsqlquery.h \
		    $$SQL_H/qsqldatabase.h \
		    $$SQL_H/qsqlfield.h \
		    $$SQL_H/qsqlrecord.h \
		    $$SQL_H/qsqlcursor.h \
		    $$SQL_H/qsqlform.h \
		    $$SQL_H/qeditorfactory.h \
		    $$SQL_H/qsqleditorfactory.h \
		    $$SQL_H/qsqldriver.h \
		    $$SQL_P/qsqldriverinterface_p.h \
		    $$SQL_H/qsqldriverplugin.h \
		    $$SQL_H/qsqlerror.h \
		    $$SQL_H/qsqlresult.h \
		    $$SQL_H/qsqlindex.h \
		    $$SQL_H/qsqlpropertymap.h \
		    $$SQL_P/qsqlmanager_p.h \
		    $$SQL_H/qdatatable.h \
		    $$SQL_H/qdataview.h \
		    $$SQL_H/qdatabrowser.h 

	SOURCES     += $$SQL_CPP/qsqlquery.cpp \
		    $$SQL_CPP/qsqldatabase.cpp \
		    $$SQL_CPP/qsqlfield.cpp \
		    $$SQL_CPP/qsqlrecord.cpp \
		    $$SQL_CPP/qsqlform.cpp \
		    $$SQL_CPP/qsqlcursor.cpp \
		    $$SQL_CPP/qeditorfactory.cpp \
		    $$SQL_CPP/qsqleditorfactory.cpp \
		    $$SQL_CPP/qsqldriver.cpp \
		    $$SQL_CPP/qsqldriverplugin.cpp \
		    $$SQL_CPP/qsqlerror.cpp \
		    $$SQL_CPP/qsqlresult.cpp \
		    $$SQL_CPP/qsqlindex.cpp \
		    $$SQL_CPP/qsqlpropertymap.cpp \
		    $$SQL_CPP/qsqlmanager_p.cpp \
		    $$SQL_CPP/qdatatable.cpp \
		    $$SQL_CPP/qdataview.cpp \
		    $$SQL_CPP/qdatabrowser.cpp 

	contains(sql-drivers, all ) {
		sql-driver += psql mysql odbc oci tds
	}			

	contains(sql-drivers, psql) {
		HEADERS += $$SQL_CPP/drivers/psql/qsql_psql.h
		SOURCES += $$SQL_CPP/drivers/psql/qsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			LIBS += -lpq
		}
		win32 {
			LIBS += libpqdll.lib
		}
	}

	contains(sql-drivers, mysql) {
		HEADERS += $$SQL_CPP/drivers/mysql/qsql_mysql.h
		SOURCES += $$SQL_CPP/drivers/mysql/qsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			LIBS += -lmysqlclient
		}
		win32 {
			LIBS += libmysql.lib
		}
	}
	
	contains(sql-drivers, odbc) {
		HEADERS += $$SQL_CPP/drivers/odbc/qsql_odbc.h
		SOURCES += $$SQL_CPP/drivers/odbc/qsql_odbc.cpp
		DEFINES += QT_SQL_ODBC
		unix {
			LIBS += -lodbc
		}
		win32 {
			!win32-borland:LIBS += odbc32.lib
			win32-borland:LIBS  += $(BCB)/lib/PSDK/odbc32.lib
		}
	}

	contains(sql-drivers, oci) {
		HEADERS += $$SQL_CPP/drivers/oci/qsql_oci.h
		SOURCES += $$SQL_CPP/drivers/oci/qsql_oci.cpp
		DEFINES += QT_SQL_OCI
		unix {
			LIBS += -lclntsh -lwtc8
		}
		win32 {
			LIBS += oci.lib
		}
	}

	contains(sql-drivers, tds) {
		HEADERS += $$SQL_CPP/drivers/tds/qsql_tds.h
		SOURCES += $$SQL_CPP/drivers/tds/qsql_tds.cpp
		DEFINES += QT_SQL_TDS
		unix {
			LIBS += -L$SYBASE/lib -lsybdb
		}
		win32 {
			LIBS += NTWDBLIB.LIB
		}
	}

}
