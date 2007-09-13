TEMPLATE    =	subdirs
CONFIG  += ordered
SUBDIRS	=	lib \
                theplugin \
		tst
!win32: SUBDIRS += almostplugin
TARGET = tst_qpluginloader

# no special install rule for subdir
INSTALLS =

DEFINES += QT_USE_USING_NAMESPACE

