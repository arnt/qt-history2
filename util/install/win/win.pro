TEMPLATE = app
HEADERS = setupwizardimpl.h environment.h shell.h folderdlgimpl.h
SOURCES = main.cpp setupwizardimpl.cpp environment.cpp shell.cpp folderdlgimpl.cpp
INTERFACES = setupwizard.ui folderdlg.ui
CONFIG += windows qt md4_keys
TARGET  = install
DESTDIR = ../../../bin
INCLUDEPATH += $(QTDIR)\src\3rdparty $(QTDIR)\util\install\archive

win32-msvc:RC_FILE = install.rc

#CONFIG += do_archive use_rcdata

use_rcdata {
    CONFIG		+= do_archive
    DEFINES		+= USE_RCDATA
    win32-msvc:RC_FILE	= install-rcdata.rc
}

do_archive {
    CONFIG		-= md4_keys
    DEFINES		+= USE_ARCHIVES
    unix:LIBS		+= -L$(QTDIR)/util/install/archive -larq
    win32:LIBS		+= ../archive/arq.lib
    INCLUDEPATH		+= ../keygen
}

md4_keys {
    DEFINES += MD4_KEYS
    HEADERS += ../md4/buf.h \
	    ../md4/checksum.h \
	    ../md4/command.h \
	    ../md4/emit.h \
	    ../md4/fileutil.h \
	    ../md4/job.h \
	    ../md4/netint.h \
	    ../md4/protocol.h \
	    ../md4/prototab.h \
	    ../md4/rsync.h \
	    ../md4/search.h \
	    ../md4/stream.h \
	    ../md4/sumset.h \
	    ../md4/trace.h \
	    ../md4/types.h \
	    ../md4/util.h \
	    ../md4/whole.h \
	    ../md4/config.h \
	    ../md4/qrsync.h

    SOURCES += ../md4/base64.c \
	    ../md4/buf.c \
	    ../md4/checksum.c \
	    ../md4/command.c \
	    ../md4/delta.c \
	    ../md4/emit.c \
	    ../md4/fileutil.c \
	    ../md4/hex.c \
	    ../md4/job.c \
	    ../md4/mdfour.c \
	    ../md4/mksum.c \
	    ../md4/msg.c \
	    ../md4/netint.c \
	    ../md4/patch.c \
	    ../md4/prototab.c \
	    ../md4/readsums.c \
	    ../md4/scoop.c \
	    ../md4/search.c \
	    ../md4/stats.c \
	    ../md4/stream.c \
	    ../md4/sumset.c \
	    ../md4/trace.c \
	    ../md4/tube.c \
	    ../md4/util.c \
	    ../md4/version.c \
	    ../md4/whole.c \
	    ../md4/qrsync.cpp
}
