#include <qapplication.h>
#include "generatordlgimpl.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    GeneratorDlgImpl dlg;

    dlg.exec();

/*
    QDir distdir;
    QStringList dirList;
    QStringList fileList;
    QString distname;

    if( ( argc < 2 ) || ( argc > 3 ) ) {
	printf( "Usage: %s <dist> [makecommand]\n\n", argv[ 0 ] );
	return 1;
    }
    if( argc == 3 )
    {
	makeCmd = QString( argv[ 2 ] );
    }
    distname = QString( "dist" ) + argv[ 1 ];

    distdir.mkdir( distname );

    fileList << QString( getenv( "QTDIR" ) ) + "\\dist\\commercial\\LICENSE";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\INSTALL_DONE.TXT";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_msvc.bat";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_borland.bat";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_gcc.bat";
    generateFileArchive( distname + "\\sys.arq", fileList );

    fileList.clear();
    fileList << QString( getenv( "QTDIR" ) ) + "\\Makefile";
    fileList << QString( getenv( "QTDIR" ) ) + "\\configure.exe";
    generateFileArchive( distname + "\\build.arq", fileList );

    fileList.clear();
    fileList << QString( getenv( "QTDIR" ) ) + "\\bin\\quninstall.exe";
    generateFileArchive( distname + "\\uninstall.arq", fileList );

    dirList << "dist\\win\\" << "src" << "include" << "mkspecs" << "plugins" << "qmake" << "tmake" << "tools";
    generateArchive( distname + "\\qt.arq", dirList );

  buildInstaller( argv[ 1 ] );

*/
    return 0;
}
