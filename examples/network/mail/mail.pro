# Project ID used by some IDEs
GUID 		= {9527f117-586b-47bb-bb67-384662e1ef3f}
TEMPLATE	= app
TARGET		= mail

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config

HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
