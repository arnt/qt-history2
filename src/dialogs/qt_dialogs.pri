# Qt dialogs module

dialogs {
	win32:DIALOGS_H	= ../include
	unix:DIALOGS_H	= dialogs
	DIALOGS_P	= dialogs
	unix:DEPENDPATH += :$$DIALOGS_H

	HEADERS	+= $$DIALOGS_H/qcolordialog.h \
		  $$DIALOGS_H/qerrormessage.h \
		  $$DIALOGS_H/qfiledialog.h \
		  $$DIALOGS_H/qfontdialog.h \
		  $$DIALOGS_H/qmessagebox.h \
		  $$DIALOGS_H/qprogressdialog.h \
		  $$DIALOGS_H/qtabdialog.h \
		  $$DIALOGS_H/qwizard.h \
		  $$DIALOGS_H/qinputdialog.h

	win32:SOURCES += dialogs/qfiledialog_win.cpp
	unix:SOURCES += dialogs/qprintdialog.cpp
	unix:HEADERS   += $$DIALOGS_H/qprintdialog.h 

	SOURCES += dialogs/qcolordialog.cpp \
		  dialogs/qerrormessage.cpp \
		  dialogs/qfiledialog.cpp \
		  dialogs/qfontdialog.cpp \
		  dialogs/qmessagebox.cpp \
		  dialogs/qprogressdialog.cpp \
		  dialogs/qtabdialog.cpp \
		  dialogs/qwizard.cpp \
		  dialogs/qinputdialog.cpp
}
