SOURCES	+= main.cpp oramonitorimpl.cpp 
HEADERS	+= oramonitorimpl.h 
QTDIR_build:REQUIRES += sql
TARGET = oramonitor
FORMS	= oramonitor.ui configdialog.ui 
TEMPLATE	=app
DBFILE	= oramonitor.db
LANGUAGE	= C++
