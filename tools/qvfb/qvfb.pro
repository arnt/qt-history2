TEMPLATE        = app
CONFIG          += qt warn_on debug uic
HEADERS         = qvfb.h qvfbview.h qvfbratedlg.h qanimationwriter.h \
                  gammaview.h skin.h config.h
SOURCES         = qvfb.cpp qvfbview.cpp qvfbratedlg.cpp \
                  main.cpp qanimationwriter.cpp skin.cpp config.cpp
FORMS           = config.ui
IMAGES          = images/logo.png
TARGET          = qvfb
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
DEPENDPATH      = ../../include
DEFINES         += QT_COMPAT
