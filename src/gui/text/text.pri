# Qt kernel module

HEADERS += \
	text/qfont.h \
	text/qfontdatabase.h \
	text/qfontengine_p.h \
	text/qfontinfo.h \
	text/qfontmetrics.h \
	text/qfontdata_p.h \
	text/qscriptengine_p.h \
	text/qtextengine_p.h \
	text/qfontengine_p.h \
	text/qtextlayout_p.h \
	text/qtextformat.h \
	text/qtextformat_p.h

SOURCES += \
	text/qfont.cpp \
	text/qfontdatabase.cpp \
	text/qscriptengine.cpp \
	text/qtextengine.cpp \
	text/qtextlayout.cpp \
	text/qtextformat.cpp


win32 {
	SOURCES += \
		text/qfont_win.cpp \
		text/qfontengine_win.cpp
}

wince-* {
	SOURCES -= text/qfontengine_win.cpp
	SOURCES += text/qfontengine_wce.cpp
}

unix:x11 {
	SOURCES += \
		text/qfont_x11.cpp \
		text/qfontengine_x11.cpp
}

!embedded:!x11:mac {
	SOURCES += \
		text/qfont_mac.cpp \
		text/qfontengine_mac.cpp
}

embedded {
	SOURCES += text/qfontengine_qws.cpp
}

