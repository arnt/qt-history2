#include "guicat.h"

#include <qpushbutton.h>
#include <qapplication.h>
#include <iostream.h>
#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

GuiCat::GuiCat( QWidget *p ) : QVBox( p )
{
    QPushButton *pb;

    pb = new QPushButton( "Read One Byte", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(readOneByte()) );

    pb = new QPushButton( "Read 16*4096 Byte", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(readMuch()) );

    pb = new QPushButton( "Write to Stdout", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(writeStdout()) );

    pb = new QPushButton( "Write to Stderr", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(writeStderr()) );

    pb = new QPushButton( "Close Stdin", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(closeStdin()) );

    pb = new QPushButton( "Close Stdout", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(closeStdout()) );

    pb = new QPushButton( "Close Stderr", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(closeStderr()) );

    pb = new QPushButton( "Quit", this );
    connect( pb, SIGNAL(clicked()), qApp, SLOT(quit()) );
}


void GuiCat::readOneByte()
{
    char ch;
    if( !cin.eof() ) {
	cin >> ch;
	buf.resize( buf.size() + 1 );
	buf.at( buf.size()-1 ) = ch;
    }
}


void GuiCat::readMuch()
{
    char ch;
    int i = 0;
    while( !cin.eof() ) {
	cin >> ch;
	i++;
	if ( i >= 16*4096 ) {
	    buf.resize( buf.size() + 1 );
	    buf.at( buf.size()-1 ) = ch;
	    break;
	}
    }
}


void GuiCat::writeStdout()
{
    for ( uint i=0; i<buf.size(); i++ )
	cout << buf.at( i );
    cout.flush();
    buf.resize( 0 );
}


void GuiCat::writeStderr()
{
    for ( uint i=0; i<buf.size(); i++ )
	cerr << buf.at( i );
    cerr.flush();
    buf.resize( 0 );
}


void GuiCat::closeStdin()
{
#if defined(Q_OS_UNIX)
    ::close( STDIN_FILENO );
#endif
}


void GuiCat::closeStdout()
{
#if defined(Q_OS_UNIX)
    ::close( STDOUT_FILENO );
#endif
}


void GuiCat::closeStderr()
{
#if defined(Q_OS_UNIX)
    ::close( STDERR_FILENO );
#endif
}
