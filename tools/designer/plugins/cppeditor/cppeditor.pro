TEMPLATE 	= lib
CONFIG		+= qt warn_on release plugin

SOURCES		+= cppeditor.cpp syntaxhighliter_cpp.cpp cppcompletion.cpp editorinterfaceimpl.cpp languageinterfaceimpl.cpp common.cpp preferenceinterfaceimpl.cpp yyreg.cpp cppbrowser.cpp projectsettingsinterfaceimpl.cpp sourcetemplateinterfaceimpl.cpp 
HEADERS		+= cppeditor.h syntaxhighliter_cpp.h cppcompletion.h editorinterfaceimpl.h languageinterfaceimpl.h preferenceinterfaceimpl.h yyreg.h cppbrowser.h projectsettingsinterfaceimpl.h sourcetemplateinterfaceimpl.h 
FORMS		= projectsettings.ui mainfilesettings.ui 
		
TARGET		= cppeditor
DESTDIR		= ../../../../plugins/designer
VERSION		= 1.0.0

INCLUDEPATH	+= ../../../../src/kernel ../../interfaces ../../editor
win32:LIBS	+= $(QTDIR)/lib/editor100.lib
unix:LIBS	+= -leditor
DBFILE		= cppeditor.db
LANGUAGE	= C++

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/designer
INSTALLS 	+= target
