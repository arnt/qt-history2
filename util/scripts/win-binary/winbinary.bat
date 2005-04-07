rem ***********************************************
rem creates a binary package from a src package
rem requires that perl and unzip are in your path
rem %1 = working directory
rem %2 = package name (without extension)
rem %3 = compiler (VS2003, optional)
rem %4 = build directory (optional, default compile)
rem %5 = NSIS directory (optional)
rem ***********************************************

set TMP_COMPILER=%3
set TMP_BUILDDIR=%4
set TMP_NSISDIR=%5

if "%3"=="" set TMP_COMPILER=VS2003
if "%4"=="" set TMP_BUILDDIR=compile
if "%5"=="" set TMP_NSISDIR="C:\Program Files\NSIS"

call :ExtractAndCopy %1 %2

rem call :Compile %1

call :OrganizeClean %1

echo Done!
goto END



rem ***********************************************
rem Organize the binary package
rem %1 = working directory
rem ***********************************************
:OrganizeClean
echo Organizing the binary package...
cd %1\clean

echo - Regenerate include directory
rd /S /Q include
set TMP_QTDIR=%QTDIR%
set QTDIR=%1\clean
perl %1\clean\bin\syncqt -copy >> %1\log.txt 2>&1
set QTDIR=%TMP_QTDIR%
set TMP_QTDIR=

echo - Removing source dir in clean
rd /S /Q src

echo - Copying the binaries
del /S /Q bin\*.exe
xcopy /E /Q /I %1\%TMP_BUILDDIR%\bin\*.exe %1\clean\bin

echo - Copying the dll's
xcopy /E /Q /I %1\%TMP_BUILDDIR%\lib\*.dll %1\clean\bin

echo - Copying the lib's
xcopy /E /Q /I %1\%TMP_BUILDDIR%\lib\*.lib %1\clean\lib

echo - Copying the pdb's
xcopy /E /Q /I %1\%TMP_BUILDDIR%\lib\*.pdb %1\clean\bin

echo - Copying additional files
xcopy /E /Q %1\%TMP_BUILDDIR%\src\corelib\global\qconfig.h %1\clean\include\QtCore\
mkdir %1\clean\include\QtCore\arch
xcopy /E /Q %1\%TMP_BUILDDIR%\src\corelib\arch\windows\arch\qatomic.h %1\clean\include\QtCore\arch\
goto :EOF



rem ***********************************************
rem Create NSIS package
rem %1 = working directory
rem ***********************************************
:CreateNSISPackage
echo Creating NSIS Package
cd %1

echo - Updating the NSIS Qt Plugin
if exist "%TMP_NSISDIR%\Plugins\qtnsisext.dll" del /Q "%TMP_NSISDIR%\Plugins\qtnsisext.dll"
xcopy /E /Q %1\qtnsisext\qtnsisext.dll %TMP_NSISDIR%\Plugins\

echo - Running makensis
"%TMP_NSISDIR%\makensis.exe" installscriptwin.nsi

goto :EOF



rem ***********************************************
rem Compile
rem %1 = working directory
rem ***********************************************
:Compile
echo Compiling in %1\%TMP_BUILDDIR% (%TMP_COMPILER%)...
call :SetEnv %1
cd %QTDIR%

echo - Creating license file
type LICENSE.TROLL > LICENSE.TROLL

echo - Running configure...
configure -release -qt-sql-sqlite -no-style-windowsxp -qt-zlib -qt-png -qt-jpeg 1>>log.txt >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo - Building...
echo   * Qt (debug and release)
nmake sub-src-all >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
echo   * Tools (release)
nmake sub-tools >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

call :ResetEnv
goto :EOF



rem ***********************************************
rem Sets up the environment for compiling
rem %1 = working directory
rem ***********************************************
:SetEnv
echo - Setting environment variables...
set OLD_PATH=%PATH%
set OLD_QMAKESPEC=%QMAKESPEC%
set OLD_QTDIR=%QTDIR%
set OLD_INCLUDE=%INCLUDE%
set OLD_LIB=%LIB%

set PATH=%1\%TMP_BUILDDIR%\bin;%PATH%
set QTDIR=%1\%TMP_BUILDDIR%

if "%TMP_COMPILER%"=="VS2003" goto VS2003Env
goto :EOF

:VS2003Env
set QMAKESPEC=win32-msvc.net
call "%VS71COMNTOOLS%vsvars32.bat" >> %1\log.txt
goto :EOF



rem ***********************************************
rem Reset environment after compiling
rem ***********************************************
:ResetEnv
echo - Resetting environment variables...
set PATH=%OLD_PATH%
set QMAKESPEC=%OLD_QMAKESPEC%
set QTDIR=%OLD_QTDIR%
set INCLUDE=%OLD_INCLUDE%
set LIB=%OLD_LIB%
goto :EOF



rem ***********************************************
rem Extracts and copies the package into two
rem directories, BUILDIR and clean.
rem %1 = working directory
rem %2 = package name
rem ***********************************************
:ExtractAndCopy
echo Extracting source package...
if not exist %1 mkdir %1
cd %1

if exist log.txt del log.txt

echo - Creating directories
if exist clean rd /S /Q clean
if exist %3 rd /S /Q %TMP_BUILDDIR%
if exist %2 rd /S /Q %2

mkdir clean
if not %errorlevel%==0 goto FAILED

echo - Extracting package
unzip %2.zip >> log.txt
if not %errorlevel%==0 goto FAILED
rename %2 %TMP_BUILDDIR%

echo - Copying directories
xcopy /E /Q %TMP_BUILDDIR% clean
if not %errorlevel%==0 goto FAILED
goto :EOF



:FAILED
echo Failed!

:END
set OLD_PATH=
set OLD_QMAKESPEC=
set OLD_QTDIR=
set OLD_INCLUDE=
set OLD_LIB=
set TMP_COMPILER=
set TMP_BUILDDIR=
set TMP_NSISDIR=