TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS	= inputpen.h \
	keyboard.h \ 
	unikeyboard.h \
	  qimpen/qimpeninput.h \
	qimpen/qimpenchar.h \
	  qimpen/qimpenwidget.h \
	 qimpen/qimpenstroke.h \
	qimpen/qimpensetup.h 
SOURCES =  inputpen.cpp \
	keyboard.cpp \ 
	unikeyboard.cpp \
	 qimpen/qimpeninput.cpp \
	qimpen/qimpenchar.cpp \
	  qimpen/qimpenwidget.cpp\
	 qimpen/qimpensetup.cpp \
	 qimpen/qimpenstroke.cpp \
	launcher.cpp
TARGET		= compact
