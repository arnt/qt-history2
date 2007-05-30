CONFIG += qttest_p4
CONFIG += ordered

include(../../src/qsharedmemory.pri)
CONFIG -= app_bundle

DEFINES	+= QSHAREDMEMORY_DEBUG
DEFINES	+= QSYSTEMSEMAPHORE_DEBUG

DESTDIR = ../
SOURCES		+= ../tst_qsharedmemory.cpp 
TARGET		= tst_qsharedmemory
