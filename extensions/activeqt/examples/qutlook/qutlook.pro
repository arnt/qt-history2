TEMPLATE = app
TARGET	 = qutlook

CONFIG	+= qt warn_on release
LIBS    += -lqaxcontainer

HEADERS	 = centralwidget.h \
	   mainwindow.h
SOURCES	 = centralwidget.cpp \
	   main.cpp \
	   mainwindow.cpp
