contains(QT_PRODUCT,qt-professional) {
	CONFIG += xml network
	NETWORK_CPP = $(QTDIR)/src/network
	XML_CPP = $(QTDIR)/src/xml
	win32 {
		WIN_ALL_H = $(QTDIR)/include
		XML_H		= $$WIN_ALL_H
		NETWORK_H	= $$WIN_ALL_H
	}
	unix {
		XML_H		= $$XML_CPP
		NETWORK_H	= $$NETWORK_CPP
		#needed for svg
		LIBS            += -lm  
	}
	INCLUDEPATH += $(QTDIR)/src/xml
	INCLUDEPATH += $(QTDIR)/src/network
	include( $(QTDIR)/src/xml/qt_xml.pri )
	include( $(QTDIR)/src/network/qt_network.pri )
	DEFINES     += QT_MODULE_XML QT_MODULE_NETWORK
	DEFINES     += QT_LICENSE_PROFESSIONAL
}
