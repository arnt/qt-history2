TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= editor.h
SOURCES		= main.cpp \
                  editor.cpp 
TARGET		= actiongroup
DEPENDPATH=../../../include
REQUIRES=full-config
