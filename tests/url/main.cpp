// #include "qurl.h"
// #include "qurlinfo.h"
#include <qapplication.h>
#include "qfiledialog.h"

// class ReadDir : public QObject
// {
//     Q_OBJECT
    
// public:
//     ReadDir( const QUrl &u ) : url( u ) {}
    
// public slots:
//     add( const QUrlInfo &i ) {
// 	qDebug( "add: %s %s", i.name().latin1() , i.makeUrl( url ).latin1()  );
//     }
    
// protected:
//     QUrl url;
    
// };

int main( int argc, char* argv[]  )
{
//     QUrl u( "ftp://reggie:blablub@ftp.troll.no/home/reggie/a_woooooohhhhhnsinn.html#woansinn" );

//     qDebug( "URL: ftp://reggie:blablub@ftp.troll.no/home/reggie/a_woooooohhhhhnsinn.html#woansinn\n\n"
// 	    "protocol: %s\n"
// 	    "user: %s\n"
// 	    "passwd: %s\n"
// 	    "host: %s\n"
// 	    "path: %s\n"
// 	    "ref: %s\n",
// 	    u.protocol().latin1(),
// 	    u.user().latin1(),
// 	    u.pass().latin1(),
// 	    u.host().latin1(),
// 	    u.path().latin1(),
// 	    u.ref().latin1() );

//     QString arg = argv[ 1 ];
//     qDebug( "%s", arg.latin1() );
//     QUrl u( arg );

//     qDebug( "URL: %s\n\n"
// 	    "protocol: %s\n"
// 	    "user: %s\n"
// 	    "passwd: %s\n"
// 	    "host: %s\n"
// 	    "path: %s\n"
// 	    "ref: %s\n",
// 	    arg.latin1(),
// 	    u.protocol().latin1(),
// 	    u.user().latin1(),
// 	    u.pass().latin1(),
// 	    u.host().latin1(),
// 	    u.path().latin1(),
// 	    u.ref().latin1() );

//     ReadDir rd( u );
//     QObject::connect( &u, SIGNAL( entry( const QUrlInfo & ) ),
//  		      &rd, SLOT( add( const QUrlInfo & ) ) );
    
//     u.listEntries();

    QApplication a( argc, argv );

//     QFileDialog *f = new QFileDialog( 0, 0, TRUE );
//     f->resize( 600, 400 );
//     a.setMainWidget( f );
//     f->exec();

    QFileDialog::getOpenFileName();
    
    a.exec();
}

// #include "main.moc"
