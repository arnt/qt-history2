TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= cursors.h \
		  dialogs.h \
		  pingpongfrontend.h \
		  widgets.h
SOURCES		= cursors.cpp \
		  dialogs.cpp \
		  main.cpp \
		  pingpongfrontend.cpp \
		  widgets.cpp
INTERFACES	= matchdialogbase.ui \
		  teameditorbase.ui \
		  statisticsbase.ui
unix:LIBS      += -lpthread
