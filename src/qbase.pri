isEmpty(TARGET):error(You must set TARGET before includ()'ing ${FILE})

# Qt project file
QMAKE_INTERNAL_CACHE_FILE = .qmake.internal.cache.$${TARGET}
TEMPLATE	= lib
VERSION		= 4.0.0
win32 {
    #because libnetwork.pro could be qmake'd (qmade?) before libqcore.pro we
    #need to override the version of libq* in all other libq*'s just to be
    #sure the same version is used
    QT_LIBS_OVERRIDE = $$VERSION
    QT_LIBS_OVERRIDE ~= s/\.//g
    for(lib, $$list(qcore qt qnetwork qxml qopengl qsql core qcompat)) {
        eval(QMAKE_$${upper($$lib)}_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
    }
}
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= $$QT_INSTALL_PREFIX/bin

CONFIG		+= qt warn_on depend_includepath
CONFIG          += qmake_cache target_qt

#mac:QMAKE_LFLAGS += -undefined suppress -flat_namespace

win32:!shared:CONFIG += staticlib

win32-borland {
	mng:QMAKE_CFLAGS_WARN_ON	+= -w-par
	mng:QMAKE_CXXFLAGS_WARN_ON	+= -w-par
	# Keep the size of the .tds file for the Qt library smaller than
	# 34 Mbytes to avoid linking problems
	QMAKE_CFLAGS_DEBUG += -vi -y-
	QMAKE_CXXFLAGS_DEBUG += -vi -y-
}

linux-*:version_script {
   QMAKE_LFLAGS += -Wl,--version-script=libqt.map
   TARGETDEPS += libqt.map
}

win32 {
	CONFIG	+= zlib
	INCLUDEPATH += tmp
	!staticlib {
	    DEFINES+=QT_MAKEDLL
	    exists(../qt.rc):RC_FILE = ../qt.rc
	}
}
win32-borland:INCLUDEPATH += kernel

aix-g++* {
	QMAKE_CFLAGS   += -mminimal-toc
	QMAKE_CXXFLAGS += -mminimal-toc
}

embedded {
	EMBEDDED_H	= $$EMBEDDED_CPP
}

DEPENDPATH += ;$$NETWORK_H;$$KERNEL_H;$$WIDGETS_H;$$SQL_H;$$TABLE_H;$$DIALOGS_H;
DEPENDPATH += $$ICONVIEW_H;$$OPENGL_H;$$THREAD_H;$$TOOLS_H;$$CODECS_H;
DEPENDPATH += $$WORKSPACE_H;$$XML_H;$$CANVAS_H;$$STYLES_H;$$COMPAT_H
embedded:DEPENDPATH += ;$$EMBEDDED_H

contains(QT_CONFIG, largefile):unix:!darwin:DEFINES += _LARGEFILE_SOURCE _LARGE_FILES _FILE_OFFSET_BITS=64

!staticlib:PRL_EXPORT_DEFINES += QT_SHARED

#install directives
include(qt_install.pri)

unix {
   CONFIG     += create_libtool create_pc
   QMAKE_PKGCONFIG_LIBDIR = $$target.path
   QMAKE_PKGCONFIG_INCDIR = $$headers.path
}

# ##### for now, until the mess is a bit more cleaned up and we can 
# enable COMPAT warnings.
DEFINES += QT_COMPAT_WARNINGS QT_NO_CAST_TO_ASCII
