isEmpty(TARGET):error(You must set TARGET before includ()'ing ${FILE})

# Qt project file
QMAKE_INTERNAL_CACHE_FILE = .qmake.internal.cache.$${TARGET}
TEMPLATE	= lib
VERSION		= 4.0.0
win32 {
    QT_LIBS_OVERRIDE = $$VERSION
    QT_LIBS_OVERRIDE ~= s/\.//g
    for(lib, $$list(qcore qt qnetwork qxml qopengl qsql core qcompat)) {
        eval(QMAKE_$${upper($$lib)}_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
    }
}
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= ../bin

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

CORE_CPP	= core

KERNEL_CPP	= kernel
CANVAS_CPP      = canvas
WIDGETS_CPP	= widgets
SQL_CPP	        = sql
TABLE_CPP	= table
DIALOGS_CPP	= dialogs
ICONVIEW_CPP	= iconview
NETWORK_CPP	= network
OPENGL_CPP	= opengl
THREAD_CPP	= thread
TOOLS_CPP	= tools
CODECS_CPP	= codecs
WORKSPACE_CPP	= workspace
XML_CPP	        = xml
STYLES_CPP	= styles
COMPAT_CPP	= compat
EMBEDDED_CPP	= embedded
ACCESSIBLE_CPP  = accessible

win32 {
	contains(QT_PRODUCT,qt-internal) {
		CORE_H 		= $$CORE_CPP

		SQL_H		= $$SQL_CPP
		KERNEL_H	= $$KERNEL_CPP
		WIDGETS_H	= $$WIDGETS_CPP
		TABLE_H		= $$TABLE_CPP
		DIALOGS_H	= $$DIALOGS_CPP
		ICONVIEW_H	= $$ICONVIEW_CPP
		NETWORK_H	= $$NETWORK_CPP
		OPENGL_H	= $$OPENGL_CPP
		THREAD_H	= $$THREAD_CPP
		TOOLS_H		= $$TOOLS_CPP
		CODECS_H	= $$CODECS_CPP
		WORKSPACE_H	= $$WORKSPACE_CPP
		XML_H		= $$XML_CPP
		CANVAS_H	= $$CANVAS_CPP
		STYLES_H	= $$STYLES_CPP
		ACCESSIBLE_H	= $$ACCESSIBLE_CPP
		COMPAT_H	= $$COMPAT_CPP
	} else {
		WIN_ALL_H = ../include
		CORE_H 		= $$WIN_ALL_H

		SQL_H		= $$WIN_ALL_H
		KERNEL_H	= $$WIN_ALL_H
		WIDGETS_H	= $$WIN_ALL_H
		TABLE_H		= $$WIN_ALL_H
		DIALOGS_H	= $$WIN_ALL_H
		ICONVIEW_H	= $$WIN_ALL_H
		NETWORK_H	= $$WIN_ALL_H
		OPENGL_H	= $$WIN_ALL_H
		THREAD_H	= $$WIN_ALL_H
		TOOLS_H		= $$WIN_ALL_H
		CODECS_H	= $$WIN_ALL_H
		WORKSPACE_H	= $$WIN_ALL_H
		XML_H		= $$WIN_ALL_H
		CANVAS_H	= $$WIN_ALL_H
		STYLES_H	= $$WIN_ALL_H
		ACCESSIBLE_H	= $$WIN_ALL_H
		COMPAT_H	= $$WIN_ALL_H
		CONFIG 		-= incremental
	}

	CONFIG	+= zlib
	INCLUDEPATH += tmp
	!staticlib {
	    DEFINES+=QT_MAKEDLL
	    exists(qt.rc):RC_FILE = qt.rc
	}
} else {
	CORE_H = $$CORE_CPP

    CANVAS_H	= $$CANVAS_CPP
    KERNEL_H	= $$KERNEL_CPP
    WIDGETS_H	= $$WIDGETS_CPP
    SQL_H		= $$SQL_CPP
    TABLE_H		= $$TABLE_CPP
    DIALOGS_H	= $$DIALOGS_CPP
    ICONVIEW_H	= $$ICONVIEW_CPP
    NETWORK_H	= $$NETWORK_CPP
    OPENGL_H	= $$OPENGL_CPP
    THREAD_H	= $$THREAD_CPP
    TOOLS_H		= $$TOOLS_CPP
    CODECS_H	= $$CODECS_CPP
    WORKSPACE_H	= $$WORKSPACE_CPP
    XML_H		= $$XML_CPP
    STYLES_H	= $$STYLES_CPP
    ACCESSIBLE_H	= $$ACCESSIBLE_CPP
    COMPAT_H	= $$COMPAT_CPP
}
win32-borland:INCLUDEPATH += kernel

aix-g++* {
	QMAKE_CFLAGS   += -mminimal-toc
	QMAKE_CXXFLAGS += -mminimal-toc
}

embedded {
	EMBEDDED_H	= $$EMBEDDED_CPP
}
embedded:PREPROCMOC += HEADERS
embedded:PREPROCH =  $$TOOLS_H/qglobal.h

DEPENDPATH += ;$$NETWORK_H;$$KERNEL_H;$$WIDGETS_H;$$SQL_H;$$TABLE_H;$$DIALOGS_H;
DEPENDPATH += $$ICONVIEW_H;$$OPENGL_H;$$THREAD_H;$$TOOLS_H;$$CODECS_H;
DEPENDPATH += $$WORKSPACE_H;$$XML_H;$$CANVAS_H;$$STYLES_H;$$COMPAT_H
embedded:DEPENDPATH += ;$$EMBEDDED_H

largefile:unix:!darwin:DEFINES += _LARGEFILE_SOURCE _LARGE_FILES _FILE_OFFSET_BITS=64

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
DEFINES += QT_COMPAT_WARNINGS
