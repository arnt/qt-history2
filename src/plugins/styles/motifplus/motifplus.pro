TARGET	 = qmotifplusstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qmotifplusstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qmotifplusstyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../gui/styles/qmotifstyle.h
	SOURCES += ../../../gui/styles/qmotifstyle.cpp
}

target.path += $$plugins.path/styles
INSTALLS += target
