TEMPLATE = subdirs

!contains(QT_CONFIG, no-png):!contains(QT_CONFIG, png):SUBDIRS   += png
!contains(QT_CONFIG, no-jpeg):!contains(QT_CONFIG, jpeg):SUBDIRS += jpeg
!contains(QT_CONFIG, no-mng):!contains(QT_CONFIG, mng):SUBDIRS   += mng
