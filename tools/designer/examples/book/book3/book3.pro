# Project ID used by some IDEs
GUID 	 = {221624ca-74f4-4e9b-b1c1-d1ec2331aefc}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui
DBFILE	= book.db
