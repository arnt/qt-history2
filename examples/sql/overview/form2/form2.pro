# Project ID used by some IDEs
GUID 	 = {1095392f-2c0e-44e9-87a2-35615b9d8034}
TEMPLATE = app

QCONFIG += sql
CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = full-config

HEADERS	 = main.h
SOURCES	 = main.cpp ../connection.cpp

