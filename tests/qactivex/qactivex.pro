CONFIG		+= qt warn_on release
HEADERS		= qactivex.h
SOURCES		= qactivex.cpp

plugin {
    TEMPLATE	= vclib
    SOURCES	+= plugin.cpp
    INCLUDEPATH += $(QTDIR)\tools\designer\interfaces
    DESTDIR	= ..\..\plugins\designer
    TARGET	= qactivexlib
}
!plugin {
    TEMPLATE	= vcapp
    CONFIG	+= console
    SOURCES	+= main.cpp
    TARGET	= qactivexapp
}
