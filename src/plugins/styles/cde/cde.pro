TARGET	 = qcdestyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qcdestyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qcdestyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../gui/styles/qmotifstyle.h
	SOURCES += ../../../gui/styles/qmotifstyle.cpp
}

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
