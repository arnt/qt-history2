# Qt thread module

thread {
	# public headers
 	HEADERS += $$THREAD_H/qmutex.h \
 		   $$THREAD_H/qsemaphore.h \
 		   $$THREAD_H/qthread.h \
 		   $$THREAD_H/qthreadstorage.h \
 		   $$THREAD_H/qwaitcondition.h
	
        # private headers
	THREAD_P = thread
	HEADERS += $$THREAD_P/qmutex_p.h \
 		   $$THREAD_P/qmutexpool_p.h \
		   $$THREAD_P/qthreadinstance_p.h

 	SOURCES += $$THREAD_CPP/qmutexpool.cpp \
		   $$THREAD_CPP/qsemaphore.cpp \
 		   $$THREAD_CPP/qthread.cpp

	unix:SOURCES += $$THREAD_CPP/qmutex_unix.cpp \
			$$THREAD_CPP/qthread_unix.cpp \
			$$THREAD_CPP/qthreadstorage_unix.cpp \
			$$THREAD_CPP/qwaitcondition_unix.cpp

	win32:SOURCES += $$THREAD_CPP/qmutex_win.cpp \
			 $$THREAD_CPP/qthread_win.cpp \
			 $$THREAD_CPP/qthreadstorage_win.cpp \
			 $$THREAD_CPP/qwaitcondition_win.cpp
}
