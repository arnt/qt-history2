%QTDIR%\configure.exe -no-stl -release -no-zlib -no-libjpeg -static -no-qt3support 

:: windows nmake does not understand shortcut arguments so therefore everything
:: has to be done by hand. Additionally environment variables cannot be stripped
:: because of that, the following paths are hardcoded :(
:: nmake sub-tools-moc sub-corelib sub-network
E:
cd \dev\qt\4.1.4\src\tools\moc
nmake
cd \dev\qt\4.1.4\src\corelib
nmake
cd \dev\qt\4.1.4\src\network
nmake
