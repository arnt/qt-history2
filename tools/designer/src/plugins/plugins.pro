TEMPLATE = subdirs
CONFIG += 

REQUIRES = !CONFIG(static,shared|static)
unset(TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS)
contains(QT_CONFIG, qt3support): TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS += tools_designer_src_plugins_widgets
win32:!contains(QT_EDITION, OpenSource):TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS += tools_designer_src_plugins_activeqt
# contains(QT_CONFIG, opengl): TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS += tools_designer_src_plugins_tools_view3d


#subdirs
tools_designer_src_plugins_widgets.subdir = $$QT_BUILD_TREE/tools/designer/src/plugins/widgets
tools_designer_src_plugins_widgets.depends =  src_corelib src_gui src_qt3support
tools_designer_src_plugins_activeqt.subdir = $$QT_BUILD_TREE/tools/designer/src/plugins/activeqt
tools_designer_src_plugins_activeqt.depends =  src_corelib src_gui
tools_designer_src_plugins_tools_view3d.subdir = $$QT_BUILD_TREE/tools/designer/src/plugins/tools/view3d
tools_designer_src_plugins_tools_view3d.depends =  src_corelib src_gui src_opengl
TOOLS_DESIGNER_SRC_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS
TOOLS_DESIGNER_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS
TOOLS_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS
SUBDIRS += $$TOOLS_DESIGNER_SRC_PLUGINS_SUBDIRS
