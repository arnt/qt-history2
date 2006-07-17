TEMPLATE	= app
TARGET		= moc

CONFIG 	       += console qtinc
CONFIG         -= qt
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
mac:CONFIG     -= app_bundle incremental
DEFINES	       += QT_BOOTSTRAPPED QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_LIBRARY \
		  QT_NO_STL QT_NO_COMPRESS QT_NO_DATASTREAM QT_NO_TEXTSTREAM \
		  QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_THREAD \
		  QT_NO_REGEXP QT_NO_QOBJECT QT_NO_SYSTEMLOCALE
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../../bin


INCLUDEPATH	 = ../../corelib/arch/generic $$QT_BUILD_TREE/include . \
                   $$QT_BUILD_TREE/include/QtCore
DEPENDPATH	+= $$INCLUDEPATH ../../corelib/base ../../corelib/tools ../../corelib/io
LIBS	        =
OBJECTS_DIR	= .


HEADERS = moc.h preprocessor.h parser.h symbols.h token.h utils.h \
           generator.h outputrevision.h qdatetime_p.h
SOURCES =  moc.cpp \
           preprocessor.cpp \
           main.cpp \
           generator.cpp \
           parser.cpp \
           token.cpp

# Qt tools needed to link moc
SOURCES	+= ../../corelib/global/qglobal.cpp \
	   ../../corelib/io/qbuffer.cpp \
	   ../../corelib/io/qdir.cpp		\
	   ../../corelib/io/qfile.cpp		\
	   ../../corelib/io/qfileinfo.cpp	\
	   ../../corelib/io/qfsfileengine.cpp	\
	   ../../corelib/io/qiodevice.cpp	\
	   ../../corelib/io/qtemporaryfile.cpp \
	   ../../corelib/io/qtextstream.cpp \
           ../../corelib/kernel/qmetatype.cpp    \
	   ../../corelib/tools/qbytearraymatcher.cpp \
	   ../../corelib/tools/qchar.cpp		\
	   ../../corelib/tools/qdatetime.cpp	\
	   ../../corelib/tools/qhash.cpp		\
	   ../../corelib/tools/qlistdata.cpp		\
	   ../../corelib/tools/qlocale.cpp \
	   ../../corelib/tools/qmap.cpp		\
	   ../../corelib/tools/qstring.cpp		\
	   ../../corelib/tools/qstringlist.cpp	\
	   ../../corelib/tools/qstringmatcher.cpp \
	   ../../corelib/tools/qvector.cpp          \
           ../../corelib/io/qabstractfileengine.cpp  \
           ../../corelib/tools/qbytearray.cpp	\
           ../../corelib/tools/qunicodetables.cpp	\
           ../../corelib/tools/qvsnprintf.cpp 

unix:SOURCES += ../../corelib/io/qfsfileengine_unix.cpp

win32:SOURCES += ../../corelib/io/qfsfileengine_win.cpp

macx: {
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
   SOURCES += ../../corelib/kernel/qcore_mac.cpp
   LIBS += -framework CoreServices
}

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

*-mwerks {
   TEMPLATE = lib
   TARGET = McMoc
   CONFIG -= static
   CONFIG += shared plugin
   DEFINES += MOC_MWERKS_PLUGIN
   MWERKSDIR = $QT_SOURCE_TREE/util/mwerks_plugin
   INCLUDEPATH += $$MWERKSDIR/Headers
   LIBS += $$MWERKSDIR/Libraries/PluginLib4.shlb
   SOURCES += mwerks_mac.cpp
}

include(../../qt_targets.pri)

