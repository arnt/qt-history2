TEMPLATE = app
TARGET	 = qutlook

!exists("c:\program files\Microsoft Office\Office10\msoutl.olb") {
    message("Microsoft Outlook type library not found at standard location!")
    REQUIRES += Outlook
} else {
    TYPELIBS = "c:\program files\Microsoft Office\Office10\msoutl.olb"
    CONFIG	+= dumpcpp qaxcontainer

    HEADERS  = addressview.h
    SOURCES  = addressview.cpp main.cpp
}
