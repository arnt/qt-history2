TEMPLATE      = subdirs
CONFIG       += 
unset(EXAMPLES_TOOLS_SUBDIRS)
include(plugandpaintplugins/plugandpaintplugins.pro)
EXAMPLES_TOOLS_SUBDIRS = examples_tools_codecs \
                         examples_tools_completer \
                         examples_tools_i18n \
                         examples_tools_plugandpaint \
                         examples_tools_regexp \
                         examples_tools_settingseditor
!cross_compile:EXAMPLES_TOOLS_SUBDIRS += examples_tools_qtdemo

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tools
EXAMPLES_TOOLS_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tools.pro README
EXAMPLES_TOOLS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/tools
INSTALLS += target EXAMPLES_TOOLS_install_sources

#subdirs
examples_tools_codecs.subdir = $$QT_BUILD_TREE/examples/tools/codecs
examples_tools_codecs.depends =  src_corelib src_gui
examples_tools_completer.subdir = $$QT_BUILD_TREE/examples/tools/completer
examples_tools_completer.depends =  src_corelib src_gui
examples_tools_i18n.subdir = $$QT_BUILD_TREE/examples/tools/i18n
examples_tools_i18n.depends =  src_corelib src_gui
examples_tools_plugandpaint.subdir = $$QT_BUILD_TREE/examples/tools/plugandpaint
examples_tools_plugandpaint.depends =  src_corelib src_gui
examples_tools_regexp.subdir = $$QT_BUILD_TREE/examples/tools/regexp
examples_tools_regexp.depends =  src_corelib src_gui
examples_tools_settingseditor.subdir = $$QT_BUILD_TREE/examples/tools/settingseditor
examples_tools_settingseditor.depends =  src_corelib src_gui
examples_tools_qtdemo.subdir = $$QT_BUILD_TREE/examples/tools/qtdemo
examples_tools_qtdemo.depends =  src_corelib src_gui src_xml src_network tools_assistant_lib
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_TOOLS_SUBDIRS
SUBDIRS += $$EXAMPLES_TOOLS_SUBDIRS
