REQUIRES  = table full-config
TEMPLATE  = app
CONFIG	 += qt warn_on release
HEADERS	  = productlist.h
SOURCES	  = wineorder.cpp \
            productlist.cpp
TARGET	  = wineorder
DEPENDPATH=../../include
