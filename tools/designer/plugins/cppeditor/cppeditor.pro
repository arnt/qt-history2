TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= cppeditor.h  \
		  syntaxhighliter_cpp.h \
		  indent_cpp.h \
		  cppcompletion.h \
		  editorinterfaceimpl.h \
		  languageinterfaceimpl.h \
		  preferenceinterfaceimpl.h \
		  yyreg.h

SOURCES		= cppeditor.cpp \
		  syntaxhighliter_cpp.cpp \
		  indent_cpp.cpp \
		  cppcompletion.cpp \
		  editorinterfaceimpl.cpp \
		  languageinterfaceimpl.cpp \
		  common.cpp \
		  preferenceinterfaceimpl.cpp \
		  yyreg.cpp
		
TARGET		= cppeditor
DESTDIR		= $(QTDIR)/plugins/designer
VERSION		= 1.0.0
unix:LIBS	+= -leditor
win32:LIBS	+= $(QTDIR)/lib/editor.lib
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces $(QTDIR)/tools/designer/editor

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins
INSTALLS += target
