!xml:contains( DEFINES, QT_INTERNAL_XML ) {
	CONFIG += xml
	XML_CPP = $(QTDIR)/src/xml
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		XML_H		= $$WIN_ALL_H
	}
	unix {
		XML_H		= $$XML_CPP
		#needed for svg
		LIBS            += -lm  
	}
	INCLUDEPATH += $(QTDIR)/src/xml
	include( $(QTDIR)/src/xml/qt_xml.pri )
	DEFINES     *= QT_MODULE_XML 
}

!network:contains( DEFINES, QT_INTERNAL_NETWORK) {
	CONFIG += network
	NETWORK_CPP = $(QTDIR)/src/network
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		NETWORK_H	= $$WIN_ALL_H
	}
	unix {
		NETWORK_H	= $$NETWORK_CPP
	}
	INCLUDEPATH += $(QTDIR)/src/network
	include( $(QTDIR)/src/network/qt_network.pri )
	DEFINES     *= QT_MODULE_NETWORK
}

!workspace:contains( DEFINES, QT_INTERNAL_WORKSPACE ) {
	CONFIG += workspace
	WORKSPACE_CPP = $(QTDIR)/src/workspace
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		WORKSPACE_H = $$WIN_ALL_H
	}
	unix {
		WORKSPACE_H = $$WORKSPACE_CPP
	}
	INCLUDEPATH += $(QTDIR)/src/workspace
	include( $(QTDIR)/src/workspace/qt_workspace.pri )
	DEFINES     *= QT_MODULE_WORKSPACE
}

!iconview:contains( DEFINES, QT_INTERNAL_ICONVIEW ) {
	CONFIG += iconview
	ICONVIEW_CPP = $(QTDIR)/src/iconview
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		ICONVIEW_H = $$WIN_ALL_H
	}
	unix {
		ICONVIEW_H = $$ICONVIEW_CPP
	}
	INCLUDEPATH += $(QTDIR)/src/iconview
	include( $(QTDIR)/src/iconview/qt_iconview.pri )
	DEFINES     *= QT_MODULE_ICONVIEW
}

!canvas:contains( DEFINES, QT_INTERNAL_CANVAS ) {
	CONFIG += canvas
	CANVAS_CPP = $(QTDIR)/src/canvas
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		CANVAS_H = $$WIN_ALL_H
	}
	unix {
		CANVAS_H = $$CANVAS_CPP
	}
	INCLUDEPATH += $(QTDIR)/src/canvas
	include( $(QTDIR)/src/canvas/qt_canvas.pri )
	DEFINES     *= QT_MODULE_CANVAS
}

contains(QT_PRODUCT,qt-professional) {
	DEFINES     *= QT_LICENSE_PROFESSIONAL
}
