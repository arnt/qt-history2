TEMPLATE = subdirs
CONFIG += ordered
win32:CONFIG += console
SUBDIRS = ping.pro pong.pro
DEFINES += QT_USE_USING_NAMESPACE
