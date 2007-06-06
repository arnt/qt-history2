CONFIG += qttest_p4
#QT = core

include(../qsharedmemory/src/qsharedmemory.pri)
win32: CONFIG += console

DEFINES	+= QSHAREDMEMORY_DEBUG
DEFINES	+= QSYSTEMSEMAPHORE_DEBUG

SOURCES		+= tst_qsystemsemaphore.cpp 
TARGET		= tst_qsystemsemaphore
