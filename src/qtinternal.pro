# Special files that are only for Qt/X11
X11ONLY	    =	dialogs/qprintdialog.h \
		dialogs/qprintdialog.cpp \
		kernel/qpsprinter.h	\
		kernel/qpsprinter.cpp \
		kernel/qnpsupport.cpp \
		kernel/qdnd_x11.cpp \
		kernel/qwidgetcreate_x11.cpp

# Non-public Qt files which we use internally in our Qt library
win32:INT_HEADERS = qt_windows.h
unix:INT_SOURCES = qt_x11.h

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
