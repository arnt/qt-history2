TEMPLATE	= lib
CONFIG		+= qt warn_on release
WIN32:CONFIG   += dll
WIN32:CONFIG   -= staticlib
sql {
	HEADERS		= sqlformwizardimpl.h
	SOURCES		= main.cpp sqlformwizardimpl.cpp
	INTERFACES	= sqlformwizard.ui
}
DESTDIR		= $(QTDIR)/plugins
PROJECTNAME	= Wizards
INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces