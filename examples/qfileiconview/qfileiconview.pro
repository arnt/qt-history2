TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= mainwindow.h \
		      qfileiconview.h \
              ../../simple/dirview/dirview.h \
              ../../simple/qiconview/qiconview.h
SOURCES		= main.cpp \
	    	  mainwindow.cpp \
    		  qfileiconview.cpp \
              ../../simple/dirview/dirview.cpp \
              ../../simple/qiconview/qiconview.cpp
TARGET		= qfileiconview
DEPENDPATH=../../include
