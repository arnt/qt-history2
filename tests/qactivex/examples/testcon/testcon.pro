SOURCES	+= main.cpp 

TARGET	    = testcon
FORMS	= mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui 
TEMPLATE	=app
CONFIG	+= qt warn_off release
INCLUDEPATH	+= ../../container
LIBS	+= $(QTDIR)/lib/qactivex.lib
DBFILE	= testcon.db
LANGUAGE	= C++
