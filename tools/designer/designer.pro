TEMPLATE = subdirs

CONFIG += qt ordered debug
REQUIRES = !CONFIG(static,shared|static)

SUBDIRS = src


