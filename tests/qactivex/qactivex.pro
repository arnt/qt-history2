CONFIG		+= qt warn_on release
HEADERS		= qactivex.h qcomobject.h
SOURCES		= qactivex.cpp qcomobject.cpp

plugin {
    TEMPLATE	= vclib
    SOURCES	+= plugin.cpp
    INCLUDEPATH += $(QTDIR)\tools\designer\interfaces
    DESTDIR	= ..\..\plugins\designer
    TARGET	= qactivexlib
}
!plugin {
    TEMPLATE	= vcapp
    SOURCES	+= main.cpp
    TARGET	= qactivexapp
}
