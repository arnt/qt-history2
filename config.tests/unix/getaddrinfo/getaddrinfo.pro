SOURCES = getaddrinfotest.cpp
CONFIG -= qt dylib
mac:CONFIG -= appbundle
solaris-cc*:LIBS += -lsocket -lnsl
