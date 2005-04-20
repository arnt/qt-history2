# Qt styles module

HEADERS += \
	styles/qstyle.h \
	styles/qstylefactory.h \
	styles/qstyleoption.h \
	styles/qstyleplugin.h \
	styles/qcommonstylepixmaps_p.h \
	styles/qcommonstyle.h
SOURCES += \
	styles/qstyle.cpp \
	styles/qstylefactory.cpp \
	styles/qstyleoption.cpp \
	styles/qstyleplugin.cpp \
	styles/qcommonstyle.cpp

RESOURCES += styles/qstyle.qrc

contains( styles, all ) {
	styles = mac windows windowsxp
}

x11|embedded|!macx-*:styles -= mac

contains( styles, mac ) {
	HEADERS += \
		styles/qmacstyle_mac.h \
		styles/qmacstylepixmaps_mac_p.h
	SOURCES += \
		styles/qmacstyle_mac.cpp

	!contains( styles, windows ) {
		message( mac requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

contains( styles, cde ) {
	HEADERS += styles/qcdestyle.h
	SOURCES += styles/qcdestyle.cpp

	!contains( styles, motif ) {
		message( cde requires motif )
		styles += motif
		DEFINES+= QT_STYLE_MOTIF
	}
}

contains( styles, motifplus ) {
	HEADERS += styles/qmotifplusstyle.h
	SOURCES += styles/qmotifplusstyle.cpp
	!contains( styles, motif ) {
		message( motifplus requires motif )
		styles += motif
		DEFINES+= QT_STYLE_MOTIF
	}
}

contains( styles, interlace ) {
	HEADERS += styles/qinterlacestyle.h
	SOURCES += styles/qinterlacestyle.cpp
	!contains( styles, windows ) {
		message( interlace requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

contains( styles, platinum ) {
	HEADERS += styles/qplatinumstyle.h
	SOURCES += styles/qplatinumstyle.cpp
	!contains( styles, windows ) {
		message( platinum requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

contains( styles, windowsxp ) {
	HEADERS += styles/qwindowsxpstyle.h
	SOURCES += styles/qwindowsxpstyle.cpp
	!contains( styles, windowsxp ) {
		message( windowsxp requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

contains( styles, sgi ) {
	HEADERS += styles/qsgistyle.h
	SOURCES += styles/qsgistyle.cpp
	!contains( styles, motif ) {
		message( sgi requires motif )
		styles += motif
		DEFINES+= QT_STYLE_MOTIF
	}
}

contains( styles, compact ) {
	HEADERS += styles/qcompactstyle.h
	SOURCES += styles/qcompactstyle.cpp
	!contains( styles, windows ) {
		message( compact requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

wince-*:styles += pocketpc
contains( styles, pocketpc ) {
	HEADERS += styles/qpocketpcstyle_wce.h
	SOURCES += styles/qpocketpcstyle_wce.cpp

	!contains( styles, windows ) {
		message( pocketpc requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}

contains( styles, plastique ) {
	HEADERS += styles/qplastiquestyle.h
	SOURCES += styles/qplastiquestyle.cpp
	!contains( styles, windows ) {
		message( plastique requires windows )
		styles += windows
		DEFINES+= QT_STYLE_WINDOWS
	}
}
				
contains( styles, windows ) {
	HEADERS += styles/qwindowsstyle.h
	SOURCES += styles/qwindowsstyle.cpp
}

contains( styles, motif ) {
	HEADERS += styles/qmotifstyle.h
	SOURCES += styles/qmotifstyle.cpp
}
