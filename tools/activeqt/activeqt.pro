TEMPLATE = subdirs

CONFIG	+= 

contains(QT_EDITION, OpenSource|Console) {
    message("You are not licensed to use ActiveQt.")
} else {
unset(TOOLS_ACTIVEQT_SUBDIRS)
TOOLS_ACTIVEQT_SUBDIRS = tools_activeqt_dumpdoc \
                         tools_activeqt_dumpcpp \
                         tools_activeqt_testcon
}

#subdirs
tools_activeqt_dumpdoc.subdir = $$QT_BUILD_TREE/tools/activeqt/dumpdoc
tools_activeqt_dumpdoc.depends =  src_corelib src_gui
tools_activeqt_dumpcpp.subdir = $$QT_BUILD_TREE/tools/activeqt/dumpcpp
tools_activeqt_dumpcpp.depends =  src_corelib src_gui
tools_activeqt_testcon.subdir = $$QT_BUILD_TREE/tools/activeqt/testcon
tools_activeqt_testcon.depends =  src_corelib src_gui
TOOLS_SUB_SUBDIRS += $$TOOLS_ACTIVEQT_SUBDIRS
SUBDIRS += $$TOOLS_ACTIVEQT_SUBDIRS
