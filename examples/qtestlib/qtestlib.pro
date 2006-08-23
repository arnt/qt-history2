TEMPLATE = subdirs
unset(EXAMPLES_QTESTLIB_SUBDIRS)
EXAMPLES_QTESTLIB_SUBDIRS = examples_qtestlib_tutorial1 \
                            examples_qtestlib_tutorial2 \
                            examples_qtestlib_tutorial3 \
                            examples_qtestlib_tutorial4

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qtestlib
EXAMPLES_QTESTLIB_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qtestlib.pro README
EXAMPLES_QTESTLIB_install_sources.path = $$[QT_INSTALL_EXAMPLES]/qtestlib
INSTALLS += target EXAMPLES_QTESTLIB_install_sources

#subdirs
examples_qtestlib_tutorial1.subdir = $$QT_BUILD_TREE/examples/qtestlib/tutorial1
examples_qtestlib_tutorial1.depends =  src_corelib src_gui
examples_qtestlib_tutorial2.subdir = $$QT_BUILD_TREE/examples/qtestlib/tutorial2
examples_qtestlib_tutorial2.depends =  src_corelib src_gui
examples_qtestlib_tutorial3.subdir = $$QT_BUILD_TREE/examples/qtestlib/tutorial3
examples_qtestlib_tutorial3.depends =  src_corelib src_gui
examples_qtestlib_tutorial4.subdir = $$QT_BUILD_TREE/examples/qtestlib/tutorial4
examples_qtestlib_tutorial4.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_QTESTLIB_SUBDIRS
SUBDIRS += $$EXAMPLES_QTESTLIB_SUBDIRS
