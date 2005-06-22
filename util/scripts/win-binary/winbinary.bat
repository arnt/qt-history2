@echo off
rem ***********************************************
rem creates a binary package from a src package
rem requires that perl and unzip are in your path
rem %1 = working directory
rem %2 = package name (without extension)
rem %3 = version
rem %4 = options (opensource|commercial, default commercial)
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

if "%4"=="" set TMP_QTCONFIG=commercial
if "%5"=="" set TMP_COMPILER=vs2003
if "%6"=="" set TMP_NSISDIR=%ProgramFiles%\NSIS

call :ExtractAndCopy %1 %2
if "%QT_WINBINARY_STATUS%"=="failed" goto End
call :Compile %1
if "%QT_WINBINARY_STATUS%"=="failed" goto End


if "%TMP_QTCONFIG%"=="eval" goto OrganizeEvaluation
call :OrganizeClean %1
goto OrganizeDone

:OrganizeEvaluation
call :OrganizeEvalClean %1
goto OrganizeDone

:OrganizeDone
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

if "%TMP_COMPILER%"=="mingw" goto MinGWConfig
echo - Copying symbols
xcopy /Q /I %1\%TMP_BUILDDIR%\lib\*.pdb %1\clean\bin >> %1\log.txt
xcopy /Q /I %1\%TMP_BUILDDIR%\src\winmain\*.pdb %1\clean\src\winmain\ >> %1\log.txt
echo - Copying .moc files
echo release_shared > %1\moc_exclude
cd %1\%TMP_BUILDDIR%\src
xcopy /Q /S /I /EXCLUDE:%1\moc_exclude moc_*.cpp %1\clean\src\ >> %1\log.txt
xcopy /Q /S /I /EXCLUDE:%1\moc_exclude *.moc %1\clean\src\ >> %1\log.txt

cd %1\%TMP_BUILDDIR%
echo - Copying additional files
echo   * qconfig.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\global\qconfig.h %1\clean\src\corelib\global\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\QtCore\qconfig.h %1\clean\include\QtCore\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\Qt\qconfig.h %1\clean\include\Qt\ >> %1\log.txt
echo   * qatomic.h
xcopy /Q %1\%TMP_BUILDDIR%\include\Qt\arch\qatomic.h %1\clean\include\Qt\arch\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\QtCore\arch\qatomic.h %1\clean\include\QtCore\arch\ >> %1\log.txt
echo   * qconfig.pri
xcopy /Q %1\%TMP_BUILDDIR%\mkspecs\qconfig.pri %1\clean\mkspecs\ >> %1\log.txt
cd %1\clean

echo - Copying all .lib files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.lib %1\clean\ >> %1\log.txt
rem Delete activeqt
del /S /Q %1\clean\examples\activeqt\*.lib >> %1\log.txt
cd %1\clean

goto CopyFiles

:MinGWConfig
cd %1\%TMP_BUILDDIR%
echo - Copying configuration files
echo   * qconfig.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\global\qconfig.h %1\clean\src\corelib\global\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\QtCore\qconfig.h %1\clean\include\QtCore\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\Qt\qconfig.h %1\clean\include\Qt\ >> %1\log.txt
echo   * qatomic.h
xcopy /Q %1\%TMP_BUILDDIR%\include\Qt\arch\qatomic.h %1\clean\include\Qt\arch\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\include\QtCore\arch\qatomic.h %1\clean\include\QtCore\arch\ >> %1\log.txt
echo   * qconfig.pri
xcopy /Q %1\%TMP_BUILDDIR%\mkspecs\qconfig.pri %1\clean\mkspecs\ >> %1\log.txt
cd %1\clean

echo - Copying all .a files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.a %1\clean\ >> %1\log.txt
rem Delete activeqt
del /S /Q %1\clean\examples\activeqt\*.a >> %1\log.txt
cd %1\clean


:CopyFiles
echo - Copying all .exe files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.exe %1\clean\ >> %1\log.txt
rem Delete activeqt examples!
del /S /Q %1\clean\examples\activeqt\*.exe >> %1\log.txt

echo - Copying all .dll files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.dll %1\clean\ >> %1\log.txt
rem Delete activeqt and dlls in /lib
del /S /Q %1\clean\examples\activeqt\*.dll >> %1\log.txt
del /S /Q %1\clean\lib\*.dll >> %1\log.txt

echo - Copying .prl files
cd %1\%TMP_BUILDDIR%\lib
xcopy /Q /I *.prl %1\clean\lib\ >> %1\log.txt

echo - Copying .qmake.cache
cd %1\%TMP_BUILDDIR%\
xcopy /Q /I .qmake.cache %1\clean\ >> %1\log.txt

