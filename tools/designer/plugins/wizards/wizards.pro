TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
sql {
	HEADERS		= sqlformwizardimpl.h
	SOURCES		= main.cpp sqlformwizardimpl.cpp
	INTERFACES	= sqlformwizard.ui mainwindowwizard.ui
}
DESTDIR		= $(QTDIR)/plugins/designer
PROJECTNAME	= Wizards
INCLUDEPATH	+= $(QTDIR)/tools/designer/interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
