call "C:\Program Files\Microsoft Visual Studio .NET 8\VC\bin\vcvars32.bat"
c:
cd \installer\win
call iwmake.bat vs2005-commercial >>out.txt
call iwmake.bat vs2005-eval >>out.txt
pscp -batch -i c:\key1.ppk c:\iwmake\*.exe qt@ares:public_html/packages

