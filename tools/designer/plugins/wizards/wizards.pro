TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
sql {
	HEADERS		= sqlformwizardimpl.h
	SOURCES		= sqlformwizardimpl.cpp
	}
SOURCES		+= main.cpp
DESTDIR		= ../../../../plugins/designer
INCLUDEPATH	+= ../../interfaces

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
INTERFACES	= sqlformwizard.ui mainwindowwizard.ui 
DBFILE	= wizards.db
IMAGEFILE	= images.cpp
PROJECTNAME	= Wizards
LANGUAGE	= C++
{SOURCES+=images.cpp}
