professional {
	CONFIG += xml iconview
	ICONVIEW_CPP	= $(QTDIR)/src/iconview
	XML_CPP	        = $(QTDIR)/src/xml
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		ICONVIEW_H	= $$WIN_ALL_H
		XML_H		= $$WIN_ALL_H
	}

	unix {
		ICONVIEW_H	= $$ICONVIEW_CPP
		XML_H		= $$XML_CPP
	}

	include( $(QTDIR)/qt_xml.pri )
	include( $(QTDIR)/src/iconview/qt_iconview.pri )
	INCLUDEPATH += $(QTDIR)/src/kernel
	DEFINES     += QT_MODULE_XML
}
