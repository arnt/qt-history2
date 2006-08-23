TEMPLATE      = subdirs
unset(EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_SUBDIRS)
EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_SUBDIRS = examples_tools_plugandpaintplugins_basictools \
                                             examples_tools_plugandpaintplugins_extrafilters

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins
EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS plugandpaintplugins.pro
EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins
INSTALLS += target EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_install_sources

#subdirs
examples_tools_plugandpaintplugins_basictools.subdir = $$QT_BUILD_TREE/examples/tools/plugandpaintplugins/basictools
examples_tools_plugandpaintplugins_basictools.depends =  src_corelib src_gui
examples_tools_plugandpaintplugins_extrafilters.subdir = $$QT_BUILD_TREE/examples/tools/plugandpaintplugins/extrafilters
examples_tools_plugandpaintplugins_extrafilters.depends =  src_corelib src_gui
EXAMPLES_TOOLS_SUB_SUBDIRS += $$EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_SUBDIRS
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_SUBDIRS
SUBDIRS += $$EXAMPLES_TOOLS_PLUGANDPAINTPLUGINS_SUBDIRS
