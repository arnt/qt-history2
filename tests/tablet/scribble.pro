TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS         = canvas.h \
                  scribble.h \
                  tabletstats.h
SOURCES         = canvas.cpp \
                  main.cpp \
                  scribble.cpp \
                  tabletstats.cpp
TARGET		= scribble
DEPENDPATH=../../include
REQUIRES=full-config
