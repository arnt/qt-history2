SOURCES	+= main.cpp oramonitorimpl.cpp 
HEADERS	+= oramonitorimpl.h 
REQUIRES += sql
TARGET = oramonitor
FORMS	= oramonitor.ui configdialog.ui 
TEMPLATE	=app
DBFILE	= oramonitor.db
LANGUAGE	= C++
