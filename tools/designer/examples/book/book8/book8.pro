# Project ID used by some IDEs
GUID 	 = {6ef891db-0415-4d43-bd7f-39b160981a58}
TEMPLATE = app
LANGUAGE = C++

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui editbook.ui
CONFIG	+= qt warn_on release
DBFILE	= book.db
