REQUIRES  = table full-config
TEMPLATE  = app
CONFIG	 += qt warn_on release
HEADERS	  = productlist.h \
            spinboxitem.h
SOURCES	  = wineorder.cpp \
            productlist.cpp \
            spinboxitem.cpp
TARGET	  = wineorder
DEPENDPATH=../../include
