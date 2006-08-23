TEMPLATE      = subdirs
unset(EXAMPLES_QTOPIACORE_SUBDIRS)
EXAMPLES_QTOPIACORE_SUBDIRS = examples_qtopiacore_mousecalibration

# install
EXAMPLES_QTOPIACORE_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS README *.pro
EXAMPLES_QTOPIACORE_install_sources.path = $$[QT_INSTALL_EXAMPLES]/qtopiacore
INSTALLS += EXAMPLES_QTOPIACORE_install_sources

#subdirs
examples_qtopiacore_mousecalibration.subdir = $$QT_BUILD_TREE/examples/qtopiacore/mousecalibration
examples_qtopiacore_mousecalibration.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_QTOPIACORE_SUBDIRS
SUBDIRS += $$EXAMPLES_QTOPIACORE_SUBDIRS
