TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= parser.h \
		  pie.h \
		  sheet.h \
		  sheetdlg.h \
		  table.h
SOURCES		= main.cpp \
		  parser.cpp \
		  pie.cpp \
		  sheet.cpp \
		  sheetdlg.cpp \
		  table.cpp
TARGET		= sheet
DEPENDPATH=$(QTDIR)/include
