TEMPLATE = app
TARGET	 = qutlook

CONFIG	+= dumpcpp
LIBS    += -lqaxcontainer

HEADERS	 = centralwidget.h \
	   mainwindow.h
SOURCES	 = centralwidget.cpp \
	   main.cpp \
	   mainwindow.cpp

TYPELIBS = "d:\program files\Microsoft Office\Office10\msoutl.olb"
