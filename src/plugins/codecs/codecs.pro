TEMPLATE = subdirs

SUBDIRS = cn
!contains(QT_CONFIG, bigcodecs):SUBDIRS	+= jp kr tw

