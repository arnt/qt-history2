TEMPLATE        = app
CONFIG          += qt warn_on uic
TARGET          = qvfb

QT		+= qt3support

DEPENDPATH      = ../../include

FORMS           = config.ui
HEADERS         = qvfb.h \
		  qvfbview.h \
		  qvfbratedlg.h \
		  qanimationwriter.h \
                  gammaview.h \
		  skin.h \
                  qvfbviewiface.h \
                  qvfbprotocol.h \
                  qvfbdisplay.h

SOURCES         = qvfb.cpp \
		  qvfbview.cpp \
		  qvfbratedlg.cpp \
                  main.cpp \
		  qanimationwriter.cpp \
		  skin.cpp \
                  qvfbviewiface.cpp \
                  qvfbprotocol.cpp

contains(QT_CONFIG, opengl) {
	HEADERS += qglvfbview.h
	SOURCES += qglvfbview.cpp 
	QT += opengl
}

contains(QT_CONFIG, system-png) {
	LIBS += -lpng
} else {
	INCLUDEPATH     += $$QT_SOURCE_TREE/src/3rdparty/libpng
}
contains(QT_CONFIG, system-zlib) {
	LIBS += -lz
} else {
	INCLUDEPATH     += $$QT_SOURCE_TREE/src/3rdparty/zlib
}

RESOURCES	+= qvfb.qrc
