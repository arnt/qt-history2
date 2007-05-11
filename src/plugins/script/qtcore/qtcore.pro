TARGET = qtscriptcore
include(../../qpluginbase.pri)
QT = core script

SOURCES += main.cpp \
	   bytearray.cpp \
	   dir.cpp \
	   event.cpp \
	   file.cpp \
	   fileinfo.cpp \
	   iodevice.cpp \
	   modelindex.cpp \
	   process.cpp \
	   textstream.cpp \
	   timeline.cpp \
	   timer.cpp \
	   string.cpp

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/script
target.path += $$[QT_INSTALL_PLUGINS]/script
INSTALLS += target
