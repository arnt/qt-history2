TEMPLATE	= app
TARGET		= moc

CONFIG 	       += console release qtinc
CONFIG         -= qt
mac:CONFIG     -= resource_fork
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS QT_NO_DATASTREAM QT_NO_TEXTCODEC \
		  QT_NO_UNICODETABLES QT_NO_THREAD QT_NO_REGEXP
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../bin


INCLUDEPATH	 = ../core/arch/generic $$QT_SOURCE_TREE/include .
DEPENDPATH	+= ../core/arch/generic $$QT_SOURCE_TREE/include . \
		   ../core/base ../core/tools ../core/io
LIBS		=
OBJECTS_DIR	= .


HEADERS = moc.h preprocessor.h scanner.h symbols.h token.h utils.h \
           generator.h outputrevision.h
SOURCES =  moc.cpp \
           preprocessor.cpp \
           main.cpp \
           generator.cpp \
           scanner.cpp

# Qt tools needed to link moc
SOURCES	+= ../core/tools/qbytearray.cpp	\
		  ../core/tools/qbytearraymatcher.cpp \
		  ../core/tools/qdatetime.cpp	\
		  ../core/io/qfile.cpp		\
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
		  ../core/tools/qlocale.cpp

unix:SOURCES += \
	../core/io/qfile_unix.cpp \
	../core/io/qdir_unix.cpp \
	../core/io/qfileinfo_unix.cpp

win32:SOURCES += \
	../core/io/qfile_win.cpp \
	../core/io/qdir_win.cpp \
	../core/io/qfileinfo_win.cpp

macx: {
SOURCES += ../core/kernel/qcore_mac.cpp
LIBS	+= -framework CoreServices
}

target.path=$$bins.path
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

