TEMPLATE	= subdirs
unset(TOOLS_LINGUIST_SUBDIRS)
TOOLS_LINGUIST_SUBDIRS = tools_linguist_linguist \
                         tools_linguist_lrelease \
                         tools_linguist_lupdate
CONFIG += 


#subdirs
tools_linguist_linguist.subdir = $$QT_BUILD_TREE/tools/linguist/linguist
tools_linguist_linguist.depends =  src_corelib src_gui src_xml src_network tools_assistant_lib
tools_linguist_lrelease.subdir = $$QT_BUILD_TREE/tools/linguist/lrelease
tools_linguist_lrelease.depends =  src_corelib src_gui src_xml
tools_linguist_lupdate.subdir = $$QT_BUILD_TREE/tools/linguist/lupdate
tools_linguist_lupdate.depends =  src_corelib src_gui src_xml
TOOLS_SUB_SUBDIRS += $$TOOLS_LINGUIST_SUBDIRS
SUBDIRS += $$TOOLS_LINGUIST_SUBDIRS
