# Project ID used by some IDEs
GUID	 = {7b4c7a80-1bd5-436d-9fdf-9da88797ec62}
TEMPLATE = app
TARGET	 = menusax

CONFIG	+= qt warn_off activeqt

SOURCES	 = main.cpp menus.cpp
HEADERS	 = menus.h
RC_FILE	 = ../../control/qaxserver.rc
