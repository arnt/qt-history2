@echo off
echo Building QActiveX library...
qmake
nmake %1
echo Building examples...
cd examples
qmake
nmake %1
cd ..
