GUID 		= {a6079d38-5a9d-4608-99e9-f20af77b9cc4}
TEMPLATE	= subdirs

QCONFIG		+= sql
CONFIG 		+= ordered

QTDIR_build:REQUIRES 	= full-config

SUBDIRS		= overview \
		  sqltable \
		  blob
