# Qt dialogs module

HEADERS += \
	dialogs/qcolordialog.h \
	dialogs/qdialog.h \
	dialogs/qdialog_p.h \
	dialogs/qerrormessage.h \
	dialogs/qfontdialog.h \
	dialogs/qmessagebox.h \
	dialogs/qprogressdialog.h \
	dialogs/qwizard.h \
	dialogs/qinputdialog.h \
	dialogs/qpagesetupdialog.h \
	dialogs/qfiledialog.h

!embedded:mac:SOURCES	+= dialogs/qcolordialog_mac.cpp \
                           dialogs/qfiledialog_mac.cpp
win32:SOURCES += dialogs/qfiledialog_win.cpp
win32:LIBS += shell32.lib 	# the filedialog needs this library

unix:SOURCES += dialogs/qprintdialog.cpp
unix:HEADERS += dialogs/qprintdialog.h 

SOURCES += \
	dialogs/qcolordialog.cpp \
	dialogs/qdialog.cpp \
	dialogs/qerrormessage.cpp \
	dialogs/qfontdialog.cpp \
	dialogs/qmessagebox.cpp \
	dialogs/qprogressdialog.cpp \
	dialogs/qwizard.cpp \
	dialogs/qinputdialog.cpp \
	dialogs/qpagesetupdialog.cpp \
	dialogs/qfiledialog.cpp
