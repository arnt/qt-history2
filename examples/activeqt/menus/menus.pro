TEMPLATE = app
TARGET	 = menusax

CONFIG	+= qt warn_off qaxserver

SOURCES	 = main.cpp menus.cpp
HEADERS	 = menus.h
RC_FILE	 = ../../../extensions/activeqt/control/qaxserver.rc
