GUID 	 = {bb36c74f-428b-4b3c-b67e-7f0a8228a2ef}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui
DBFILE	= book.db
