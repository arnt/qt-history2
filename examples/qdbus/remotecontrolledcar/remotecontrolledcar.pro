TEMPLATE = subdirs
SUBDIRS = car \
	  controller

# install
sources.files = *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qdbus/remotecontrolledcar
INSTALLS += sources
DEFINES += QT_USE_USING_NAMESPACE
