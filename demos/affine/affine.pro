SOURCES += main.cpp xform.cpp
HEADERS += xform.h

contains(QT_CONFIG, opengl) {
	DEFINES += QT_OPENGL_SUPPORT
	QT += opengl
}

SHARED_FOLDER = ../shared

include($$SHARED_FOLDER/shared.pri)

RESOURCES += affine.qrc

# install
target.path = $$[QT_INSTALL_DEMOS]/affine
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.html *.jpg
sources.path = $$[QT_INSTALL_DEMOS]/affine
INSTALLS += target sources

DEFINES += QT_USE_USING_NAMESPACE
