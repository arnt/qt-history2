TEMPLATE = app
HEADERS = setupwizardimpl.h environment.h shell.h folderdlgimpl.h
SOURCES = main.cpp setupwizardimpl.cpp environment.cpp shell.cpp folderdlgimpl.cpp
INTERFACES = setupwizard.ui folderdlg.ui
CONFIG += windows qt
TARGET  = install
DESTDIR = ../../bin
INCLUDEPATH = $(QTDIR)\src\3rdparty

win32-msvc:RC_FILE = install.rc

do_archive {
   DEFINES += USE_ARCHIVES
   unix:LIBS += -L$(QTDIR)/util/install/archive -larq
}
