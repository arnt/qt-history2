GUID 	 = {7c80b107-8807-48e4-abe2-ff4183a2c5a2}
TEMPLATE = app
LANGUAGE = C++
TARGET	 = filechooser

SOURCES	+= filechooser.cpp main.cpp
HEADERS	+= filechooser.h
CONFIG	+= qt warn_on release
DBFILE	= filechooser.db
DEFINES += FILECHOOSER_IS_WIDGET
