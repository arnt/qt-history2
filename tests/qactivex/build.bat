@echo off
if "%1"=="clean" call unregcontrols
qmake
nmake %1
if NOT "%1"=="clean" call regcontrols
