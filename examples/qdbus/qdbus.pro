TEMPLATE = subdirs
unset(EXAMPLES_QDBUS_SUBDIRS)
include(pingpong/pingpong.pro)
include(complexpingpong/complexpingpong.pro)
EXAMPLES_QDBUS_SUBDIRS = examples_qdbus_listnames \
                         examples_qdbus_chat

#subdirs
examples_qdbus_listnames.subdir = $$QT_BUILD_TREE/examples/qdbus/listnames
examples_qdbus_listnames.depends =  src_corelib
examples_qdbus_chat.subdir = $$QT_BUILD_TREE/examples/qdbus/chat
examples_qdbus_chat.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_QDBUS_SUBDIRS
SUBDIRS += $$EXAMPLES_QDBUS_SUBDIRS
