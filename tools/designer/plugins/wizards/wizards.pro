TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
sql {
	HEADERS		= sqlformwizardimpl.h
	SOURCES		= main.cpp sqlformwizardimpl.cpp
	INTERFACES	= sqlformwizard.ui mainwindowwizard.ui
}
DESTDIR		= $(QTDIR)/plugins
PROJECTNAME	= Wizards
INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces