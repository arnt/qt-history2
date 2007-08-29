set PATH=C:\MinGW\bin;%PATH%
c:
cd \installer\win
call iwmake.bat mingw-opensource >>out.txt
pscp -batch -i c:\key1.ppk c:\iwmake\*.exe qt@ares:public_html/packages

