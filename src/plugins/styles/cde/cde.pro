TEMPLATE = lib
TARGET	 = qcdestyle

CONFIG  += qt warn_off plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qcdestyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qcdestyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../gui/styles/qmotifstyle.h
	SOURCES += ../../../gui/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
