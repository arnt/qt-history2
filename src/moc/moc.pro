TEMPLATE	= app
TARGET		= moc

CONFIG 	       += console qtinc
CONFIG         -= qt
build_all:CONFIG += release
mac:CONFIG     -= resource_fork incremental
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS QT_NO_DATASTREAM QT_NO_TEXTSTREAM \
		  QT_NO_TEXTCODEC QT_NO_UNICODETABLES QT_NO_THREAD \
		  QT_NO_REGEXP QT_NO_QOBJECT
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../bin


INCLUDEPATH	 = ../core/arch/generic $$QT_BUILD_TREE/include . \
                   $$QT_BUILD_TREE/include/QtCore
DEPENDPATH	+= $$INCLUDEPATH ../core/base ../core/tools ../core/io
LIBS	        =
OBJECTS_DIR	= .


HEADERS = moc.h preprocessor.h scanner.h symbols.h token.h utils.h \
           generator.h outputrevision.h qdatetime_p.h
SOURCES =  moc.cpp \
           preprocessor.cpp \
           main.cpp \
           generator.cpp \
           scanner.cpp

# Qt tools needed to link moc
SOURCES	+= ../core/tools/qbytearray.cpp	\
                  ../core/tools/qvsnprintf.cpp \
		  ../core/tools/qbytearraymatcher.cpp \
		  ../core/tools/qdatetime.cpp	\
		  ../core/io/qfile.cpp		\
		  ../core/io/qtemporaryfile.cpp \
                  ../core/io/qfileengine.cpp  \
		  ../core/io/qdir.cpp		\
		  ../core/io/qfileinfo.cpp	\
		  ../core/global/qglobal.cpp		\
		  ../core/io/qiodevice.cpp	\
		  ../core/tools/qlist.cpp		\
		  ../core/tools/qchar.cpp		\
		  ../core/tools/qstring.cpp		\
		  ../core/tools/qstringmatcher.cpp \
                  ../core/tools/qunicodetables.cpp	\
		  ../core/tools/qstringlist.cpp	\
		  ../core/tools/qmap.cpp		\
		  ../core/tools/qvector.cpp          \
		  ../core/tools/qlocale.cpp \
		  ../core/kernel/qinternal.cpp

unix:SOURCES += ../core/io/qfileengine_unix.cpp

win32:SOURCES += ../core/io/qfileengine_win.cpp

macx: {
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
   SOURCES += ../core/kernel/qcore_mac.cpp
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

