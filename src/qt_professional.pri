!xml:contains( DEFINES, QT_INTERNAL_XML ) {
	CONFIG += xml
	XML_CPP = $$QT_SOURCE_TREE/src/xml
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		XML_H		= $$WIN_ALL_H
	}
	unix {
		XML_H		= $$XML_CPP
		#needed for svg
		LIBS            += -lm  
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/xml
	include( $$QT_SOURCE_TREE/src/xml/qt_xml.pri )
	DEFINES     *= QT_MODULE_XML 
}

!network:contains( DEFINES, QT_INTERNAL_NETWORK) {
	CONFIG += network
	NETWORK_CPP = $$QT_SOURCE_TREE/src/network
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		NETWORK_H	= $$WIN_ALL_H
	}
	unix {
		NETWORK_H	= $$NETWORK_CPP
	}
	mac:INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/dlcompat
	INCLUDEPATH += $$QT_SOURCE_TREE/src/network
	include( $$QT_SOURCE_TREE/src/network/qt_network.pri )
	DEFINES     *= QT_MODULE_NETWORK
}

!iconview:contains( DEFINES, QT_INTERNAL_ICONVIEW ) {
	CONFIG += iconview
	ICONVIEW_CPP = $$QT_SOURCE_TREE/src/iconview
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		ICONVIEW_H = $$WIN_ALL_H
	}
	unix {
		ICONVIEW_H = $$ICONVIEW_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/iconview
	include( $$QT_SOURCE_TREE/src/iconview/qt_iconview.pri )
	DEFINES     *= QT_MODULE_ICONVIEW
}

!canvas:contains( DEFINES, QT_INTERNAL_CANVAS ) {
	CONFIG += canvas
	CANVAS_CPP = $$QT_SOURCE_TREE/src/canvas
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		CANVAS_H = $$WIN_ALL_H
	}
	unix {
		CANVAS_H = $$CANVAS_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/canvas
	include( $$QT_SOURCE_TREE/src/canvas/qt_canvas.pri )
	DEFINES     *= QT_MODULE_CANVAS
}

contains(QT_PRODUCT,qt-professional) {
	DEFINES     *= QT_LICENSE_PROFESSIONAL
}
