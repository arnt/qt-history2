# Qt dialogs module

HEADERS += \
	dialogs/qcolordialog.h \
	dialogs/qdialog.h \
	dialogs/qdialog_p.h \
	dialogs/qerrormessage.h \
	dialogs/qfontdialog.h \
	dialogs/qmessagebox.h \
	dialogs/qprogressdialog.h \
	dialogs/qtabdialog.h \
	dialogs/qwizard.h \
	dialogs/qinputdialog.h \
	dialogs/qpagesetupdialog.h

!embedded:mac:SOURCES	+= dialogs/qcolordialog_mac.cpp
unix:SOURCES += dialogs/qprintdialog.cpp
unix:HEADERS += dialogs/qprintdialog.h 

SOURCES += \
	dialogs/qcolordialog.cpp \
	dialogs/qdialog.cpp \
	dialogs/qerrormessage.cpp \
	dialogs/qfontdialog.cpp \
	dialogs/qmessagebox.cpp \
	dialogs/qprogressdialog.cpp \
	dialogs/qtabdialog.cpp \
	dialogs/qwizard.cpp \
	dialogs/qinputdialog.cpp \
	dialogs/qpagesetupdialog.cpp
