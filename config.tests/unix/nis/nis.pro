SOURCES = nis.cpp
CONFIG -= qt dylib
mac: CONFIG -= appbundle
solaris-*:LIBS += -lnsl
else:LIBS += $$QMAKE_LIBS_NIS
