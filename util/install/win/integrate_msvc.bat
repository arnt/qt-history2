@echo off
cd %QTDIR%\tools\designer\integration\qmsdev
nmake MSVCADDINS=%1
