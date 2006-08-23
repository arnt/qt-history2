TEMPLATE      = subdirs
unset(EXAMPLES_LAYOUTS_SUBDIRS)
EXAMPLES_LAYOUTS_SUBDIRS = examples_layouts_basiclayouts \
                           examples_layouts_borderlayout \
                           examples_layouts_dynamiclayouts \
                           examples_layouts_flowlayout

# install
EXAMPLES_LAYOUTS_install_sources.files = README *.pro
EXAMPLES_LAYOUTS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/layouts
INSTALLS += EXAMPLES_LAYOUTS_install_sources

#subdirs
examples_layouts_basiclayouts.subdir = $$QT_BUILD_TREE/examples/layouts/basiclayouts
examples_layouts_basiclayouts.depends =  src_corelib src_gui
examples_layouts_borderlayout.subdir = $$QT_BUILD_TREE/examples/layouts/borderlayout
examples_layouts_borderlayout.depends =  src_corelib src_gui
examples_layouts_dynamiclayouts.subdir = $$QT_BUILD_TREE/examples/layouts/dynamiclayouts
examples_layouts_dynamiclayouts.depends =  src_corelib src_gui
examples_layouts_flowlayout.subdir = $$QT_BUILD_TREE/examples/layouts/flowlayout
examples_layouts_flowlayout.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_LAYOUTS_SUBDIRS
SUBDIRS += $$EXAMPLES_LAYOUTS_SUBDIRS
