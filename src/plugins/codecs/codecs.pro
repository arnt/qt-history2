TEMPLATE = subdirs

SUBDIRS = cn jp 
!contains(QT_CONFIG, bigcodecs):SUBDIRS	+= kr tw

