# Project ID used by some IDEs
GUID 	 = {93bc4f0a-4318-49d4-8a99-0c1256125f96}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release
SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui editbook.ui
DBFILE	= book.db
