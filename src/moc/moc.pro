TEMPLATE	= app
TARGET		= moc

CONFIG 		= console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS QT_NO_DATASTREAM QT_NO_TEXTCODEC \
		  QT_NO_UNICODETABLES
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../bin


INCLUDEPATH	= $$QT_SOURCE_TREE/include ../tools . $$QT_SOURCE_TREE/arch/$$ARCH
DEPENDPATH	+= $$QT_SOURCE_TREE/include ../core/base ../core/tools ../core/io .
LIBS		=
OBJECTS_DIR	= .
SOURCES		= ../core/tools/qbitarray.cpp	\
		  ../core/tools/qbytearray.cpp	\
		  ../core/tools/qhash.cpp	\
		  ../core/tools/qdatetime.cpp	\
		  ../core/io/qfile.cpp		\
		  ../core/io/qdir.cpp		\
		  ../core/io/qfileinfo.cpp	\
		  ../core/base/qglobal.cpp		\
		  ../core/io/qiodevice.cpp	\
		  ../core/tools/qlist.cpp		\
		  ../core/tools/qregexp.cpp		\
		  ../core/tools/qchar.cpp		\
		  ../core/tools/qstring.cpp		\
                  ../core/tools/qunicodetables.cpp	\
		  ../core/tools/qstringlist.cpp	\
		  ../core/tools/qmap.cpp		\ 
		  ../core/tools/qvector.cpp          \
		  ../core/tools/qlocale.cpp

include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)

isEmpty(QT_PRODUCT)|contains(QT_PRODUCT, qt-internal) {
    LEXSOURCES  = moc.l
    YACCSOURCES = moc.y
} else {
    SOURCES   += moc_yacc.cpp
}

unix:SOURCES += \
	../core/io/qfile_unix.cpp \
	../core/io/qdir_unix.cpp \
	../core/io/qfileinfo_unix.cpp

win32:SOURCES += \
	../core/io/qfile_win.cpp \
	../core/io/qdir_win.cpp \
	../core/io/qfileinfo_win.cpp

macx:LIBS	+= -framework CoreServices

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

# ##### for now, until the mess is a bit more cleaned up and we can 
# enable COMPAT warnings.
DEFINES += QT_COMPAT
