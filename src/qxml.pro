REQUIRES = !qt_one_lib
TARGET		= qxml
QCONFIG = kernel gui

DEFINES += QT_BUILD_XML_LIB

include(qbase.pri)
include($$XML_CPP/qt_xml.pri)
