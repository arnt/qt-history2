# Project ID used by some IDEs
GUID 	 = {50985a3a-cb1b-46a5-b289-f7dc75d0f259}
TEMPLATE = app

CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = full-config

HEADERS	 = main.h
SOURCES	 = main.cpp ../connection.cpp
