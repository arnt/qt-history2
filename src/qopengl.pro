REQUIRES = !qt_one_lib
TARGET		= qopengl
include(qbase.pri)
QCONFIG = kernel gui
include($$OPENGL_CPP/qt_opengl.pri)
