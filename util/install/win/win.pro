TEMPLATE	= app
CONFIG		+= windows qt

HEADERS		= globalinformation.h \
		  setupwizardimpl.h \
		  environment.h \
		  shell.h \
		  resource.h \
		  dialogs/folderdlgimpl.h \
		  pages/pages.h \
		  pages/sidedecorationimpl.h

SOURCES		= main.cpp \
		  globalinformation.cpp \
		  setupwizardimpl.cpp \
		  setupwizardimpl_config.cpp \
		  environment.cpp \
		  shell.cpp \
		  resource.cpp \
		  dialogs/folderdlgimpl.cpp \
		  pages/pages.cpp \
		  pages/sidedecorationimpl.cpp

INTERFACES	= dialogs/folderdlg.ui \
		  pages/buildpage.ui \
		  pages/configpage.ui \
		  pages/finishpage.ui \
		  pages/folderspage.ui \
		  pages/licenseagreementpage.ui \
		  pages/licensepage.ui \
		  pages/optionspage.ui \
		  pages/progresspage.ui \
		  pages/sidedecoration.ui \
		  pages/winintropage.ui

TARGET		= install
DESTDIR		= ../../../bin
INCLUDEPATH	+= $$QT_SOURCE_TREE/src/3rdparty $$QT_SOURCE_TREE/util/install/archive

win32:RC_FILE	= install.rc

# Comment out one of the following lines to build the installer for:
#
#  - a Qt/Windows evaluation version (eval),
#  - a Qt/Windows evaluation version that can be burned on CD and
#    distributed on tradeshows (eval-cd)
#  - the QSA pre-release (qsa)
#  - educational version (edu)
#
#CONFIG += eval
#CONFIG += eval-cd
#CONFIG += qsa
CONFIG += edu


unix:LIBS		+= -L$$QT_BUILD_TREE/util/install/archive -larq
win32:LIBS		+= ../archive/arq.lib
INCLUDEPATH		+= ../keygen

# We have the following dependencies on config:
#
#  qsa     -> eval-cd and eval
#  eval-cd -> eval
#  eval    -> (none)
#  edu     -> (none)
#
# For the code this means that the following defines are defined:
#
# eval   : EVAL
# eval-cd: EVAL, EVAL_CD
# qsa    : EVAL, EVAL_CD, QSA
# edu    : EDU
#

qsa {
    CONFIG += eval-cd
    DEFINES		+= QSA
}

eval-cd {
    CONFIG += eval
    DEFINES		+= EVAL_CD
}

eval {
    !exists($(QTEVAL)/src) {
	error(You must set the QTEVAL environment variable to the directory where you checked out //depot/qteval/main in order to be able to build the evaluation version of install.)
    }
    DEFINES		+= EVAL
    win32:RC_FILE	= install-eval.rc
    SOURCES		+= $(QTEVAL)/src/check-and-patch.cpp
    INCLUDEPATH		+= $(QTEVAL)/src
}

edu {
    !exists($(QTEVAL)/src) {
	error(You must set the QTEVAL environment variable to the directory where you checked out //depot/qteval/main in order to be able to build the evaluation version of install.)
    }
    DEFINES		+= EDU
    win32:RC_FILE	= install-edu.rc
    SOURCES		+= $(QTEVAL)/src/check-and-patch.cpp
    INCLUDEPATH		+= $(QTEVAL)/src
}
