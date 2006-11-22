TEMPLATE = subdirs
win32 {
    exists($$[QT_INSTALL_LIBS]/QtCore4.dll) {
        SUBDIRS = releaseplugin
    }
    exists($$[QT_INSTALL_LIBS]/QtCored4.dll) {
        SUBDIRS += debugplugin
    }
}else{
    SUBDIRS = debugplugin releaseplugin
    tst_qplugin_pro.depends += debugplugin releaseplugin
} 
SUBDIRS += tst_qplugin.pro
