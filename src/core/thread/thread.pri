# Qt core thread module

# public headers
HEADERS += thread/qmutex.h \
 	   thread/qsemaphore.h \
 	   thread/qthread.h \
 	   thread/qthreadstorage.h \
 	   thread/qwaitcondition.h \
	   thread/qatomic.h \
	   $$QT_SOURCE_TREE/include/QtCore/arch/qatomic.h
	
# private headers
HEADERS += thread/qmutex_p.h \
 	   thread/qmutexpool_p.h \
	   thread/qspinlock_p.h \
	   thread/qthread_p.h

SOURCES += thread/qmutexpool.cpp \
	   thread/qsemaphore.cpp \
	   thread/qmutex.cpp \
 	   thread/qthread.cpp \
           thread/qthreadstorage.cpp 

unix:SOURCES += thread/qmutex_unix.cpp \
		thread/qspinlock_unix.cpp \
		thread/qthread_unix.cpp \
		thread/qwaitcondition_unix.cpp

win32:SOURCES += thread/qmutex_win.cpp \
		 thread/qspinlock_win.cpp \
		 thread/qthread_win.cpp \
		 thread/qwaitcondition_win.cpp
