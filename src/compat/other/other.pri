# Qt compat module

HEADERS +=      other/qdropsite.h \
		other/q3dragobject.h \
		other/qdragobject.h \
                other/q3guardedptr.h \
                other/qguardedptr.h \
                other/qlocalfs.h \
                other/qnetworkprotocol.h \
                other/qurloperator.h \
                other/q3accel.h \
                other/qaccel.h \
                other/qmimefactory.h \
		other/q3url.h \
		other/q3polygonscanner.h \
		other/q3process.h

SOURCES +=      other/qdropsite.cpp \
		other/q3dragobject.cpp \
                other/qlocalfs.cpp \
                other/qnetworkprotocol.cpp \
                other/qurloperator.cpp \
                other/q3accel.cpp \
                other/qmimefactory.cpp \
		other/q3url.cpp \
		other/q3polygonscanner.cpp \
		other/q3process.cpp

unix:SOURCES += other/q3process_unix.cpp
win32:SOURCES+= other/q3process_win.cpp


