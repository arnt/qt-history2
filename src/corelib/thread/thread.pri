# Qt core thread module

# public headers
HEADERS += thread/qmutex.h \
           thread/qreadwritelock.h \
           thread/qsemaphore.h \
 	   thread/qthread.h \
 	   thread/qthreadstorage.h \
 	   thread/qwaitcondition.h \
	   thread/qatomic.h
	
# private headers
HEADERS += thread/qmutex_p.h \
           thread/qmutexpool_p.h \
           thread/qthread_p.h

SOURCES += thread/qmutex.cpp \
           thread/qreadwritelock.cpp \
	   thread/qmutexpool.cpp \
	   thread/qsemaphore.cpp \
 	   thread/qthread.cpp \
           thread/qthreadstorage.cpp 

unix:SOURCES += thread/qmutex_unix.cpp \
                thread/qthread_unix.cpp \
		thread/qwaitcondition_unix.cpp

win32:SOURCES += thread/qmutex_win.cpp \
                 thread/qthread_win.cpp \
		 thread/qwaitcondition_win.cpp
