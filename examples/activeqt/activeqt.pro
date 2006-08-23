TEMPLATE      = subdirs
unset(EXAMPLES_ACTIVEQT_SUBDIRS)
EXAMPLES_ACTIVEQT_SUBDIRS += examples_activeqt_comapp \
                             examples_activeqt_hierarchy \
                             examples_activeqt_menus \
                             examples_activeqt_multiple \
                             examples_activeqt_simple \
                             examples_activeqt_webbrowser \
                             examples_activeqt_wrapper

contains(QT_CONFIG, opengl):EXAMPLES_ACTIVEQT_SUBDIRS += examples_activeqt_opengl

# For now only the contain examples with mingw, for the others you need
# an IDL compiler
win32-g++:EXAMPLES_ACTIVEQT_SUBDIRS = examples_activeqt_webbrowser

# install
target.path = $$[QT_INSTALL_EXAMPLES]/activeqt
EXAMPLES_ACTIVEQT_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS activeqt.pro
EXAMPLES_ACTIVEQT_install_sources.path = $$[QT_INSTALL_EXAMPLES]/activeqt
INSTALLS += target EXAMPLES_ACTIVEQT_install_sources

#subdirs
examples_activeqt_comapp.subdir = $$QT_BUILD_TREE/examples/activeqt/comapp
examples_activeqt_comapp.depends =  src_corelib src_gui
examples_activeqt_hierarchy.subdir = $$QT_BUILD_TREE/examples/activeqt/hierarchy
examples_activeqt_hierarchy.depends =  src_corelib src_gui
examples_activeqt_menus.subdir = $$QT_BUILD_TREE/examples/activeqt/menus
examples_activeqt_menus.depends =  src_corelib src_gui
examples_activeqt_multiple.subdir = $$QT_BUILD_TREE/examples/activeqt/multiple
examples_activeqt_multiple.depends =  src_corelib src_gui
examples_activeqt_simple.subdir = $$QT_BUILD_TREE/examples/activeqt/simple
examples_activeqt_simple.depends =  src_corelib src_gui
examples_activeqt_webbrowser.subdir = $$QT_BUILD_TREE/examples/activeqt/webbrowser
examples_activeqt_webbrowser.depends =  src_corelib src_gui
examples_activeqt_wrapper.subdir = $$QT_BUILD_TREE/examples/activeqt/wrapper
examples_activeqt_wrapper.depends =  src_corelib src_gui
examples_activeqt_opengl.subdir = $$QT_BUILD_TREE/examples/activeqt/opengl
examples_activeqt_opengl.depends =  src_corelib src_gui src_opengl
examples_activeqt_webbrowser.subdir = $$QT_BUILD_TREE/examples/activeqt/webbrowser
examples_activeqt_webbrowser.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_ACTIVEQT_SUBDIRS
SUBDIRS += $$EXAMPLES_ACTIVEQT_SUBDIRS
