# Project ID used by some IDEs
GUID 		= {e89c21c1-f771-4ff7-b75d-27ba39dd6e1c}
TEMPLATE	= app
TARGET		= progressbar

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= progressbar.h
SOURCES		= main.cpp \
		  progressbar.cpp
