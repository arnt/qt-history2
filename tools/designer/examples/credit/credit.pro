GUID 	 = {ae8087db-c58e-4355-9f01-b7f87b75c222}
TEMPLATE = app
LANGUAGE = C++
TARGET	 = credit

SOURCES	+= main.cpp
HEADERS	+= creditform.h
SOURCES += creditform.cpp
FORMS	= creditformbase.ui
CONFIG	+= qt warn_on release
DBFILE	= credit.db
IMAGEFILE	= images.cpp
