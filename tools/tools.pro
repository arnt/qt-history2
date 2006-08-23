TEMPLATE        = subdirs
no-png {
    message("Tools not available without PNG support")
} else {
unset(TOOLS_SUBDIRS)
unix:contains(QT_CONFIG, qdbus):include(qdbus/qdbus.pro)
include(porting/porting.pro)
include(qtestlib/qtestlib.pro)
TOOLS_SUBDIRS = tools_assistant_lib \
                tools_assistant
    contains(QT_EDITION, Console) {
TOOLS_SUBDIRS += tools_designer_src_uitools
    } else {
include(designer/designer.pro)
    }
include(linguist/linguist.pro)
unix:!embedded:contains(QT_CONFIG, qt3support):TOOLS_SUBDIRS += tools_qtconfig
unix:!embedded:contains(QT_CONFIG, qdbus): {
    include(qdbus/qdbus.pro)
}
win32:!contains(QT_EDITION, OpenSource|Console): {
    include(activeqt/activeqt.pro)
}
}

#CONFIG+=ordered
QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

# This creates a sub-tools rule
sub_tools_target.CONFIG = recursive
sub_tools_target.recurse = $$TOOLS_SUBDIRS $$TOOLS_SUB_SUBDIRS 
sub_tools_target.target = sub-tools
sub_tools_target.recurse_target =
QMAKE_EXTRA_TARGETS += sub_tools_target

#subdirs
tools_assistant_lib.subdir = $$QT_BUILD_TREE/tools/assistant/lib
tools_assistant_lib.depends =  src_corelib src_gui src_network
tools_assistant.subdir = $$QT_BUILD_TREE/tools/assistant
tools_assistant.depends =  src_corelib src_gui src_xml src_network
tools_designer_src_uitools.subdir = $$QT_BUILD_TREE/tools/designer/src/uitools
tools_designer_src_uitools.depends =  src_corelib src_gui src_xml
tools_qtconfig.subdir = $$QT_BUILD_TREE/tools/qtconfig
tools_qtconfig.depends =  src_corelib src_gui src_qt3support
SUBDIRS += $$TOOLS_SUBDIRS
