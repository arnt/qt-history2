GUID 	 = {8448b17d-3160-4ada-9a4d-d7fd23efb70b}
TEMPLATE = app

CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = full-config

HEADERS	 = main.h
SOURCES	 = main.cpp ../connection.cpp
