# Project ID used by some IDEs
GUID 	 = {a675f46b-3cbc-483d-ac5e-1ce1ccd109d1}
TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui editbook.ui
DBFILE	= book.db
