SOURCES = mysql.cpp
CONFIG -= qt dylib
mac:CONFIG -= appbundle
LIBS += -lmysqlclient
