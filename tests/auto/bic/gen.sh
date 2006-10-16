#!/bin/sh

if [ "$#" -lt 2 ]; then
    echo "$0 - Generates reference files for b/c autotest"
    echo "Usage: $0 [module|-all] [platform]"
    echo "Examples: $0 -all 4.1.0.macx-gcc-ppc32"
    echo "          $0 QtGui 4.0.0.linux-gcc-ia32"
    exit 1
fi

if [ "$1" = "-all" ]; then
    modules="QtCore QtGui QtSql QtSvg QtNetwork QtXml QtOpenGL Qt3Support QtTest QtDBus"
else
    modules="$1"
fi

for module in $modules; do
    echo "#include <$module/$module>" >test.cpp
    g++ -c -I$QTDIR/include -DQT_NO_STL -DQT3_SUPPORT -fdump-class-hierarchy test.cpp
    mv test.cpp*.class $module.$2.txt
done

