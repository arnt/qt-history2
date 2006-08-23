TEMPLATE = subdirs
CONFIG += 
unset(EXAMPLES_QDBUS_COMPLEXPINGPONG_SUBDIRS)
EXAMPLES_QDBUS_COMPLEXPINGPONG_SUBDIRS = examples_qdbus_complexpingpong_complexping_pro \
                                         examples_qdbus_complexpingpong_complexpong_pro

#subdirs
examples_qdbus_complexpingpong_complexping_pro.file = $$QT_BUILD_TREE/examples/qdbus/complexpingpong/complexping.pro
examples_qdbus_complexpingpong_complexping_pro.depends =  src_corelib
examples_qdbus_complexpingpong_complexpong_pro.file = $$QT_BUILD_TREE/examples/qdbus/complexpingpong/complexpong.pro
examples_qdbus_complexpingpong_complexpong_pro.depends =  src_corelib
EXAMPLES_QDBUS_SUB_SUBDIRS += $$EXAMPLES_QDBUS_COMPLEXPINGPONG_SUBDIRS
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_QDBUS_COMPLEXPINGPONG_SUBDIRS
SUBDIRS += $$EXAMPLES_QDBUS_COMPLEXPINGPONG_SUBDIRS
