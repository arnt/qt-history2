GUID 	 = {6cc0eb7b-1d4f-48cb-b58a-28d731a33f04}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui editbook.ui
DBFILE	= book.db
