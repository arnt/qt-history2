# Project ID used by some IDEs
GUID 		= {3e75489d-79f3-4179-85bf-4a1edceffad3}
TEMPLATE	= app
TARGET		= cursor

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= small-config

HEADERS		=
SOURCES		= cursor.cpp

