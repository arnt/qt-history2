GUID 	 = {ba58e99f-7f56-47c7-83af-98edac9c11e4}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui
DBFILE	= book.db
