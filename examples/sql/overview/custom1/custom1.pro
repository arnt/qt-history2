# Project ID used by some IDEs
GUID 	 = {e2e99ab9-5c47-4e52-9f52-2747675e15d1}
TEMPLATE = app

CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = full-config

HEADERS	 = main.h
SOURCES	 = main.cpp ../connection.cpp
