include(../src/src.pri)

QT = core gui script network

CONFIG += qtestlib

DESTDIR = ./

win32: CONFIG += console
mac:CONFIG -= app_bundle

DEFINES += QLOCALSERVER_DEBUG QLOCALSOCKET_DEBUG

SOURCES		+= main.cpp 
TARGET		= lackey

DEFINES += QT_USE_USING_NAMESPACE

