TEMPLATE = lib
CONFIG += qt release warn_off staticlib
SOURCES = qactiveqtmain.cpp qactiveqtbase.cpp ../shared/types.cpp
HEADERS = qactiveqtbase.h qactiveqt.h ../shared/types.h
TARGET = activeqt
DESTDIR = $(QTDIR)\lib
DEFINES += QAX_NODLL
