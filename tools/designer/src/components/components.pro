TEMPLATE = subdirs
CONFIG += 

unset(TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS)
TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS = tools_designer_src_components_lib




#subdirs
tools_designer_src_components_lib.subdir = $$QT_BUILD_TREE/tools/designer/src/components/lib
tools_designer_src_components_lib.depends =  src_corelib src_gui tools_designer_src_lib
TOOLS_DESIGNER_SRC_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS
TOOLS_DESIGNER_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS
TOOLS_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS
SUBDIRS += $$TOOLS_DESIGNER_SRC_COMPONENTS_SUBDIRS
