TEMPLATE = app
HEADERS = setupwizardimpl.h environment.h shell.h folderdlgimpl.h
SOURCES = main.cpp setupwizardimpl.cpp environment.cpp shell.cpp folderdlgimpl.cpp
INTERFACES = setupwizard.ui folderdlg.ui confirmdlg.ui environmentdlg.ui
DEFINES += DISTVER=\"3.0.0\"
TARGET  = $(QTDIR)\bin\setup
INCLUDEPATH = $(QTDIR)\src\3rdparty