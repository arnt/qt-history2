TEMPLATE = subdirs
CONFIG += 
unset(TOOLS_QDBUS_SUBDIRS)
include(tools/tools.pro)
TOOLS_QDBUS_SUBDIRS = tools_qdbus_src


#subdirs
tools_qdbus_src.subdir = $$QT_BUILD_TREE/tools/qdbus/src
tools_qdbus_src.depends =  src_corelib src_xml
TOOLS_SUB_SUBDIRS += $$TOOLS_QDBUS_SUBDIRS
SUBDIRS += $$TOOLS_QDBUS_SUBDIRS
