# Qt compat module

HEADERS +=      network/q3dns.h \
                network/qsocket.h \
                network/qsocketdevice.h \
                network/qsocketdevice_p.h \
                network/qserversocket.h

SOURCES +=      network/q3dns.cpp \
                network/qsocket.cpp \
                network/qsocketdevice.cpp \
                network/qserversocket.cpp

win32:SOURCES +=  network/qsocketdevice_win.cpp
unix:SOURCES += network/qsocketdevice_unix.cpp


