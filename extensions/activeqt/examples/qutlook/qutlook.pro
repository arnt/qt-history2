GUID 	 = {f4a5f55a-e09b-485c-b9b7-8c9f5b9ae81e}
TEMPLATE = app
TARGET	 = qutlook

CONFIG	+= qt warn_on release
LIBS    += -lqaxcontainer

HEADERS	 = centralwidget.h \
	   mainwindow.h
SOURCES	 = centralwidget.cpp \
	   main.cpp \
	   mainwindow.cpp
