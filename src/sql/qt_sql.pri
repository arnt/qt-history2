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
		    $$SQL_H/qdatabrowser.h \ 
		    $$SQL_H/qsqlselectcursor.h 

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
		    $$SQL_CPP/qdatabrowser.cpp \
		    $$SQL_CPP/qsqlselectcursor.cpp

	contains(sql-drivers, all ) {
		sql-driver += psql mysql odbc oci tds db2 sqlite
	}			

	contains(sql-drivers, psql) {
		HEADERS += $$SQL_CPP/drivers/psql/qsql_psql.h
		SOURCES += $$SQL_CPP/drivers/psql/qsql_psql.cpp
		DEFINES += QT_SQL_POSTGRES
		unix {
			!contains( LIBS, .*pq.* ) {
				LIBS *= -lpq
			}
		}
		win32 {
			!contains( LIBS, .*libpq.* ) {
				LIBS *= libpqdll.lib
			}
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:libpqdll.dll
#			}
#			win32-borland: {
#				QMAKE_LFLAGS += /dlibpqdll.dll
#			}		
		}
	}

	contains(sql-drivers, mysql) {
		HEADERS += $$SQL_CPP/drivers/mysql/qsql_mysql.h
		SOURCES += $$SQL_CPP/drivers/mysql/qsql_mysql.cpp
		DEFINES += QT_SQL_MYSQL
		unix {
			!contains( LIBS, .*mysql.* ) {
				LIBS    *= -lmysqlclient
			}
		}
		win32 {
			!contains( LIBS, .*mysql.* ) {
				LIBS    *= libmysql.lib
			}
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:libmysql.dll
#			}
#			win32-borland: {
#				QMAKE_LFLAGS += /dlibmysql.dll
#			}
		}
	}
	
	contains(sql-drivers, odbc) {
		HEADERS += $$SQL_CPP/drivers/odbc/qsql_odbc.h
		SOURCES += $$SQL_CPP/drivers/odbc/qsql_odbc.cpp
		DEFINES += QT_SQL_ODBC

		mac {
			!contains( LIBS, .*odbc.* ) {
				LIBS        *= -liodbc
			}
		}

		unix {
			!contains( LIBS, .*odbc.* ) {
				LIBS        *= -lodbc
			}
		}

		win32 {
			!win32-borland:LIBS     *= odbc32.lib
			win32-borland:LIBS      *= $(BCB)/lib/PSDK/odbc32.lib
		}

	}

	contains(sql-drivers, oci) {
		HEADERS += $$SQL_CPP/drivers/oci/qsql_oci.h
		SOURCES += $$SQL_CPP/drivers/oci/qsql_oci.cpp
		DEFINES += QT_SQL_OCI
		unix {
			!contains( LIBS, .*clnts.* ) {
			    LIBS += -lclntsh -lwtc8
			}
		}
		win32 {
			LIBS += oci.lib
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:oci.dll
#			}
#			win32-borland: {
#				QMAKE_LFLAGS += /doci.dll
#			}		
		}
	}

	contains(sql-drivers, tds) {
		HEADERS += $$SQL_CPP/drivers/tds/qsql_tds.h \
			   $$SQL_CPP/drivers/shared/qsql_result.h
		SOURCES += $$SQL_CPP/drivers/tds/qsql_tds.cpp \
			   $$SQL_CPP/drivers/shared/qsql_result.cpp
		DEFINES += QT_SQL_TDS
		unix {
			LIBS += -L$SYBASE/lib -lsybdb
		}
		win32 {
			!win32-borland:LIBS += NTWDBLIB.LIB
			win32-borland:LIBS += $(BCB)/lib/PSDK/NTWDBLIB.LIB
#			win32-msvc: { 
#				LIBS *= delayimp.lib
#				QMAKE_LFLAGS += /DELAYLOAD:ntwdblib.dll
#			}
#			win32-borland: {
#				QMAKE_LFLAGS += /dntwdblib.dll
#			}		
		}
	}

	contains(sql-drivers, db2) {
		HEADERS += $$SQL_CPP/drivers/db2/qsql_db2.h
		SOURCES += $$SQL_CPP/drivers/db2/qsql_db2.cpp
		DEFINES += QT_SQL_DB2
		unix {
			LIBS += -ldb2
		}
		win32 {
			!win32-borland:LIBS += db2cli.lib
#			win32-borland:LIBS  += $(BCB)/lib/PSDK/db2cli.lib
		}
	}

        contains(sql-drivers, sqlite) {
                HEADERS += $$SQL_CPP/drivers/sqlite/qsql_sqlite.h
                SOURCES += $$SQL_CPP/drivers/sqlite/qsql_sqlite.cpp
                DEFINES += QT_SQL_SQLITE
                unix {
                        LIBS += -lsqlite
                }
                win32 {
                        LIBS += sqlite.lib
                }
        }
}

