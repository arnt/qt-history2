TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qformatstuff.h \
		  qrichtextintern.h \
		  qtextbrowser.h \
		  qtextview.h
SOURCES		= main.cpp \
		  qformatstuff.cpp \
		  qrichtext.cpp \
		  qtextbrowser.cpp \
		  qtextview.cpp
TARGET		= text
