#include "finger.h"

#include <qapplication.h>

#include <qlayout.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtextstream.h>
#include <qpushbutton.h>

#include <qsocket.h>


Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    input = new QLineEdit( this, "user and stuff" );
    input->setText( "root@lupinella.troll.no" );
    input->setFocus();
    connect( input, SIGNAL(returnPressed()), SLOT(makeConnection()) );

    QPushButton * quit = new QPushButton( "Quit", this );
    quit->setFixedSize( quit->sizeHint() );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    output = new QMultiLineEdit( this );
    output->setReadOnly( TRUE );

    QBoxLayout * l = new QBoxLayout( this, QBoxLayout::Up );
    l->addWidget( output, 1 );
    l->addSpacing( 3 );

    QBoxLayout * l2 = new QBoxLayout( QBoxLayout::LeftToRight );
    l->addLayout( l2, 0 );

    l2->addWidget( input, 1 );
    l2->addSpacing( 3 );
    l2->addWidget( quit );

    socket = 0;
    s = 0;
}


void Main::makeConnection()
{
    QString user( input->text() );
    if ( !user.contains( '@' ) ) {
	output->insertLine( "Syntax error" );
	return;
    }

    QString host( user );
    host = host.mid( user.find( '@' )+1, user.length() );
    user = user.left( user.find( '@' ) );

    output->insertLine( QString( "\n\nConnecting to host " ) + host );

    delete socket;
    socket = new QSocket( this, "finger" );
    socket->connectToHost( host, 79 );
    socket->setMode( QSocket::Ascii );
    connect( socket, SIGNAL(readyRead()),
	     this, SLOT(dataArrived()) );
    connect( socket, SIGNAL(closed()),
	     this, SLOT(timeToUpdate()) );
    delete s;
    s = new QTextStream( socket );
    s->setEncoding( QTextStream::Latin1 );
    output->insertLine( QString( "Asking about user " ) + user );

    *s << user << "\r\n";
}


void Main::dataArrived()
{
    while ( !s->eof() && socket->canReadLine() )
	output->insertLine( s->readLine() );
}


void Main::timeToUpdate()
{
    output->insertLine( "\n\nConnection closed" );

    delete socket;
    socket = 0;
    delete s;
    s = 0;
}


main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);

    Main m;
    m.resize( 400, 600 );
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
