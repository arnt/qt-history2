TEMPLATE = subdirs

contains(QT_CONFIG, accessibility) {
     SUBDIRS += widgets 
     contains(QT_CONFIG, compat):SUBDIRS += compat
}
