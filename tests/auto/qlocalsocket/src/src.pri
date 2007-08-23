DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT += network

SOURCES += $$PWD/qlocalserver.cpp \
	   $$PWD/qlocalsocket.cpp

unix:SOURCES += $$PWD/qlocalserver_unix.cpp \
                $$PWD/qlocalsocket_unix.cpp \

win32:SOURCES += $$PWD/qlocalserver_win.cpp \
                 $$PWD/qlocalsocket_win.cpp

win32:HEADERS += $$PWD/qlocalserver_p.h

HEADERS += $$PWD/qlocalsocket.h \
           $$PWD/qlocalsocket_p.h \
	   $$PWD/qlocalserver.h \
