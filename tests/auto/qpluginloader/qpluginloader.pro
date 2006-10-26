TEMPLATE    =	subdirs
CONFIG  += ordered
SUBDIRS	=	lib \
		tst
TARGET = tst_qlibrary

# no special install rule for subdir
INSTALLS =
