@echo off
rem
rem Configures to build the Qt library
rem
rem Copyright 2001 Trolltech AS.  All rights reserved.
rem

setlocal

rem **************************************
rem   Build environment sanity check
rem **************************************
if x%QTDIR% == x (
	echo QTDIR must be defined
	goto :end)

set DELEXP=yes
if not %DELEXP%==!DELEXP! (
	echo Delayed variable expansion is not enabled.
	echo Spawning a new shell with delayed expansion enabled...
	cmd /V:ON /C call configure.bat
	goto :end)

rem **************************************
rem   Defaults
rem **************************************
set CONFIG_CMDLINE=%*
set QCONFIG=full
set EMBEDDED=no
set DEBUG=no
set SHARED=yes
set GIF=no
set THREAD=no
set ZLIB=yes
set LIBPNG=yes
set JPEG=no
set MNG=no
set QMAKE_VARS=
set QMAKE_CONFIG=
set QMAKE_OUTDIR=
set MAKE=nmake
set XMKSPEC=
set MODULES=tools kernel widgets dialogs iconview workspace network canvas table xml opengl sql styles
set MODULES_UPPER=TOOLS KERNEL WIDGETS DIALOGS ICONVIEW WORKSPACE NETWORK CANVAS TABLE XML OPENGL SQL STYLES

rem **************************************
rem   Parse command line arguments
rem **************************************

:loop
if x%1==x goto endloop
if x%1==x-help set HELP=yes
if x%1==x-qconfig (
	set QCONFIG=%2
	shift)
if x%1==x-release set DEBUG=NO
if x%1==x-debug set DEBUG=yes
if x%1==x-shared set SHARED=yes
if x%1==x-static set SHARED=no
if x%1==x-no-thread set THREAD=no
if x%1==x-thread set THREAD=yes
if x%1==xmkspec (
	set XMKSPEC=%2
	shift)
if x%1==x-no-gif set GIF=no
if x%1==x-qt-gif set GIF=yes
if x%1==x-qt-zlib set ZLIB=yes
if x%1==x-system-zlib set ZLIB=no
if x%1==x-qt-libpng set LIBPNG=yes
if x%1==x-system-libpng set LIBPNG=no
if x%1==x-no-mng set MNG=no
if x%1==x-system-mng set MNG=yes
if x%1==x-no-jpeg set JPEG=no
if x%1==x-system-jpeg set JPEG=yes

cd src
for %%m in ( %MODULES% ) do (
	set ENABLEMODULE=x-enable-%%m%
	set DISABLEMODULE=x-disable-%%m%
	if x%1==x-enable-%%m% (
		set QMAKE_CONFIG=!QMAKE_CONFIG! %%m%
		)
	if x%1==x-disable-%%m% (
		set DISABLED_%%m%=yes
		)
)
cd ..

rem *** The following is a workaround
if x%1==x-internal set QMAKE_CONFIG=%MODULES%
shift
goto loop
:endloop

rem ******************************************
rem   Figure out which configuration to use
rem ******************************************
set QMAKE_CONFIG_TMP=
for %%c in ( minimal small medium large full ) do (
	set QMAKE_CONFIG_TMP=!QMAKE_CONFIG_TMP! %%c%-config
	if %%c%==!QCONFIG! goto foundconfig
)
echo No such configuration: %QCONFIG%
set HELP=yes
:foundconfig
set QMAKE_CONFIG=%QMAKE_CONFIG% %QMAKE_CONFIG_TMP%

rem **************************************
rem   Display help text if needed
rem **************************************
if x%HELP%==xyes (
	@echo .
	@echo Command line arguments:   * indicates default behaviour
	@echo .
	@echo -help                  Bring up this help text.
	@echo -debug                 Enable debug information.
	@echo -release             * Disable debug information.
	@echo -shared              * Build Qt as a shared library.
	@echo -static                Build Qt as a static library.
	@echo -thread                Configure Qt with thread support.
	@echo -no-thread           * Configure Qt without thread support.
	@echo -mkspec                Specify a mkspec, uses %%MKSPEC%% as default.
	@echo -qconfig               Specify config, available configs:
	@echo                          minimal
	@echo                          small
	@echo                          medium
	@echo                          large
	@echo                      *   full
	@echo -qt-gif                Enable GIF support.
	@echo -no-gif              * Disable GIF support.
	@echo -qt-zlib             * Compile in zlib
	@echo -system-zlib           Use existing zlib in system
	@echo -qt-libpng           * Compile in libPNG
	@echo -system-libpng         Use existing libPNG in system
	@echo -no-mng              * Disable MNG support
	@echo -system-mng            Enable MNG support
	@echo -no-jpeg             * Disable JPEG support
	@echo -system-jpeg           Enable JPEG support
	@echo .
	@echo -enable-*              Enable the specified module
	@echo -disable-*             Disable the specified module
	@echo .
	goto end )

rem **************************************
rem   Create qconfig file
rem **************************************
echo Generating qconfig.h
set QCONFIGTMP=src\tools\qconfig.h.new
set QCONFIGTARGET=src\tools\qconfig.h
if exist %QCONFIGTMP% del /F %QCONFIGTMP%
if x%QCONFIG%==xfull (
	echo // Everything > %QCONFIGTMP%
) else (
	set QCONFIGFILE=qconfig-%QCONFIG%.h
	echo // Copied from !QCONFIGFILE! > %QCONFIGTMP%
	type !QCONFIGFILE! >> %QCONFIGTMP%
)
rem *** We should really do a diff on the files, but the Windows COMP command
rem *** is broken...
if exist %QCONFIGTARGET% attrib -r %QCONFIGTARGET%
move /Y %QCONFIGTMP% %QCONFIGTARGET%

