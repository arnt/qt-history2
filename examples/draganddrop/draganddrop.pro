TEMPLATE    = subdirs
unset(EXAMPLES_DRAGANDDROP_SUBDIRS)
EXAMPLES_DRAGANDDROP_SUBDIRS = examples_draganddrop_draggableicons \
                               examples_draganddrop_draggabletext \
                               examples_draganddrop_dropsite \
                               examples_draganddrop_fridgemagnets \
                               examples_draganddrop_puzzle

# install
EXAMPLES_DRAGANDDROP_install_sources.files = README *.pro
EXAMPLES_DRAGANDDROP_install_sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop
INSTALLS += EXAMPLES_DRAGANDDROP_install_sources

#subdirs
examples_draganddrop_draggableicons.subdir = $$QT_BUILD_TREE/examples/draganddrop/draggableicons
examples_draganddrop_draggableicons.depends =  src_corelib src_gui
examples_draganddrop_draggabletext.subdir = $$QT_BUILD_TREE/examples/draganddrop/draggabletext
examples_draganddrop_draggabletext.depends =  src_corelib src_gui
examples_draganddrop_dropsite.subdir = $$QT_BUILD_TREE/examples/draganddrop/dropsite
examples_draganddrop_dropsite.depends =  src_corelib src_gui
examples_draganddrop_fridgemagnets.subdir = $$QT_BUILD_TREE/examples/draganddrop/fridgemagnets
examples_draganddrop_fridgemagnets.depends =  src_corelib src_gui
examples_draganddrop_puzzle.subdir = $$QT_BUILD_TREE/examples/draganddrop/puzzle
examples_draganddrop_puzzle.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_DRAGANDDROP_SUBDIRS
SUBDIRS += $$EXAMPLES_DRAGANDDROP_SUBDIRS
