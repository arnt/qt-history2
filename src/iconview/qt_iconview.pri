# Qt iconview module

iconview {
	win32:ICONVIEW_H	= ../include
	unix:ICONVIEW_H	= iconview
	unix:DEPENDPATH += :$$ICONVIEW_H
	HEADERS += $$ICONVIEW_H/qiconview.h
	SOURCES += iconview/qiconview.cpp
}