isEmpty(TARGET):error(You must set TARGET before includ()'ing ${FILE})
INCLUDEPATH *= $$QMAKE_INCDIR_QT/$$TARGET #just for today to have some compat
TEMPLATE	= lib
VERSION		= 4.0.0

# debug/release combos for our libraries
dll:unix {
  !build_pass {
     DebugPackage.target = debug
     DebugPackage.CONFIG = debug
     ReleasePackage.target = release
     ReleasePackage.CONFIG = release
     BUILDS = DebugPackage ReleasePackage
  } else:DebugPackage {
     OBJECTS_DIR ~= s,release,debug,g
     MOC_DIR ~= s,release,debug,g
     #TARGET = $${TARGET}d
  } else:ReleasePackage {
     OBJECTS_DIR ~= s,debug,release,g
     MOC_DIR ~= s,debug,release,g
  }
}

#exported symbol table (for linux only now)
sam_version_map:dll:linux-g++:!isEmpty(QPRO_PWD) {
   0:exists($(QTDIR)/src/libqt.map) {
       QMAKE_LFLAGS += -Wl,--version-script=$(QTDIR)/src/libqt.map
       TARGETDEPS += $(QTDIR)/src/libqt.map
   } else { 
       TARGET_MAP = lib$${TARGET}.map
       exists($$QPRO_PWD/$$TARGET_MAP)|contains(QT_PRODUCT, qt-internal) {
           QMAKE_LFLAGS += -Wl,--version-script=$${TARGET_MAP}
           TARGETDEPS += $$TARGET_MAP
           contains(QT_PRODUCT, qt-internal) {
               VERSION_MAP.commands = $(QTDIR)/util/scripts/exports.pl -name lib$${TARGET} -o $$TARGET_MAP $$QPRO_PWD $$QPRO_SYMBOLS
               VERSION_MAP.target = $$TARGET_MAP
               QMAKE_EXTRA_TARGETS += VERSION_MAP
               exports.commands = [ -w "$$TARGET_MAP" ] || p4 edit "$$TARGET_MAP"; $$VERSION_MAP.commands
               QMAKE_EXTRA_TARGETS += exports
            }
       }
   }
}

#version overriding 
win32 {
    #because libnetwork.pro could be qmake'd (qmade?) before libqcore.pro we
    #need to override the version of libq* in all other libq*'s just to be
    #sure the same version is used
    QT_LIBS_OVERRIDE = $$VERSION
    QT_LIBS_OVERRIDE ~= s/\.//g
    for(lib, $$list(qtcore qtgui qtnetwork qtxml qtopengl qtsql qt3compat)) {
        eval(QMAKE_$${upper($$lib)}_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
	eval(QMAKE_$${upper($$lib)}D_VERSION_OVERRIDE = $$QT_LIBS_OVERRIDE)
    }
}

#other
DESTDIR		= $$QMAKE_LIBDIR_QT
DLLDESTDIR	= $$QT_INSTALL_PREFIX/bin

CONFIG		+= qt warn_on depend_includepath
CONFIG          += qmake_cache target_qt

mac {
   #QMAKE_LFLAGS += -undefined suppress -flat_namespace
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
   QMAKE_CFLAGS += -fconstant-cfstrings
   QMAKE_CXXFLAGS += -fconstant-cfstrings
}

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
