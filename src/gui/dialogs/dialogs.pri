# Qt dialogs module

HEADERS += \
	dialogs/qabstractprintdialog.h \
	dialogs/qabstractprintdialog_p.h \
	dialogs/qcolordialog.h \
	dialogs/qdialog.h \
	dialogs/qdialog_p.h \
	dialogs/qerrormessage.h \
	dialogs/qfontdialog.h \
	dialogs/qmessagebox.h \
	dialogs/qprogressdialog.h \
	dialogs/qinputdialog.h \
	dialogs/qfiledialog.h

!embedded:mac:SOURCES	+= dialogs/qcolordialog_mac.cpp \
                           dialogs/qfiledialog_mac.cpp
win32: {
    SOURCES += dialogs/qfiledialog_win.cpp \
	       dialogs/qprintdialog_win.cpp
    HEADERS += dialogs/qprintdialog_win.h 
}
win32:LIBS += shell32.lib 	# the filedialog needs this library

SOURCES += \
	dialogs/qabstractprintdialog.cpp \
	dialogs/qcolordialog.cpp \
	dialogs/qdialog.cpp \
	dialogs/qerrormessage.cpp \
	dialogs/qfontdialog.cpp \
	dialogs/qmessagebox.cpp \
	dialogs/qprogressdialog.cpp \
	dialogs/qinputdialog.cpp \
	dialogs/qfiledialog.cpp
