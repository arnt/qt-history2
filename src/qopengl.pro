REQUIRES = !qt_one_lib
TARGET		= qopengl
include(qbase.pri)
QCONFIG = kernel gui
!win32:!embedded:!mac:CONFIG	   += x11 x11inc

DEFINES += QT_BUILD_OPENGL_LIB

x11:include($$KERNEL_CPP/qt_x11.pri)
mac:include($$KERNEL_CPP/qt_mac.pri)
embedded:include($$KERNEL_CPP/qt_qws.pri)

include($$OPENGL_CPP/qt_opengl.pri)
