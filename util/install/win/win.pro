TEMPLATE	= app
HEADERS		= setupwizardimpl.h \
		  environment.h \
		  shell.h \
		  folderdlgimpl.h
SOURCES		= main.cpp \
		  setupwizardimpl.cpp \
		  setupwizardimpl_config.cpp \
		  environment.cpp \
		  shell.cpp \
		  folderdlgimpl.cpp
INTERFACES	= setupwizard.ui \
		  folderdlg.ui
CONFIG		+= windows qt
TARGET		= install
DESTDIR		= ../../../bin
INCLUDEPATH	+= $(QTDIR)/src/3rdparty $(QTDIR)/util/install/archive

win32:RC_FILE	= install.rc

#CONFIG += eval

unix:LIBS		+= -L$(QTDIR)/util/install/archive -larq
win32:LIBS		+= ../archive/arq.lib
INCLUDEPATH		+= ../keygen

eval {
    !exists($(QTEVAL)/install) {
	error(You must set the QTEVAL environment variable to the directory where you checked out //depot/qteval/main in order to be able to build the evaluation version of install.)
    }
    DEFINES		+= EVAL
    win32:RC_FILE	= install-eval.rc
    SOURCES		+= $(QTEVAL)/install/check-and-patch.cpp
    INCLUDEPATH		+= $(QTEVAL)/install
}
