# Qt kernel module

HEADERS += \
	text/qfont.h \
	text/qfontdatabase.h \
	text/qfontengine_p.h \
	text/qfontinfo.h \
	text/qfontmetrics.h \
	text/qfont_p.h \
	text/qscriptengine_p.h \
	text/qtextengine_p.h \
	text/qtextlayout.h \
	text/qtextformat.h \
	text/qtextformat_p.h \
	text/qtextobject.h \
	text/qtextobject_p.h \
	text/qtextoption.h \
	text/qfragmentmap_p.h \
	text/qtextdocument.h \
	text/qtextdocument_p.h \
	text/qtexthtmlparser_p.h \
	text/qabstracttextdocumentlayout.h \
	text/qtextdocumentlayout_p.h \
	text/qtextcursor.h \
	text/qtextcursor_p.h \
	text/qtextdocumentfragment.h \
	text/qtextdocumentfragment_p.h \
	text/qtextimagehandler_p.h \
	text/qtexttable.h \
	text/qtextlist.h 

SOURCES += \
	text/qfont.cpp \
	text/qfontengine.cpp \
	text/qfontmetrics.cpp \
	text/qfontdatabase.cpp \
	text/qscriptengine.cpp \
	text/qtextengine.cpp \
	text/qtextlayout.cpp \
	text/qtextformat.cpp \
	text/qtextobject.cpp \
	text/qtextoption.cpp \
	text/qfragmentmap.cpp \
	text/qtextdocument.cpp \
	text/qtextdocument_p.cpp \
	text/qtexthtmlparser.cpp \
	text/qabstracttextdocumentlayout.cpp \
	text/qtextdocumentlayout.cpp \
	text/qtextcursor.cpp \
	text/qtextdocumentfragment.cpp \
	text/qtextimagehandler.cpp \
	text/qtexttable.cpp \
	text/qtextlist.cpp


win32 {
	SOURCES += \
		text/qfont_win.cpp \
		text/qfontengine_win.cpp
	HEADERS += text/qfontengine_win_p.h
}

wince-* {
	SOURCES -= text/qfontengine_win.cpp
	SOURCES += text/qfontengine_wce.cpp
}

unix:x11 {
	HEADRES += \
		text/qfontengine_x11_p.h
	SOURCES += \
		text/qfont_x11.cpp \
		text/qfontengine_x11.cpp
        contains(QT_CONFIG,fontconfig):CONFIG += have_freetype
}

!embedded:!x11:mac {
	SOURCES += \
		text/qfont_mac.cpp \
		text/qfontengine_mac.cpp
}

embedded {
	SOURCES += \
		text/qfont_qws.cpp \
		text/qfontengine_qws.cpp
	CONFIG += have_freetype
}

have_freetype {
	INCLUDEPATH += ../3rdparty/opentype
	HEADERS += text/qopentype_p.h
	SOURCES += \
		../3rdparty/opentype/ftxopentype.c \
		text/qopentype.cpp
	DEFINES += QT_HAVE_FREETYPE
}
