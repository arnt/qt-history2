TEMPLATE = app
TARGET	 = qutlook

!exists("c:\program files\Microsoft Office\Office10\msoutl.olb") {
    message("Microsoft Outlook type library not found at standard location!")
    REQUIRES += Outlook
} else {
    TYPELIBS = "c:\program files\Microsoft Office\Office10\msoutl.olb"
    CONFIG	+= dumpcpp
    LIBS    += -lqaxcontainer

    HEADERS  = centralwidget.h \
	       mainwindow.h
    SOURCES  = centralwidget.cpp \
	       main.cpp \
	       mainwindow.cpp
}
