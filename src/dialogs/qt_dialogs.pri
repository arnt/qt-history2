# Qt dialogs module

dialogs {
	DIALOGS_P	= dialogs

	HEADERS	+= $$DIALOGS_H/qcolordialog.h \
		  $$DIALOGS_H/qdialog.h \
		  $$DIALOGS_H/qerrormessage.h \
		  $$DIALOGS_H/qfontdialog.h \
		  $$DIALOGS_H/qmessagebox.h \
		  $$DIALOGS_H/qprogressdialog.h \
		  $$DIALOGS_H/qtabdialog.h \
		  $$DIALOGS_H/qwizard.h \
		  $$DIALOGS_H/qinputdialog.h \
		  $$DIALOGS_H/qpagesetupdialog.h

        !embedded:mac:SOURCES  += $$DIALOGS_CPP/qcolordialog_mac.cpp
	unix:SOURCES += $$DIALOGS_CPP/qprintdialog.cpp
	unix:HEADERS   += $$DIALOGS_H/qprintdialog.h 

	SOURCES += $$DIALOGS_CPP/qcolordialog.cpp \
		  $$DIALOGS_CPP/qdialog.cpp \
		  $$DIALOGS_CPP/qerrormessage.cpp \
		  $$DIALOGS_CPP/qfontdialog.cpp \
		  $$DIALOGS_CPP/qmessagebox.cpp \
		  $$DIALOGS_CPP/qprogressdialog.cpp \
		  $$DIALOGS_CPP/qtabdialog.cpp \
		  $$DIALOGS_CPP/qwizard.cpp \
		  $$DIALOGS_CPP/qinputdialog.cpp \
		  $$DIALOGS_CPP/qpagesetupdialog.cpp
}
