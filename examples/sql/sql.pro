TEMPLATE	= subdirs

QT		+= sql
CONFIG 		+= ordered

QTDIR_build:REQUIRES 	= full-config

SUBDIRS		= overview \
		  sqltable \
		  blob
