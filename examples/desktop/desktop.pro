TEMPLATE      = subdirs
CONFIG       += 
unset(EXAMPLES_DESKTOP_SUBDIRS)
EXAMPLES_DESKTOP_SUBDIRS = examples_desktop_screenshot \
                           examples_desktop_systray

# install
target.path = $$[QT_INSTALL_EXAMPLES]/desktop
EXAMPLES_DESKTOP_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS desktop.pro README
EXAMPLES_DESKTOP_install_sources.path = $$[QT_INSTALL_EXAMPLES]/desktop
INSTALLS += target EXAMPLES_DESKTOP_install_sources

#subdirs
examples_desktop_screenshot.subdir = $$QT_BUILD_TREE/examples/desktop/screenshot
examples_desktop_screenshot.depends =  src_corelib src_gui
examples_desktop_systray.subdir = $$QT_BUILD_TREE/examples/desktop/systray
examples_desktop_systray.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_DESKTOP_SUBDIRS
SUBDIRS += $$EXAMPLES_DESKTOP_SUBDIRS
