SOURCES	+= main.cpp ../connection.cpp 
TARGET	= book
FORMS	= book.ui editbook.ui 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= book.db
LANGUAGE	= C++
