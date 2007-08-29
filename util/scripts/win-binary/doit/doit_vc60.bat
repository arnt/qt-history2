call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat"
set INCLUDE=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\include;%INCLUDE%
set LIB=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\lib;%LIB%
c:
cd \installer\win
call iwmake.bat vc60-commercial >>out.txt
call iwmake.bat vc60-eval >>out.txt
pscp -batch -i c:\key1.ppk c:\iwmake\*.exe qt@ares:public_html/packages

