TEMPLATE	= app
LANGUAGE	= C++
TARGET		= distributor

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= full-config

SOURCES		+= main.cpp
FORMS		= distributor.ui
