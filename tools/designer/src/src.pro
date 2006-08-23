TEMPLATE = subdirs
CONFIG += 

unset(TOOLS_DESIGNER_SRC_SUBDIRS)
include(components/components.pro)
TOOLS_DESIGNER_SRC_SUBDIRS = tools_designer_src_uitools \
                             tools_designer_src_lib \
                             tools_designer_src_designer

CONFIG(shared,shared|static): {
    include(plugins/plugins.pro)
}

#subdirs
tools_designer_src_uitools.subdir = $$QT_BUILD_TREE/tools/designer/src/uitools
tools_designer_src_uitools.depends =  src_corelib src_gui src_xml
tools_designer_src_lib.subdir = $$QT_BUILD_TREE/tools/designer/src/lib
tools_designer_src_lib.depends =  src_corelib src_gui src_xml tools_assistant_lib
tools_designer_src_designer.subdir = $$QT_BUILD_TREE/tools/designer/src/designer
tools_designer_src_designer.depends =  src_corelib src_gui src_xml src_network tools_designer_src_components_lib
TOOLS_DESIGNER_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_SUBDIRS
TOOLS_SUB_SUBDIRS += $$TOOLS_DESIGNER_SRC_SUBDIRS
SUBDIRS += $$TOOLS_DESIGNER_SRC_SUBDIRS
