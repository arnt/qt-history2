TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS         = scribble.h \
                  tabletstats.h
SOURCES		= main.cpp \
                  scribble.cpp \
                  tabletstats.cpp
TARGET		= scribble
DEPENDPATH=../../include
REQUIRES=full-config
