TEMPLATE      = subdirs
unset(EXAMPLES_ASSISTANT_SUBDIRS)
EXAMPLES_ASSISTANT_SUBDIRS = examples_assistant_simpletextviewer

# install
target.path = $$[QT_INSTALL_EXAMPLES]/assistant
EXAMPLES_ASSISTANT_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS assistant.pro README
EXAMPLES_ASSISTANT_install_sources.path = $$[QT_INSTALL_EXAMPLES]/assistant
INSTALLS += target EXAMPLES_ASSISTANT_install_sources

#subdirs
examples_assistant_simpletextviewer.subdir = $$QT_BUILD_TREE/examples/assistant/simpletextviewer
examples_assistant_simpletextviewer.depends =  src_corelib src_gui src_network tools_assistant_lib
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_ASSISTANT_SUBDIRS
SUBDIRS += $$EXAMPLES_ASSISTANT_SUBDIRS