cd %1\%TMP_BUILDDIR%
goto :EOF



rem ***********************************************
rem Organize the binary package (Evaluation)
rem %1 = working directory
rem ***********************************************
:OrganizeEvalClean
echo Organizing the binary package (Evaluation)...
cd %1\clean

echo - Regenerate include directory
rd /S /Q include
set TMP_QTDIR=%QTDIR%
set QTDIR=%1\clean
perl %1\clean\bin\syncqt -copy >> %1\log.txt 2>&1
set QTDIR=%TMP_QTDIR%
set TMP_QTDIR=

echo - Copying required files
echo - * Phrase Books
mkdir %1\clean\phrasebooks
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\tools\linguist\phrasebooks\*.qph %1\clean\phrasebooks\ >> %1\log.txt
echo - * Porting tool .xml file
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\tools\porting\src\*.xml %1\clean\ >> %1\log.txt

echo - Copying integration setup
copy /Y \\Lupinella\tmp\thomas\setup_vs2003_*.exe %1\

echo - Copying additional files
echo   * qconfig.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\global\qconfig.h %1\clean\include\Qt\ >> %1\log.txt
mkdir %1\clean\include\QtCore\arch
mkdir %1\clean\include\Qt\arch
echo   * qatomic.h
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\arch\windows\arch\qatomic.h %1\clean\include\Qt\arch\ >> %1\log.txt
xcopy /Q %1\%TMP_BUILDDIR%\src\corelib\arch\windows\arch\qatomic.h %1\clean\include\QtCore\arch\ >> %1\log.txt
echo   * qconfig.pri
xcopy /Q %1\%TMP_BUILDDIR%\mkspecs\qconfig.pri %1\clean\mkspecs\ >> %1\log.txt
cd %1\clean

echo - Copying symbols
mkdir %1\clean\src\winmain
xcopy /Q /I %1\%TMP_BUILDDIR%\lib\*.pdb %1\clean\bin >> %1\log.txt
xcopy /Q /I %1\%TMP_BUILDDIR%\src\winmain\*.pdb %1\clean\src\winmain\ >> %1\log.txt

echo - Copying all .lib files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.lib %1\clean\ >> %1\log.txt
rem Delete activeqt
del /S /Q %1\clean\examples\activeqt\*.lib >> %1\log.txt
cd %1\clean

echo - Copying all .exe files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.exe %1\clean\ >> %1\log.txt
rem Delete activeqt examples!
del /S /Q %1\clean\examples\activeqt\*.exe >> %1\log.txt

echo - Copying all .dll files
xcopy /Q /Y /I /S %1\%TMP_BUILDDIR%\*.dll %1\clean\ >> %1\log.txt
rem Delete activeqt and dlls in /lib
del /S /Q %1\clean\examples\activeqt\*.dll >> %1\log.txt
del /S /Q %1\clean\lib\*.dll >> %1\log.txt

echo - Copying .prl files
cd %1\%TMP_BUILDDIR%\lib
xcopy /Q /I *.prl %1\clean\lib\ >> %1\log.txt

echo - Copying .qmake.cache
cd %1\%TMP_BUILDDIR%\
xcopy /Q /I .qmake.cache %1\clean\ >> %1\log.txt

echo - Removing source directory
cd %1\clean
rd /S /Q src
cd %1\%TMP_BUILDDIR%

echo - Removing tools directory
cd %1\clean
rd /S /Q tools
cd %1\%TMP_BUILDDIR%

echo - Removing qmake directory
cd %1\clean
rd /S /Q qmake

echo - Removing extensions directory
cd %1\clean
rd /S /Q extensions

cd %1\%TMP_BUILDDIR%
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

echo - Updating the NSIS MinGW Plugin
if exist "%TMP_NSISDIR%\Plugins\gwnsisext.dll" del /Q "%TMP_NSISDIR%\Plugins\gwnsisext.dll"
xcopy /Q "%1\gwnsisext\gwnsisext.dll" "%TMP_NSISDIR%\Plugins\" >> %1\log.txt
if not %errorlevel%==0 goto FAILED

echo - Running makensis (%TMP_QTCONFIG%)

if "%TMP_QTCONFIG%"=="opensource" goto OpenSourceNSIS
if "%TMP_COMPILER%"=="mingw" goto MinGWNSIS
if "%TMP_QTCONFIG%"=="eval" goto QtEvaluation
"%TMP_NSISDIR%\makensis.exe" /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="%TMP_COMPILER%" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2-%TMP_COMPILER%" installscriptwin.nsi >> %1\log.txt
goto EndNSIS

