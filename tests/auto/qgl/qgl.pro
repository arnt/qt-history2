############################################################
# Project file for autotest for file qgl.h
############################################################

load(qttest_p4)
!embedded: QT += opengl

SOURCES += tst_qgl.cpp

DEFINES += QT_USE_USING_NAMESPACE

