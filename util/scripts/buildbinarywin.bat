@echo off
rem ***********************************************
rem Used for building Win binaries
rem
rem Usage:
rem     buildbinarywin.bat makespec localpackagedir
rem ***********************************************
set QMAKESPEC=%1
if not %errorlevel%==0 goto error
if %1==win32-msvc (
	echo win32-msvc
	call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat"
	if not %errorlevel%==0 goto error
) else if %1==win32-msvc.net (
	echo win32-msvc.net
	call "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32.bat"
	if not %errorlevel%==0 goto error
) else (
	exit 1
)
set QTDIR=%CD%\%2
if not %errorlevel%==0 goto error
set PATH=%QTDIR%\bin;%PATH%
if not %errorlevel%==0 goto error
cd %2
if not %errorlevel%==0 goto error
type LICENSE.TROLL > LICENSE.TROLL
if not %errorlevel%==0 goto error
configure.exe -debug
if not %errorlevel%==0 goto error
nmake sub-src-all sub-tools
if not %errorlevel%==0 goto error

:error
exit errorlevel
