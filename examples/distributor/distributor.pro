TEMPLATE	= app
LANGUAGE	= C++
TARGET		= distributor

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

SOURCES		+= main.cpp
FORMS		= distributor.ui
