# Qt compat module

HEADERS +=      network/q3dns.h \
                network/q3socket.h \
                network/q3socketdevice.h \
                network/q3serversocket.h

SOURCES +=      network/q3dns.cpp \
                network/q3socket.cpp \
                network/q3socketdevice.cpp \
                network/q3serversocket.cpp

win32:SOURCES +=  network/q3socketdevice_win.cpp
unix:SOURCES += network/q3socketdevice_unix.cpp


