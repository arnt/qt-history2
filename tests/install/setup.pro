TEMPLATE = app
HEADERS = setupwizardimpl.h environment.h shell.h
SOURCES = main.cpp setupwizardimpl.cpp environment.cpp shell.cpp
INTERFACES = setupwizard.ui folderdlg.ui
TARGET  = setup
INCLUDEPATH = $(QTDIR)\src\3rdparty