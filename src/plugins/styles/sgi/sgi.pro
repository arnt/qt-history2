TARGET	 = qsgistyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qsgistyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qsgistyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../gui/styles/qmotifstyle.h
	SOURCES += ../../../gui/styles/qmotifstyle.cpp
}

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
