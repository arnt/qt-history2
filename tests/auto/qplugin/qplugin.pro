TEMPLATE = subdirs
SUBDIRS = debugplugin releaseplugin tst_qplugin.pro

tst_qplugin_pro.depends += debugplugin releaseplugin
