TEMPLATE = app
TARGET	 = qutlook

exists("c:\program files\Microsoft Office\Office10\msoutl.olb") {
    TYPELIBS = "c:\program files\Microsoft Office\Office10\msoutl.olb"
} else {
    message("Microsoft Outlook not installed at standard location!")
    DEFINES += NO_OUTLOOK_TLB
}

CONFIG	+= dumpcpp
LIBS    += -lqaxcontainer

HEADERS	 = centralwidget.h \
	   mainwindow.h
SOURCES	 = centralwidget.cpp \
	   main.cpp \
	   mainwindow.cpp
