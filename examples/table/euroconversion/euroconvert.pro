REQUIRES  = table full-config
TEMPLATE  = app
CONFIG	 += qt warn_on release
HEADERS	  = converter.h
SOURCES	  = converter.cpp \
            euroconvert.cpp
TARGET	  = euroconvert
DEPENDPATH=../../include
