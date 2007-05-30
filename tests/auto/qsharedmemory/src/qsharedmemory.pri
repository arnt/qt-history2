INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += $$PWD/qsharedmemory.cpp \
           $$PWD/qsystemlock.cpp \
           $$PWD/qsystemsemaphore.cpp

unix:SOURCES += $$PWD/qsharedmemory_unix.cpp \
                $$PWD/qsystemlock_unix.cpp \
                $$PWD/qsystemsemaphore_unix.cpp

win32:SOURCES += $$PWD/qsharedmemory_win.cpp \
                 $$PWD/qsystemlock_win.cpp \
                 $$PWD/qsystemsemaphore_win.cpp

HEADERS += $$PWD/qsharedmemory.h \
	   $$PWD/qsharedmemory_p.h \
           $$PWD/qsystemlock.h \
	   $$PWD/qsystemlock_p.h \
           $$PWD/qsystemsemaphore.h \
	   $$PWD/qsystemsemaphore_p.h
