# Qt util module

HEADERS += \
        util/qsystemtrayicon.h \
		util/qcompleter.h \
		util/qcompleter_p.h \
                util/qdesktopservices.h
		
SOURCES += \
        util/qsystemtrayicon.cpp \
		util/qcompleter.cpp
		
win32 {
		SOURCES += \
				util/qsystemtrayicon_win.cpp \
				util/qdesktopservices_win.cpp
}

unix:x11 {
		SOURCES += \
				util/qsystemtrayicon_x11.cpp \
				util/qdesktopservices_x11.cpp
}

embedded {
		SOURCES += \
				util/qsystemtrayicon_qws.cpp \
				util/qdesktopservices_qws.cpp
}

!embedded:!x11:mac {
		OBJECTIVE_SOURCES += util/qsystemtrayicon_mac.mm
		SOURCES += \
				util/qdesktopservices_mac.cpp
}
