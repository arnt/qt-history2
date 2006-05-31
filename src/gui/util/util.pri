# Qt util module

HEADERS += \
        util/qsystemtrayicon.h
		
SOURCES += \
        util/qsystemtrayicon.cpp
		
win32 {
		SOURCES += \
				util/qsystemtrayicon_win.cpp
}

unix:x11 {
		SOURCES += \
				util/qsystemtrayicon_x11.cpp
}

embedded {
		SOURCES += \
				util/qsystemtrayicon_qws.cpp
}

!embedded:!x11:mac {
		OBJECTIVE_SOURCES += util/qsystemtrayicon_mac.mm
}
