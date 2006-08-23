TEMPLATE      = subdirs
unset(EXAMPLES_LINGUIST_SUBDIRS)
EXAMPLES_LINGUIST_SUBDIRS = examples_linguist_arrowpad \
                            examples_linguist_hellotr \
                            examples_linguist_trollprint

# install
EXAMPLES_LINGUIST_install_sources.files = README *.pro
EXAMPLES_LINGUIST_install_sources.path = $$[QT_INSTALL_EXAMPLES]/linguist
INSTALLS += EXAMPLES_LINGUIST_install_sources

#subdirs
examples_linguist_arrowpad.subdir = $$QT_BUILD_TREE/examples/linguist/arrowpad
examples_linguist_arrowpad.depends =  src_corelib src_gui
examples_linguist_hellotr.subdir = $$QT_BUILD_TREE/examples/linguist/hellotr
examples_linguist_hellotr.depends =  src_corelib src_gui
examples_linguist_trollprint.subdir = $$QT_BUILD_TREE/examples/linguist/trollprint
examples_linguist_trollprint.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_LINGUIST_SUBDIRS
SUBDIRS += $$EXAMPLES_LINGUIST_SUBDIRS
