# Qt styles module

styles {
	STYLES_P = styles
	HEADERS +=$$STYLES_H/qstylefactory.h \
		  $$STYLES_P/qstyleinterface_p.h \
		  $$STYLES_H/qstyleplugin.h \
		  $$STYLES_H/qcommonstyle.h
	SOURCES +=$$STYLES_CPP/qstylefactory.cpp \
		  $$STYLES_CPP/qstyleplugin.cpp \
		  $$STYLES_CPP/qcommonstyle.cpp

	contains( styles, all ) {
		styles += aqua mac cde motifplus sgi platinum compact interlace windows motif
	}

	!macx-*:styles -= aqua mac
	contains( styles, mac ) {
		HEADERS +=$$STYLES_H/qmacstyle_mac.h
		SOURCES +=$$STYLES_CPP/qmacstyle_mac.cpp

		!contains( styles, aqua ) {
			message( mac requires aqua )
			styles += aqua
		}
	}
	!contains( styles, mac ):DEFINES += QT_NO_STYLE_MAC

	contains( styles, aqua ) {
		HEADERS +=$$STYLES_H/qaquastyle.h
		SOURCES +=$$STYLES_CPP/qaquastyle.cpp

		!contains( styles, windows ) {
			message( aqua requires windows )
			styles += windows
		}
	}
	!contains( styles, aqua):DEFINES += QT_NO_STYLE_AQUA

	contains( styles, cde ) {
		HEADERS +=$$STYLES_H/qcdestyle.h
		SOURCES +=$$STYLES_CPP/qcdestyle.cpp

		!contains( styles, motif ) {
			message( cde requires motif )
			styles += motif
		}
	}
	!contains( styles, cde ):DEFINES += QT_NO_STYLE_CDE

	contains( styles, motifplus ) {
		HEADERS +=$$STYLES_H/qmotifplusstyle.h
		SOURCES +=$$STYLES_CPP/qmotifplusstyle.cpp
		!contains( styles, motif ) {
			message( motifplus requires motif )
			styles += motif
		}
	}
	!contains( styles, motifplus ):DEFINES += QT_NO_STYLE_MOTIFPLUS

	contains( styles, interlace ) {
		HEADERS +=$$STYLES_H/qinterlacestyle.h
		SOURCES +=$$STYLES_CPP/qinterlacestyle.cpp
		!contains( styles, windows ) {
			message( interlace requires windows )
			styles += windows
		}
	}
	!contains( styles, interlace ):DEFINES += QT_NO_STYLE_INTERLACE

	contains( styles, platinum ) {
		HEADERS +=$$STYLES_H/qplatinumstyle.h
		SOURCES +=$$STYLES_CPP/qplatinumstyle.cpp
		!contains( styles, windows ) {
			message( platinum requires windows )
			styles += windows
		}
	}
	!contains( styles, platinum ):DEFINES += QT_NO_STYLE_PLATINUM

	contains( styles, sgi ) {
		HEADERS +=$$STYLES_H/qsgistyle.h
		SOURCES +=$$STYLES_CPP/qsgistyle.cpp
		!contains( styles, motif ) {
			message( sgi requires motif )
			styles += motif
		}
	}
	!contains( styles, sgi ):DEFINES += QT_NO_STYLE_SGI

	contains( styles, compact ) {
		HEADERS +=$$STYLES_H/qcompactstyle.h
		SOURCES +=$$STYLES_CPP/qcompactstyle.cpp
		!contains( styles, windows ) {
			message( compact requires windows )
			styles += windows
		}
	}
	!contains( styles, compact ):DEFINES += QT_NO_STYLE_COMPACT

	contains( styles, windows ) {
		HEADERS +=$$STYLES_H/qwindowsstyle.h
		SOURCES +=$$STYLES_CPP/qwindowsstyle.cpp
	}
	!contains( styles, windows ):DEFINES += QT_NO_STYLE_WINDOWS

	contains( styles, motif ) {
		HEADERS +=$$STYLES_H/qmotifstyle.h
		SOURCES +=$$STYLES_CPP/qmotifstyle.cpp
	}
	!contains( styles, motif ):DEFINES += QT_NO_STYLE_MOTIF
}
