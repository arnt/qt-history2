@echo off
rem
rem Configures to build the Qt library
rem
rem Copyright 1999-2000 Trolltech AS.  All rights reserved.
rem

rem **************************************
rem   Build environment sanity check
rem **************************************
if x%QTDIR% == x (
	echo QTDIR must be defined
	goto :end)
if x%MKSPEC% == x (
	echo MKSPEC must be defined
	goto :end)

rem **************************************
rem   Parse command line arguments
rem **************************************
set DEBUG=no
set THREADS=no
set SHARED=no
set QMAKE_VARS=
set QMAKE_CONFIG=
set QMAKE_OUTDIR=
set MAKE=nmake
:loop
if x%1==x goto endloop
if %1==debug set DEBUG=yes
if %1==thread set THREADS=yes
if %1==shared set SHARED=yes
shift
goto loop
:endloop

set QMAKE_CONFIG=internal
if x%DEBUG%==xyes (
	set QMAKE_CONFIG=%QMAKE_CONFIG% debug
	set QMAKE_OUTDIR=%QMAKE_OUTDIR%debug)
if x%DEBUG%==xno (
	set QMAKE_CONFIG=%QMAKE_CONFIG% release
	set QMAKE_OUTDIR=%QMAKE_OUTDIR%release)
if x%SHARED%==xyes (
	set QMAKE_CONFIG=%QMAKE_CONFIG% dll
	set QMAKE_OUTDIR=%QMAKE_OUTDIR%-shared)
if x%SHARED%==xno (
	set QMAKE_CONFIG=%QMAKE_CONFIG% staticlib
	set QMAKE_OUTDIR=%QMAKE_OUTDIR%-static)
if x%THREADS%==xyes (
	set QMAKE_CONFIG=%QMAKE_CONFIG% thread
	set QMAKE_OUTDIR=%QMAKE_OUTDIR%-mt)

set QMAKE_LIBDIR_QT=%QTDIR%\lib
set QMAKE_VARS=%QMAKE_VARS% "OBJECTS_DIR=.obj\%QMAKE_OUTDIR%" "MOC_DIR=.moc\%QMAKE_OUTDIR%"

rem **************************************
rem   Build qmake
rem **************************************
echo Creating qmake...
rem %QTDIR%\bin\syncqt
cd %QTDIR%\qmake
%MAKE%
cd ..

rem **************************************
rem   Generate .qmake.cache
rem **************************************
echo Generating .qmake.cache
echo CONFIG=%QMAKE_CONFIG%>.qmake.cache
echo MKSPEC=%MKSPEC%>>.qmake.cache

rem **************************************
rem   Create the makefiles
rem **************************************
echo Creating makefiles
set QMAKE_ALL_ARGS="CONFIG+=%QMAKE_CONFIG%" %QMAKE_VARS%

echo @echo off > tmp.bat
for /R %QTDIR% %%d IN (.) DO (
	set QMAKE_DIR=%%d%
	if exist %%d%\*.pro (
		for /F "usebackq" %%f IN (`dir /B %%d%\*.pro`) DO (
			set QMAKE_FILE=%%f%
			if not "!QMAKE_FILE!" == "qtmain.pro" (
				set QMAKE_EXTRA_ARGS="QMAKE_ABSOLUTE_SOURCE_PATH=!QMAKE_DIR!"
rem				set QMAKE_COMMAND=%QTDIR%\bin\qmake !QMAKE_DIR!\!QMAKE_FILE! %QMAKE_ALL_ARGS% !QMAKE_EXTRA_ARGS! -o !QMAKE_DIR!\Makefile -mkspec %MKSPEC%
				set QMAKE_COMMAND=%QTDIR%\bin\qmake !QMAKE_DIR!\!QMAKE_FILE! %QMAKE_ALL_ARGS% -o !QMAKE_DIR!\Makefile -mkspec %MKSPEC%

				echo echo !QMAKE_DIR!\!QMAKE_FILE! >> tmp.bat
				echo !QMAKE_COMMAND! >> tmp.bat
			)
		)
	)
)
call tmp.bat

cd %QTDIR%

echo =============================================
echo Qt is now ready for building.
echo Just type %MAKE% -f Makefile.win32
:end
