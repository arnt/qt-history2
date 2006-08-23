TEMPLATE = subdirs
unset(TOOLS_QTESTLIB_SUBDIRS)
TOOLS_QTESTLIB_SUBDIRS = tools_qtestlib_src \
                         tools_qtestlib_updater
CONFIG += 


#subdirs
tools_qtestlib_src.subdir = $$QT_BUILD_TREE/tools/qtestlib/src
tools_qtestlib_src.depends =  src_corelib
tools_qtestlib_updater.subdir = $$QT_BUILD_TREE/tools/qtestlib/updater
tools_qtestlib_updater.depends =  src_corelib
TOOLS_SUB_SUBDIRS += $$TOOLS_QTESTLIB_SUBDIRS
SUBDIRS += $$TOOLS_QTESTLIB_SUBDIRS
