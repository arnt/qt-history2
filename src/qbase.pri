isEmpty(TARGET):error(You must set TARGET before include()'ing $${_FILE_})
INCLUDEPATH *= $$QMAKE_INCDIR_QT/$$TARGET #just for today to have some compat
TEMPLATE	= lib
VERSION		= 4.0.0

#exported symbol table (for linux only now)
sam_version_map:shared {
   macx-g++ {
       !isEmpty(QPRO_PWD) {
           TARGET_MAP = lib$${TARGET}.symbols
           exists($$QPRO_PWD/$$TARGET_MAP)|contains(QT_PRODUCT, qt-internal) {
	       TARGET_MAP_IN = $${TARGET_MAP}.in
               #QMAKE_LFLAGS += -exported_symbols_list $$TARGET_MAP
               TARGETDEPS += $$TARGET_MAP_IN
               contains(QT_PRODUCT, qt-internal) {
                   VERSION_MAP_in.target = $$TARGET_MAP_IN
                   VERSION_MAP_in.commands = $(QTDIR)/util/scripts/exports.pl -format symbol_list -o $$TARGET_MAP_IN $$QPRO_PWD $$QPRO_SYMBOLS
                   QMAKE_EXTRA_TARGETS += VERSION_MAP_in
		   VERSION_MAP = $(QTDIR)/util/scripts/globalsyms.pl -o "$$TARGET_MAP" $$TARGET_MAP_IN $(DESTDIR)$(TARGET)
		   QMAKE_POST_LINK += $$quote($$VERSION_MAP\n)
		   exports.depends = $$TARGET_MAP_IN
                   exports.commands = [ -w "$$TARGET_MAP" ] || p4 edit "$$TARGET_MAP"; $$VERSION_MAP
                   QMAKE_EXTRA_TARGETS += exports
               }
           }
       }
   } else:linux-g++ {
       0:exists($(QTDIR)/src/libqt.map) {
         QMAKE_LFLAGS += -Wl,--version-script=$(QTDIR)/src/libqt.map
         TARGETDEPS += $(QTDIR)/src/libqt.map
       } else:!isEmpty(QPRO_PWD) {
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
}
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

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
DLLDESTDIR	= $$[QT_INSTALL_PREFIX]/bin

CONFIG		+= qt warn_on depend_includepath
CONFIG          += qmake_cache target_qt 
CONFIG          -= fix_output_dirs
!macx-xcode:CONFIG += debug_and_release
win32-msvc {
    equals(TEMPLATE_PREFIX, "vc"):CONFIG -= debug_and_release
    equals(TEMPLATE, "vcapp"):CONFIG -= debug_and_release
    equals(TEMPLATE, "vclib"):CONFIG -= debug_and_release
}
contains(QT_CONFIG, largefile):CONFIG += largefile

mac {
   CONFIG += explicitlib
   QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.2 #enables weak linking for 10.2 (exported)
   !macx-xlc: {
       QMAKE_CFLAGS += -fconstant-cfstrings
       QMAKE_CXXFLAGS += -fconstant-cfstrings
   }
}

win32:!shared:CONFIG += static

win32-borland {
    mng:QMAKE_CFLAGS_WARN_ON	+= -w-par
    mng:QMAKE_CXXFLAGS_WARN_ON	+= -w-par
    # Keep the size of the .tds file for the Qt library smaller than
    # 34 Mbytes to avoid linking problems
    QMAKE_CFLAGS_DEBUG += -vi -y-
    QMAKE_CXXFLAGS_DEBUG += -vi -y-
}

win32 {
    CONFIG += zlib
    INCLUDEPATH += tmp
    !static {
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
     EMBEDDED_H = $$EMBEDDED_CPP
}

DEPENDPATH += ;$$NETWORK_H;$$KERNEL_H;$$WIDGETS_H;$$SQL_H;$$TABLE_H;$$DIALOGS_H;
DEPENDPATH += $$ICONVIEW_H;$$OPENGL_H;$$THREAD_H;$$TOOLS_H;$$CODECS_H;
DEPENDPATH += $$WORKSPACE_H;$$XML_H;$$STYLES_H;$$COMPAT_H
embedded:DEPENDPATH += ;$$EMBEDDED_H

!static:PRL_EXPORT_DEFINES += QT_SHARED

#install directives
include(qt_install.pri)

unix {
   CONFIG     += create_libtool create_pc
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]
}

DEFINES += QT_NO_CAST_TO_ASCII
contains(QT_CONFIG, compat):DEFINES *= QT_COMPAT_WARNINGS #enable warnings

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}