rem **************************************
rem   Build variables for output
rem **************************************
if x%DEBUG%==xyes (
set QMAKE_CONFIG=%QMAKE_CONFIG% debug
set QMAKE_OUTDIR=%QMAKE_OUTDIR%debug)
if x%DEBUG%==xno (
set QMAKE_CONFIG=%QMAKE_CONFIG% release
set QMAKE_OUTDIR=%QMAKE_OUTDIR%release)
if x%THREAD%==xyes (
set QMAKE_CONFIG=%QMAKE_CONFIG% thread
set QMAKE_OUTDIR=%QMAKE_OUTDIR%-mt)

set QMAKE_VARS=%QMAKE_VARS% "QMAKE_LIBDIR_QT=%QTDIR%\lib"
set QMAKE_VARS=%QMAKE_VARS% "OBJECTS_DIR=.obj\%QMAKE_OUTDIR%" "MOC_DIR=.moc\%QMAKE_OUTDIR%"

if x%SHARED%==xyes (
set QMAKE_CONFIG=%QMAKE_CONFIG% dll
set QMAKE_OUTDIR=%QMAKE_OUTDIR%-shared
set QMAKE_VARS=%QMAKE_VARS% "DEFINES+=QT_DLL")
if x%SHARED%==xno (
set QMAKE_CONFIG=%QMAKE_CONFIG% staticlib
set QMAKE_OUTDIR=%QMAKE_OUTDIR%-static)

if x%JPEG%==xyes set QMAKE_CONFIG=%QMAKE_CONFIG% jpeg
if x%MNG%==xyes set QMAKE_CONFIG=%QMAKE_CONFIG% mng
if x%GIF%==xyes set QMAKE_CONFIG=%QMAKE_CONFIG% gif
if x%ZLIB%==xyes set QMAKE_CONFIG=%QMAKE_CONFIG% zlib
if x%LIBPNG%==xyes set QMAKE_CONFIG=%QMAKE_CONFIG% libpng

if x%XMKSPEC% == x set XMKSPEC=%MKSPEC%

if x%XMKSPEC% == x (
	echo MKSPEC must be defined
	goto end)

rem **************************************
rem   Generating qmodules.h
rem **************************************
echo Generating qmodules.h
set QMODULESTMP=src\tools\qmodules.h.new
set QMODULESTARGET=src\tools\qmodules.h
if exist %QMODULESTMP% del /F %QMODULESTMP%
echo // These modules are present in this configuration of Qt > %QMODULESTMP%
for %%m in ( %MODULES% ) do (
	if not x!DISABLED_%%m%!==xyes (
		for %%u in ( %MODULES_UPPER% ) do (
			if /I %%u==%%m echo #define QT_MODULE_%%u >> %QMODULESTMP%
		)
	)
)
rem *** We should really do a diff on the files, but the Windows COMP command
rem *** is broken...
if exist %QMODULESTARGET% attrib -r %QMODULESTARGET%
move /Y %QMODULESTMP% %QMODULESTARGET%

rem **************************************
rem   Generate .qmake.cache
rem **************************************
echo Generating .qmake.cache
if exist .qmake.cache del .qmake.cache
for %%v in ( %QMAKE_VARS% ) do (
	echo %%~v >> .qmake.cache
)
echo CONFIG=%QMAKE_CONFIG%>>.qmake.cache
echo MKSPEC=%XMKSPEC%>>.qmake.cache

rem **************************************
rem   Give some feedback
rem **************************************
echo Configuration.........................%QMAKE_CONFIG%
echo Thread support........................%THREAD%
echo GIF support...........................%GIF%
echo MNG support...........................%MNG%
echo JPEG support..........................%JPEG%

rem **************************************
rem   Build qmake
rem **************************************
echo Creating qmake...
cd qmake
%MAKE%
cd ..

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
rem				set QMAKE_COMMAND=%QTDIR%\bin\qmake !QMAKE_DIR!\!QMAKE_FILE! %QMAKE_ALL_ARGS% !QMAKE_EXTRA_ARGS! -o !QMAKE_DIR!\Makefile -mkspec %XMKSPEC%
				set QMAKE_COMMAND=%QTDIR%\bin\qmake !QMAKE_DIR!\!QMAKE_FILE! %QMAKE_ALL_ARGS% -o !QMAKE_DIR!\Makefile -mkspec %XMKSPEC%

				echo echo !QMAKE_DIR!\!QMAKE_FILE! >> tmp.bat
				echo !QMAKE_COMMAND! >> tmp.bat
			)
		)
	)
)
call tmp.bat

rem *************************************
rem   Clean up
rem *************************************
del tmp.bat

set LASTCONFIG=lastconfig.bat
if exist %LASTCONFIG% del %LASTCONFIG%
echo configure %CONFIG_CMDLINE% > %LASTCONFIG%

echo .
echo Qt is now configured for building. Just run %MAKE%
echo To reconfigure, run %MAKE% clean and configure
echo .

:end

endlocal
