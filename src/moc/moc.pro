GUID 		= {0870e913-4812-4534-8c58-70a655348852}
TEMPLATE	= app
TARGET		= moc

CONFIG 		= console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC QT_NO_CODECS QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS
win32:DEFINES  += QT_NODLL
DESTDIR         = ../../bin


INCLUDEPATH	= $$QT_SOURCE_TREE/include ../tools . $$QT_SOURCE_TREE/arch/$$ARCH
DEPENDPATH	+= $$QT_SOURCE_TREE/include ../tools .
LIBS		=
OBJECTS_DIR	= .
SOURCES		= ../compat/qptrcollection.cpp	\
		  ../tools/qbitarray.cpp	\
		  ../tools/qbytearray.cpp	\
		  ../tools/qdatetime.cpp	\
		  ../tools/qfile.cpp		\
		  ../tools/qdir.cpp		\
		  ../tools/qfileinfo.cpp	\
		  ../compat/qgdict.cpp		\
		  ../compat/qglist.cpp		\
		  ../compat/qgvector.cpp	\
		  ../tools/qglobal.cpp		\
		  ../tools/qhash.cpp		\
		  ../tools/qiodevice.cpp	\
		  ../tools/qlist.cpp		\
		  ../tools/qregexp.cpp		\
		  ../tools/qchar.cpp		\
		  ../tools/qstring.cpp		\
                  ../tools/qunicodetables.cpp	\
		  ../tools/qstringlist.cpp	\
		  ../tools/qmap.cpp		\ 
		  ../tools/qvector.cpp          \
		  ../tools/qlocale.cpp

include($$QT_SOURCE_TREE/arch/$$ARCH/arch.pri)

isEmpty(QT_PRODUCT)|contains(QT_PRODUCT, qt-internal) {
    LEXSOURCES  = moc.l
    YACCSOURCES = moc.y
} else {
    SOURCES   += moc_yacc.cpp
}

unix:SOURCES	+= ../tools/qfile_unix.cpp ../tools/qdir_unix.cpp ../tools/qfileinfo_unix.cpp
win32:SOURCES	+= ../tools/qfile_win.cpp ../tools/qdir_win.cpp ../tools/qfileinfo_win.cpp
macx:LIBS	+= -framework Carbon

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
