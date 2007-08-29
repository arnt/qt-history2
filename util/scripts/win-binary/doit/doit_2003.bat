call "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32.bat"
c:
cd \installer\win
call iwmake.bat vs2003-commercial >>out.txt
call iwmake.bat vs2003-eval >>out.txt
pscp -batch -i c:\key1.ppk c:\iwmake\*.exe qt@ares:public_html/packages

