# Project ID used by some IDEs
GUID 		= {b6834557-bc00-493d-ac84-df68248a5fbc}
TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= dlg2ui.h
SOURCES		= main.cpp dlg2ui.cpp
DESTDIR		= ../../../../plugins/designer
!xml:DEFINES 	+= QT_INTERNAL_XML
include( ../../../../src/qt_professional.pri )
TARGET		= dlgplugin
INCLUDEPATH	+= ../../interfaces
QCONFIG += xml

target.path += $$plugins.path/designer
INSTALLS 	+= target
