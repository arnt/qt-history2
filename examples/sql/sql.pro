TEMPLATE	= subdirs

QT		+= sql
CONFIG 		+= ordered

QTDIR_build:REQUIRES 	= "contains(QT_CONFIG, full-config)"

SUBDIRS		= overview \
		  sqltable \
		  blob
