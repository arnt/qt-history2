REQUIRES = !qt_one_lib
TARGET		= qnetwork

DEFINES += QT_BUILD_NETWORK_LIB

include(qbase.pri)
QCONFIG = core
include($$NETWORK_CPP/qt_network.pri)
