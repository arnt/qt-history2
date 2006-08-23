TEMPLATE = subdirs
CONFIG += 
unset(EXAMPLES_QDBUS_PINGPONG_SUBDIRS)
EXAMPLES_QDBUS_PINGPONG_SUBDIRS = examples_qdbus_pingpong_ping_pro \
                                  examples_qdbus_pingpong_pong_pro

#subdirs
examples_qdbus_pingpong_ping_pro.file = $$QT_BUILD_TREE/examples/qdbus/pingpong/ping.pro
examples_qdbus_pingpong_ping_pro.depends =  src_corelib
examples_qdbus_pingpong_pong_pro.file = $$QT_BUILD_TREE/examples/qdbus/pingpong/pong.pro
examples_qdbus_pingpong_pong_pro.depends =  src_corelib
EXAMPLES_QDBUS_SUB_SUBDIRS += $$EXAMPLES_QDBUS_PINGPONG_SUBDIRS
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_QDBUS_PINGPONG_SUBDIRS
SUBDIRS += $$EXAMPLES_QDBUS_PINGPONG_SUBDIRS
