TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qvfb.h qvfbview.h qvfbratedlg.h qanimationwriter.h
SOURCES		= qvfb.cpp qvfbview.cpp qvfbratedlg.cpp \
		  main.cpp qanimationwriter.cpp
TARGET		= qvfb
DEPENDPATH=../../include
