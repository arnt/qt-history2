# Special files that are only for Qt/X11
X11ONLY	    =	dialogs/qprndlg.h \
		dialogs/qprndlg.cpp \
		kernel/qpsprn.h	\
		kernel/qpsprn.cpp \
		kernel/qnpsupport.cpp \
		kernel/qt_x11.cpp

# Non-public Qt files which we use internally in our Qt library
INT_HEADERS =	dialogs/qfontdialog.h \
		widgets/qheader.h \
		widgets/qscrollview.h \
		widgets/qspinbox.h
INT_SOURCES =	dialogs/qfontdialog.cpp \
		widgets/qheader.cpp \
		widgets/qscrollview.cpp \
		widgets/qspinbox.cpp

# Other non-public Qt files
INT_MISC    =	kernel/qpshdr.txt \
		kernel/qmutex.h \
		kernel/qthread.h \
		kernel/qthread.cpp

# Don't change anything below
INTERNAL    =	$$INT_HEADERS $$INT_SOURCES $$INT_MISC
DIRS	    =	dialogs kernel tools widgets
HEADERS    +=	$$INT_HEADERS
SOURCES    +=	$$INT_SOURCES
