# Qt table module

table {
	win32:TABLE_H	= ../include
	unix:TABLE_H	= table
	unix:DEPENDPATH += :$$TABLE_H
	HEADERS += $$TABLE_H/qtable.h
	SOURCES += table/qtable.cpp
}