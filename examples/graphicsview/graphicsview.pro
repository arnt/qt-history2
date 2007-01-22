TEMPLATE      = subdirs
SUBDIRS       = elasticnodes collidingmice diagramscene dragdroprobot
contains(QT_CONFIG, qt3support):SUBDIRS += portedcanvas portedasteroids

# install
target.path = $$[QT_INSTALL_EXAMPLES]/graphicsview
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS graphicsview.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/graphicsview
INSTALLS += target sources
