@echo off
rem ***********************************************
rem creates a binary package from a src package
rem requires that perl and unzip are in your path
rem %1 = working directory
rem %2 = package name (without extension)
rem %3 = version
rem %4 = options (release|full, default release)
rem %5 = compiler (vs2003, optional, default vs2003)
rem %6 = NSIS directory (optional)
rem ***********************************************

set TMP_QTVERSION=%3
set TMP_QTCONFIG=%4
set TMP_COMPILER=%5
set TMP_NSISDIR=%6
set QT_WINBINARY_STATUS=ok

rem We need to compile qt in a long directory, since we want to patch the pdb files.
set TMP_BUILDDIR=__________________________________________________PADDING__________________________________________________

if "%4"=="" set TMP_QTCONFIG=release
if "%5"=="" set TMP_COMPILER=vs2003
if "%6"=="" set TMP_NSISDIR=C:\Program Files\NSIS

call :ExtractAndCopy %1 %2
if "%QT_WINBINARY_STATUS%"=="failed" goto End
call :Compile %1
if "%QT_WINBINARY_STATUS%"=="failed" goto End
call :OrganizeClean %1
if "%QT_WINBINARY_STATUS%"=="failed" goto End
call :CreateNSISPackage %1 %2
if "%QT_WINBINARY_STATUS%"=="failed" goto End

echo Done!
goto END



rem ***********************************************
rem Organize the binary package
rem %1 = working directory
rem ***********************************************
:OrganizeClean
echo Organizing the binary package...
cd %1\clean

if "%TMP_QTCONFIG%"=="release" goto ReleaseConfig
echo - Copying symbols
xcopy /Q /I %1\%TMP_BUILDDIR%\lib\*.pdb %1\clean\bin >> %1\log.txt
xcopy /Q /I %1\%TMP_BUILDDIR%\src\winmain\*.pdb %1\clean\src\winmain\ >> %1\log.txt
echo - Copying .moc files
echo release_shared > %1\moc_exclude
cd %1\%TMP_BUILDDIR%\src
xcopy /Q /S /I /EXCLUDE:%1\moc_exclude moc_*.cpp %1\clean\src\ >> %1\log.txt
xcopy /Q /S /I /EXCLUDE:%1\moc_exclude *.moc %1\clean\src\ >> %1\log.txt
cd %1\clean
goto CopyFiles

:ReleaseConfig
echo - Regenerate include directory
rd /S /Q include
set TMP_QTDIR=%QTDIR%
set QTDIR=%1\clean
perl %1\clean\bin\syncqt -copy >> %1\log.txt 2>&1
set QTDIR=%TMP_QTDIR%
set TMP_QTDIR=
echo - Removing source dir in clean
rd /S /Q src

:CopyFiles
echo - Copying all .exe files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.exe %1\clean\ >> %1\log.txt
rem Delete activeqt examples!
del /S /Q %1\clean\examples\activeqt\*.exe >> %1\log.txt

echo - Copying dlls
xcopy /Q /I %1\%TMP_BUILDDIR%\lib\*.dll %1\clean\bin >> %1\log.txt

echo - Copying libs
xcopy /Q /I %1\%TMP_BUILDDIR%\lib\*.lib %1\clean\lib >> %1\log.txt

echo - Copying plugins
cd %1\%TMP_BUILDDIR%\plugins
xcopy /Q /I /S *.dll %1\clean\plugins\ >> %1\log.txt

echo - Copying .prl files
cd %1\%TMP_BUILDDIR%\lib
xcopy /Q /I *.prl %1\clean\lib\ >> %1\log.txt

cd %1\%TMP_BUILDDIR%

echo - Copying additional files
echo   * qconfig.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\global\qconfig.h %1\clean\include\QtCore\ >> %1\log.txt
mkdir %1\clean\include\QtCore\arch
echo   * qatomic.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\arch\windows\arch\qatomic.h %1\clean\include\QtCore\arch\ >> %1\log.txt
echo   * qconfig.pri
xcopy /Q %1\%TMP_BUILDDIR%\mkspecs\qconfig.pri %1\clean\mkspecs\ >> %1\log.txt

