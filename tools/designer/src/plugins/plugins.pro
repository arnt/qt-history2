TEMPLATE = subdirs
CONFIG += ordered

contains(QT_CONFIG, qt3support): SUBDIRS += widgets
