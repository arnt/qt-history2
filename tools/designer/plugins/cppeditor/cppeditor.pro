TEMPLATE 	= lib
CONFIG		+= qt warn_on plugin

SOURCES		+= cppeditor.cpp syntaxhighliter_cpp.cpp cppcompletion.cpp editorinterfaceimpl.cpp languageinterfaceimpl.cpp common.cpp preferenceinterfaceimpl.cpp yyreg.cpp cppbrowser.cpp projectsettingsinterfaceimpl.cpp sourcetemplateinterfaceimpl.cpp 
HEADERS		+= cppeditor.h syntaxhighliter_cpp.h cppcompletion.h editorinterfaceimpl.h languageinterfaceimpl.h preferenceinterfaceimpl.h yyreg.h cppbrowser.h projectsettingsinterfaceimpl.h sourcetemplateinterfaceimpl.h 
FORMS		= projectsettings.ui mainfilesettings.ui 
		
TARGET		= cppeditor
DESTDIR		= ../../../../plugins/designer
VERSION		= 1.0.0

INCLUDEPATH	+= ../../interfaces ../../editor
win32{
    LIBS	+= $$QT_BUILD_TREE/lib/editor.lib
} else {
    !hpux-acc:LIBS	+= -L$$QT_BUILD_TREE/lib -leditor
    hpux-acc:LIBS += -L$$QT_BUILD_TREE/lib -Wl,-a,default -leditor
}

target.path += $$plugins.path/designer
INSTALLS 	+= target
