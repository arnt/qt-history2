TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= mainwindow.h \
		      qfileiconview.h \
              ../dirview/dirview.h \
              ../qiconview/qiconview.h
SOURCES		= main.cpp \
	    	  mainwindow.cpp \
    		  qfileiconview.cpp \
              ../dirview/dirview.cpp \
              ../qiconview/qiconview.cpp
TARGET		= qfileiconview
DEPENDPATH=../../include
