# Qt dialogs module

HEADERS += \
	dialogs/qabstractprintdialog.h \
	dialogs/qabstractprintdialog_p.h \
	dialogs/qcolordialog.h \
	dialogs/qdialog.h \
	dialogs/qdialog_p.h \
	dialogs/qerrormessage.h \
	dialogs/qfiledialog.h \
	dialogs/qfontdialog.h \
	dialogs/qinputdialog.h \
	dialogs/qmessagebox.h \
	dialogs/qprintdialog.h \
	dialogs/qprogressdialog.h

!embedded:mac:SOURCES	+= dialogs/qcolordialog_mac.cpp \
                           dialogs/qfiledialog_mac.cpp
win32: {
    SOURCES += dialogs/qfiledialog_win.cpp \
	       dialogs/qprintdialog_win.cpp
    HEADERS += dialogs/qprintdialog_win.h 
}
win32:LIBS += shell32.lib 	# the filedialog needs this library

unix:{
	SOURCES += dialogs/qprintdialog_unix.cpp
	HEADERS += dialogs/qprintdialog_unix.h
}

SOURCES += \
	dialogs/qabstractprintdialog.cpp \
	dialogs/qcolordialog.cpp \
	dialogs/qdialog.cpp \
	dialogs/qerrormessage.cpp \
	dialogs/qfiledialog.cpp \
	dialogs/qfontdialog.cpp \
	dialogs/qinputdialog.cpp \
	dialogs/qmessagebox.cpp \
	dialogs/qprintdialog.cpp \
	dialogs/qprogressdialog.cpp 
