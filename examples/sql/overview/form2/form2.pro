TEMPLATE = app

QT += sql
CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = full-config

HEADERS	 = main.h
SOURCES	 = main.cpp ../connection.cpp

