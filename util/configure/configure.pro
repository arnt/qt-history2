CONFIG+=qt
HEADERS=../install/environment.h configureapp.h
SOURCES=main.cpp ../install/environment.cpp configureapp.cpp
INTERFACES=
TARGET=configure
DESTDIR=../../bin

win32:CONFIG+=console