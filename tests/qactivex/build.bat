@echo off
if "%1"=="clean" call unregcontrols
qmake
nmake %1
