TEMPLATE	= subdirs
QCONFIG += sql
SUBDIRS		= overview sqltable blob
CONFIG += ordered
QTDIR_build:REQUIRES=full-config
