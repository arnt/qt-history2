REQUIRES = !qt_one_lib
TARGET		= qnetwork

include(qbase.pri)
QCONFIG = kernel
include($$NETWORK_CPP/qt_network.pri)
