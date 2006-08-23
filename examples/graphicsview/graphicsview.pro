TEMPLATE      = subdirs
unset(EXAMPLES_GRAPHICSVIEW_SUBDIRS)
EXAMPLES_GRAPHICSVIEW_SUBDIRS = examples_graphicsview_elasticnodes \
                                examples_graphicsview_collidingmice \
                                examples_graphicsview_dragdroprobot
contains(QT_CONFIG, qt3support):EXAMPLES_GRAPHICSVIEW_SUBDIRS += examples_graphicsview_portedcanvas \
                                                                 examples_graphicsview_portedasteroids

# install
target.path = $$[QT_INSTALL_EXAMPLES]/graphicsview
EXAMPLES_GRAPHICSVIEW_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS graphicsview.pro README
EXAMPLES_GRAPHICSVIEW_install_sources.path = $$[QT_INSTALL_EXAMPLES]/graphicsview
INSTALLS += target EXAMPLES_GRAPHICSVIEW_install_sources

#subdirs
examples_graphicsview_elasticnodes.subdir = $$QT_BUILD_TREE/examples/graphicsview/elasticnodes
examples_graphicsview_elasticnodes.depends =  src_corelib src_gui
examples_graphicsview_collidingmice.subdir = $$QT_BUILD_TREE/examples/graphicsview/collidingmice
examples_graphicsview_collidingmice.depends =  src_corelib src_gui
examples_graphicsview_dragdroprobot.subdir = $$QT_BUILD_TREE/examples/graphicsview/dragdroprobot
examples_graphicsview_dragdroprobot.depends =  src_corelib src_gui
examples_graphicsview_portedcanvas.subdir = $$QT_BUILD_TREE/examples/graphicsview/portedcanvas
examples_graphicsview_portedcanvas.depends =  src_corelib src_gui src_qt3support
examples_graphicsview_portedasteroids.subdir = $$QT_BUILD_TREE/examples/graphicsview/portedasteroids
examples_graphicsview_portedasteroids.depends =  src_corelib src_gui src_qt3support
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_GRAPHICSVIEW_SUBDIRS
SUBDIRS += $$EXAMPLES_GRAPHICSVIEW_SUBDIRS
