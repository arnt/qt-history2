TARGET = qtscriptxml
include(../../qpluginbase.pri)
QT = core xml script

SOURCES += main.cpp \
	   xmlstreamattribute.cpp \
	   xmlstreamentitydeclaration.cpp \
	   xmlstreamnamespacedeclaration.cpp \
	   xmlstreamnotationdeclaration.cpp \
	   xmlstreamreader.cpp \
	   xmlstreamwriter.cpp

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/script
target.path += $$[QT_INSTALL_PLUGINS]/script
INSTALLS += target
