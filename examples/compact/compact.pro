TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS	= ../../util/qws/input_pen.h \
	  ../../util/qws/qimpen/qimpeninput.h \
	../../util/qws/qimpen/qimpenchar.h \
	  ../../util/qws/qimpen/qimpenwidget.h \
	 ../../util/qws/qimpen/qimpenstroke.h \
	../../util/qws/qimpen/qimpensetup.h 
SOURCES =  ../../util/qws/input_pen.cpp \
	 ../../util/qws/qimpen/qimpeninput.cpp \
	../../util/qws/qimpen/qimpenchar.cpp \
	  ../../util/qws/qimpen/qimpenwidget.cpp\
	 ../../util/qws/qimpen/qimpensetup.cpp \
	 ../../util/qws/qimpen/qimpenstroke.cpp \
	launcher.cpp
TARGET		= compact
