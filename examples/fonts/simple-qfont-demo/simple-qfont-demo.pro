REQUIRES = full-config
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS	= viewer.h
SOURCES	= simple-qfont-demo.cpp \
          viewer.cpp
TARGET		= fontdemo
DEPENDPATH=../../include
