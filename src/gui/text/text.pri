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
	text/qtextlist.h \
	text/qsyntaxhighlighter.h

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
	text/qtextlist.cpp \
	text/qsyntaxhighlighter.cpp


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
        contains(QT_CONFIG,fontconfig) {
		CONFIG += have_freetype
		# pull in the proper freetype2 include directory
 		include($$QT_SOURCE_TREE/config.tests/x11/fontconfig/fontconfig.pri)
	}
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
	contains(QT_CONFIG,ft):CONFIG += have_freetype
}

x11|embedded {
        contains(QT_CONFIG, freetype) {
	    SOURCES += \
		../3rdparty/freetype/builds/unix/ftsystem.c \
		../3rdparty/freetype/src/base/ftbase.c \
		../3rdparty/freetype/src/base/ftbbox.c \
		../3rdparty/freetype/src/base/ftdebug.c \
		../3rdparty/freetype/src/base/ftglyph.c \
		../3rdparty/freetype/src/base/ftinit.c \
		../3rdparty/freetype/src/base/ftmm.c \
                ../3rdparty/freetype/src/base/ftbitmap.c\
		../3rdparty/freetype/src/bdf/bdf.c \
		../3rdparty/freetype/src/cache/ftcache.c \
		../3rdparty/freetype/src/cff/cff.c \
		../3rdparty/freetype/src/cid/type1cid.c \
		../3rdparty/freetype/src/gzip/ftgzip.c \
		../3rdparty/freetype/src/pcf/pcf.c \
		../3rdparty/freetype/src/pfr/pfr.c \
		../3rdparty/freetype/src/psaux/psaux.c \
		../3rdparty/freetype/src/pshinter/pshinter.c \
		../3rdparty/freetype/src/psnames/psmodule.c \
		../3rdparty/freetype/src/raster/raster.c \
		../3rdparty/freetype/src/sfnt/sfnt.c \
		../3rdparty/freetype/src/smooth/smooth.c \
		../3rdparty/freetype/src/truetype/truetype.c \
		../3rdparty/freetype/src/type1/type1.c \
		../3rdparty/freetype/src/type42/type42.c \
		../3rdparty/freetype/src/winfonts/winfnt.c \
		../3rdparty/freetype/src/lzw/ftlzw.c\
                ../3rdparty/freetype/src/otvalid/otvalid.c\
                ../3rdparty/freetype/src/otvalid/otvbase.c\
                ../3rdparty/freetype/src/otvalid/otvgdef.c\
                ../3rdparty/freetype/src/otvalid/otvjstf.c\
                ../3rdparty/freetype/src/otvalid/otvcommn.c\
                ../3rdparty/freetype/src/otvalid/otvgpos.c\
                ../3rdparty/freetype/src/otvalid/otvgsub.c\
                ../3rdparty/freetype/src/otvalid/otvmod.c\
                ../3rdparty/freetype/src/autofit/afangles.c\
                ../3rdparty/freetype/src/autofit/afglobal.c\
                ../3rdparty/freetype/src/autofit/aflatin.c\
                ../3rdparty/freetype/src/autofit/afmodule.c\
                ../3rdparty/freetype/src/autofit/afdummy.c\
                ../3rdparty/freetype/src/autofit/afhints.c\
                ../3rdparty/freetype/src/autofit/afloader.c\
                ../3rdparty/freetype/src/autofit/autofit.c

	    INCLUDEPATH += \
		../3rdparty/freetype/src \
		../3rdparty/freetype/include \
		../3rdparty/freetype/builds/unix

	    DEFINES += FT_CONFIG_OPTION_SYSTEM_ZLIB
        } else {
            # pull in the proper freetype2 include directory
            include($$QT_SOURCE_TREE/config.tests/x11/fontconfig/fontconfig.pri)
        }
}

have_freetype {
	INCLUDEPATH += ../3rdparty/opentype
	HEADERS += text/qopentype_p.h
	SOURCES += \
		../3rdparty/opentype/ftxopentype.c \
		text/qopentype.cpp
	DEFINES += QT_HAVE_FREETYPE
}
