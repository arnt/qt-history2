TEMPLATE    =	subdirs
CONFIG  += ordered
SUBDIRS	=	lib \
                lib2 \
		tst
TARGET = tst_qlibrary

# no special install rule for subdir
INSTALLS =

DEFINES += QT_USE_USING_NAMESPACE

