TEMPLATE	= app
CONFIG = console release qtinc yacc lex_included yacc_no_name_mangle
DEFINES	       += QT_MOC QT_NO_CODECS QT_NO_TEXTCODEC \
		  QT_LITE_UNICODE QT_NO_COMPONENT \
		  QT_NO_STL QT_NO_COMPRESS QT_NO_DATASTREAM
win32:DEFINES  += QT_NODLL
INCLUDEPATH	+= $$QT_SOURCE_TREE/include ../tools .
DEPENDPATH	+= $$QT_SOURCE_TREE/include ../tools .
LIBS		=
DESTDIR         = ../../bin
OBJECTS_DIR	= .
SOURCES		= ../compat/qptrcollection.cpp  \
		  ../tools/qbitarray.cpp	    \
		  ../tools/qbytearray.cpp	    \
		  ../tools/qdatetime.cpp    \
		  ../tools/qfile.cpp	    \
		  ../tools/qdir.cpp	    \
		  ../tools/qfileinfo.cpp    \
		  ../compat/qgdict.cpp	    \
		  ../compat/qglist.cpp	    \
		  ../compat/qgvector.cpp	    \
		  ../tools/qglobal.cpp	    \
		  ../tools/qiodevice.cpp    \
		  ../tools/qlist.cpp	    \
		  ../tools/qregexp.cpp	    \
		  ../tools/qchar.cpp	    \
		  ../tools/qstring.cpp	    \
                  ../tools/qunicodetables.cpp \
		  ../tools/qstringlist.cpp  \
		  ../tools/qmap.cpp        \ 
		  ../tools/qvector.cpp     

isEmpty(QT_PRODUCT)|contains(QT_PRODUCT, qt-internal) {
    LEXSOURCES  = moc.l
    YACCSOURCES = moc.y
} else {
    SOURCES   += moc_yacc.cpp
}

unix:SOURCES	+= ../tools/qfile_unix.cpp ../tools/qdir_unix.cpp ../tools/qfileinfo_unix.cpp
win32:SOURCES	+= ../tools/qfile_win.cpp ../tools/qdir_win.cpp ../tools/qfileinfo_win.cpp
macx:LIBS	+= -framework Carbon

TARGET		= moc

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
