TEMPLATE = subdirs

!contains(QT_CONFIG, no-jpeg):!contains(QT_CONFIG, jpeg):SUBDIRS += jpeg
!contains(QT_CONFIG, no-gif):SUBDIRS += gif
!contains(QT_CONFIG, no-mng):SUBDIRS += mng