goto :EOF



rem ***********************************************
rem Create NSIS package
rem %1 = working directory
rem %2 = package name
rem ***********************************************
:CreateNSISPackage
echo Creating NSIS Package
cd %1

echo - Updating the NSIS Qt Plugin
if exist "%TMP_NSISDIR%\Plugins\qtnsisext.dll" del /Q "%TMP_NSISDIR%\Plugins\qtnsisext.dll"
xcopy /Q "%1\qtnsisext\qtnsisext.dll" "%TMP_NSISDIR%\Plugins\" >> %1\log.txt
if not %errorlevel%==0 goto FAILED

if "%TMP_QTCONFIG%"=="release" goto ReleaseNSIS
echo - Running makensis (full)
"%TMP_NSISDIR%\makensis.exe" /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="%TMP_COMPILER%" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2-%TMP_COMPILER%" installscriptwin.nsi >> %1\log.txt
if not %errorlevel%==0 goto FAILED
goto EndNSIS

:ReleaseNSIS
echo - Running makensis (release)
"%TMP_NSISDIR%\makensis.exe" /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2" installscriptwin.nsi >> %1\log.txt
if not %errorlevel%==0 goto FAILED

:EndNSIS
goto :EOF



rem ***********************************************
rem Compile
rem %1 = working directory
rem ***********************************************
:Compile
echo Compiling (%TMP_COMPILER%)...
call :SetEnv %1
cd %QTDIR%

echo - Creating license file
type LICENSE.TROLL > LICENSE.TROLL

echo - Running configure...
configure -release -plugin-sql-sqlite -plugin-sql-odbc -qt-style-windowsxp -qt-libpng -qt-libjpeg 1>>log.txt >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo - Building...
echo   * Qt (%TMP_QTCONFIG%)

if "%TMP_QTCONFIG%"=="release" goto ReleaseBuild

nmake sub-src-all >> %1\log.txt 2>&1
goto BuildAdditional

:ReleaseBuild
nmake sub-src >> %1\log.txt 2>&1

:BuildAdditional
echo   * Tools (release)
nmake sub-tools >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo   * Demos (release)
nmake sub-demos >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo   * ActiveQt (releases)
cd %QTDIR%\extensions\activeqt
nmake >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

cd %QTDIR%

echo   * Examples (release)
nmake sub-examples >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

cd %QTDIR%

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

if "%TMP_COMPILER%"=="vs2003" goto VS2003Env
if "%TMP_COMPILER%"=="vs2002" goto VS2002Env
if "%TMP_COMPILER%"=="vc60" goto VC60Env
goto :EOF

:VS2003Env
set QMAKESPEC=win32-msvc.net
call "%VS71COMNTOOLS%vsvars32.bat" >> %1\log.txt
goto :EOF

:VS2002Env
set QMAKESPEC=win32-msvc.net
call %VSCOMNTOOLS%vsvars32.bat >> %1\log.txt
goto :EOF

:VC60Env
set QMAKESPEC=win32-msvc
rem Hardcoded :(
call "%ProgramFiles%\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" >> %1\log.txt
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
if exist %TMP_BUILDDIR% rd /S /Q %TMP_BUILDDIR%
if exist %2 rd /S /Q %2

mkdir clean
if not %errorlevel%==0 goto FAILED

echo - Extracting package
unzip %2.zip >> log.txt
if not %errorlevel%==0 goto FAILED
rename %2 %TMP_BUILDDIR%

echo - Copying directories
xcopy /S /Q %TMP_BUILDDIR% clean >> %1\log.txt
if not %errorlevel%==0 goto FAILED
goto :EOF



:FAILED
echo Failed!
set QT_WINBINARY_STATUS=failed

:END
cd %1
set OLD_PATH=
set OLD_QMAKESPEC=
set OLD_QTDIR=
set OLD_INCLUDE=
set OLD_LIB=
set TMP_QTVERSION=
set TMP_QTCONFIG=
set TMP_COMPILER=
set TMP_BUILDDIR=
set TMP_NSISDIR=