:MinGWNSIS
"%TMP_NSISDIR%\makensis.exe" /DUSEMINGW /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="%TMP_COMPILER%" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2-%TMP_COMPILER%" installscriptwin.nsi >> %1\log.txt
goto EndNSIS

:OpenSourceNSIS
"%TMP_NSISDIR%\makensis.exe" /DQTOPENSOURCE /DUSEMINGW /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="%TMP_COMPILER%" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2-%TMP_COMPILER%" installscriptwin.nsi >> %1\log.txt
goto EndNSIS

:QtEvaluation
"%TMP_NSISDIR%\makensis.exe" /DQTEVALUATION /DQTBUILDDIR="%1\%TMP_BUILDDIR%" /DFORCE_MAKESPEC="%TMP_COMPILER%" /DPRODUCT_VERSION="%TMP_QTVERSION%" /DPACKAGEDIR="%1\clean" /DDISTNAME="%2-%TMP_COMPILER%" installscriptwin.nsi >> %1\log.txt
goto EndNSIS

:EndNSIS
if not %errorlevel%==0 goto FAILED
goto :EOF



rem ***********************************************
rem Compile
rem %1 = working directory
rem ***********************************************
:Compile
echo Compiling (%TMP_COMPILER%)...
call :SetEnv %1
cd %QTDIR%

if "%TMP_COMPILER%"=="mingw" goto MinGWBuild

set EVALDEFINE= 
if "%TMP_QTCONFIG%"=="eval" set EVALDEFINE=-D QT_EVAL

echo - Running configure...
configure -confirm-license -release -plugin-sql-sqlite -plugin-sql-odbc -qt-style-windowsxp -qt-libpng -qt-libjpeg %EVALDEFINE% 1>>log.txt >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo - Building (%TMP_QTCONFIG%)
nmake sub-src-all >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake sub-tools-all >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake sub-extensions-all >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake sub-demos-make_first >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake sub-examples-make_first >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

cd %QTDIR%\src\plugins\sqldrivers\mysql
qmake "INCLUDEPATH+=/sql/include/mysql" "LIBS+=libmysqld.lib ws2_32.lib advapi32.lib" >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake debug >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
qmake "INCLUDEPATH+=/sql/include/mysql" "LIBS+=libmysql.lib ws2_32.lib advapi32.lib" >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake release >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

cd %QTDIR%\src\plugins\sqldrivers\psql
qmake "INCLUDEPATH+=/sql/include/psql" "LIBS+=libpqd.lib shell32.lib" >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake debug >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
qmake "INCLUDEPATH+=/sql/include/psql" "LIBS+=libpq.lib shell32.lib" >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED
nmake release >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

cd %QTDIR%

goto DoneBuilding
:MinGWBuild

echo - Running configure...
configure -confirm-license -release -plugin-sql-sqlite -plugin-sql-odbc -qt-style-windowsxp -qt-libpng -qt-libjpeg 1>>log.txt >> %1\log.txt 2>&1
if not %errorlevel%==0 goto FAILED

echo - Building (%TMP_QTCONFIG%)
mingw32-make
if not %errorlevel%==0 goto FAILED
cd %QTDIR%

:DoneBuilding
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
if "%TMP_COMPILER%"=="vs2005" goto VS2005Env
if "%TMP_COMPILER%"=="vc60" goto VC60Env
if "%TMP_COMPILER%"=="mingw" goto MinGWEnv
goto :EOF

:VS2003Env
set QMAKESPEC=win32-msvc.net
call "%VS71COMNTOOLS%vsvars32.bat" >> %1\log.txt
set TMP_MAKE=nmake
goto :EOF

:VS2002Env
set QMAKESPEC=win32-msvc.net
call "%VSCOMNTOOLS%vsvars32.bat" >> %1\log.txt
set TMP_MAKE=nmake
goto :EOF

:VS2005Env
set QMAKESPEC=win32-msvc2005
call "%VS80COMNTOOLS%vsvars32.bat" >> %1\log.txt
set TMP_MAKE=nmake
goto :EOF

:VC60Env
set QMAKESPEC=win32-msvc
rem Hardcoded :(
call "%ProgramFiles%\Microsoft Visual Studio\VC98\Bin\vcvars32.bat" >> %1\log.txt
set TMP_MAKE=nmake
goto :EOF

:MinGWEnv
set QMAKESPEC=win32-g++
rem Hardcoded :(
set PATH=C:\MinGW\bin;%PATH%
set TMP_MAKE=mingw32-make
set MINGW_IN_SHELL=1
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
