# Qt xml module

xml {
	win32:XML_H	= ../include
	unix:XML_H	= xml
	unix:DEPENDPATH += :$$XML_H
	HEADERS += $$XML_H/qxml.h $$XML_H/qdom.h
	SOURCES += xml/qxml.cpp xml/qdom.cpp
}