TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS	= input_pen.h \
	  qimpen/qimpeninput.h \
	qimpen/qimpenchar.h \
	  qimpen/qimpenwidget.h \
	 qimpen/qimpenstroke.h \
	qimpen/qimpensetup.h 
SOURCES =  input_pen.cpp \
	 qimpen/qimpeninput.cpp \
	qimpen/qimpenchar.cpp \
	  qimpen/qimpenwidget.cpp\
	 qimpen/qimpensetup.cpp \
	 qimpen/qimpenstroke.cpp \
	launcher.cpp
TARGET		= compact
