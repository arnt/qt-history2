INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += $$PWD/qsystemlock.cpp \
           $$PWD/qsharedmemory.cpp \
           $$PWD/qsystemsemaphore.cpp

HEADERS += $$PWD/qsystemlock.h \
           $$PWD/qsystemlock_p.h \
           $$PWD/qsharedmemory.h \
           $$PWD/qsystemsemaphore.h \
           $$PWD/qsharedmemory_p.h \
           $$PWD/qsystemsemaphore_p.h

unix:SOURCES += $$PWD/qsystemlock_unix.cpp \
                $$PWD/qsharedmemory_unix.cpp \
                $$PWD/qsystemsemaphore_unix.cpp
win32:SOURCES += $$PWD/qsystemlock_win.cpp \
                 $$PWD/qsharedmemory_win.cpp \
                 $$PWD/qsystemsemaphore_win.cpp
