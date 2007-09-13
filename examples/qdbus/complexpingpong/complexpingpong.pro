TEMPLATE = subdirs
CONFIG += ordered
win32:CONFIG += console
SUBDIRS = complexping.pro complexpong.pro
DEFINES += QT_USE_USING_NAMESPACE
