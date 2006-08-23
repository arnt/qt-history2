TEMPLATE = subdirs
unset(TOOLS_PORTING_SUBDIRS)
TOOLS_PORTING_SUBDIRS = tools_porting_src

#subdirs
tools_porting_src.subdir = $$QT_BUILD_TREE/tools/porting/src
tools_porting_src.depends =  src_xml src_corelib
TOOLS_SUB_SUBDIRS += $$TOOLS_PORTING_SUBDIRS
SUBDIRS += $$TOOLS_PORTING_SUBDIRS
