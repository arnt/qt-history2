TEMPLATE = app
CONFIG 	+= qt warn_off release

SOURCES = main.cpp
INCLUDEPATH += ..\..\..
LIBS += $(QTDIR)\plugins\designer\qactivex.lib
