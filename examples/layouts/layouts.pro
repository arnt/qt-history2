TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= qtbuttonrow.h \
		  qtgrid.h \
		  qthbox.h \
		  qtlabelled.h \
		  qtvbox.h
SOURCES		= layouts.cpp \
		  qtbuttonrow.cpp \
		  qtgrid.cpp \
		  qthbox.cpp \
		  qtlabelled.cpp \
		  qtvbox.cpp
TARGET		= layouts
DEPENDPATH=$(QTDIR)/include
