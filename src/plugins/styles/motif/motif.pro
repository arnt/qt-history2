TARGET	 = qmotifstyle
include(../../qpluginbase.pri)

DESTDIR	 = $$QT_BUILD_TREE/plugins/styles

HEADERS		= ../../../gui/styles/qmotifstyle.h
SOURCES		= main.cpp \
		  ../../../gui/styles/qmotifstyle.cpp

target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target
