GUID 	 = {db19dd1b-fb61-42b7-91e8-a6d0d4a7d777}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui
DBFILE	= book.db
