# Qt workspace module

workspace {
	win32:WORKSPACE_H	= ../include
	unix:WORKSPACE_H	= workspace
	unix:DEPENDPATH += :$$WORKSPACE_H
	HEADERS += $$WORKSPACE_H/qworkspace.h
	SOURCES += workspace/qworkspace.cpp
}