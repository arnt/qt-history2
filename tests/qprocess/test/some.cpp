#include <qapplication.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "../qprocess.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QVBox vb;

#if 0
    QProcess proc( "aclock" );
#if defined(UNIX)
    proc.setPath( QDir("../../../examples/aclock/") );
#else
    proc.setPath( QDir("../../../examples/aclock/Debug/") );
#endif
    proc.start();
#endif

#if 1
#if defined(UNIX)
    QProcess proc( "cat" );
#else
    QDir dir( "cat/Debug" );
    QProcess proc( dir.absFilePath("cat.exe").latin1() );
#endif

    QLineEdit *in = new QLineEdit( &vb );
    QLabel *out = new QLabel( &vb );
    QPushButton *close = new QPushButton( "Close Stdin", &vb );

    QObject::connect( in, SIGNAL(textChanged(const QString&)),
	    &proc, SLOT(dataStdin(const QString&)) );
    QObject::connect( close, SIGNAL(clicked()),
	    &proc, SLOT(closeStdin()) );
    QObject::connect( &proc, SIGNAL(dataStdout(const QString&)),
	    out, SLOT(setText(const QString&)) );

    if ( !proc.start() ) {
	return -1 ;
    }
#endif

#if 0
    QProcess proc1( "echo", "b\na\nc\n" );
    proc1.setPath( QDir("/bin/") );

    QProcess proc2( "sort" );
    proc2.setPath( QDir("/usr/bin/") );

    QLabel *out = new QLabel( &vb );

    QObject::connect( &proc1, SIGNAL(dataStdout(const QString&)),
	    &proc2, SLOT(dataStdin(const QString&)) );
    QObject::connect( &proc2, SIGNAL(dataStdout(const QString&)),
	    out, SLOT(setText(const QString&)) );
    QObject::connect( &proc1, SIGNAL(processExited()),
	    &proc2, SLOT(closeStdin()) );

    proc1.start();
    proc2.start();
#endif

    QPushButton quit( "Quit", &vb );
    QObject::connect( &quit, SIGNAL(clicked()),
	    &a, SLOT(quit()) );

    a.setMainWidget( &vb );
    vb.show();
    return a.exec();
}
