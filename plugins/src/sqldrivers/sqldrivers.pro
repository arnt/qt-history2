# Project ID used by some IDEs
GUID 	 = {81d20869-90db-413c-978b-f00e0818144a}
TEMPLATE = subdirs

contains(sql-plugins, psql)	: SUBDIRS += psql
contains(sql-plugins, mysql)	: SUBDIRS += mysql
contains(sql-plugins, odbc)	: SUBDIRS += odbc
contains(sql-plugins, tds)	: SUBDIRS += tds
contains(sql-plugins, oci)	: SUBDIRS += oci
contains(sql-plugins, db2)	: SUBDIRS += db2
contains(sql-plugins, sqlite)	: SUBDIRS += sqlite
contains(sql-plugins, ibase)	: SUBDIRS += ibase
