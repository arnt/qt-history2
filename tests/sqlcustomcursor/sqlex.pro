TEMPLATE	= app
CONFIG		+= qt warn_on debug 
SOURCES		= main.cpp 
INTERFACES      = sqlex.ui
TARGET		= sqlex
QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"
