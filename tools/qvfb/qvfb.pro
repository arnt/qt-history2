TEMPLATE        = app
CONFIG          += qt warn_on uic
TARGET          = qvfb
DESTDIR         = ../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

DEPENDPATH      = ../../include

FORMS           = config.ui
HEADERS         = qvfb.h \
		  qvfbview.h \
		  qvfbratedlg.h \
		  qanimationwriter.h \
                  gammaview.h \
		  skin.h \
                  qvfbprotocol.h \
                  qvfbshmem.h \
                  qvfbmmap.h

SOURCES         = qvfb.cpp \
		  qvfbview.cpp \
		  qvfbratedlg.cpp \
                  main.cpp \
		  qanimationwriter.cpp \
		  skin.cpp \
                  qvfbprotocol.cpp \
                  qvfbshmem.cpp \
                  qvfbmmap.cpp

contains(QT_CONFIG, opengl) {
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
