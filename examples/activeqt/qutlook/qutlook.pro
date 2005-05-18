TEMPLATE = app
TARGET	 = qutlook
CONFIG	+= qaxcontainer

TYPELIBS = $$system(dumpcpp -getfile {00062FFF-0000-0000-C000-000000000046})

isEmpty(TYPELIBS) {
    message("Microsoft Outlook type library not found!")
    REQUIRES += Outlook
} else {
    HEADERS  = addressview.h
    SOURCES  = addressview.cpp main.cpp
}
