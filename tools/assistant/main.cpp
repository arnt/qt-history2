#include <qapplication.h>
#include "mainwindow.h"
#include <qserversocket.h>
#include <qsocket.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qsettings.h>
#include "assistant.h"

const int server_port = 7358;


static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};


class Socket : public QSocket
{
    Q_OBJECT

public:
    Socket( QObject *parent );
    ~Socket();

    void start();

private slots:
    void startOwnWindow();
    void sendFile();

};


class ServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    ServerSocket( MainWindow *mw );
    ~ServerSocket();
    void newConnection( int );

private slots:
    void dataReceived();

private:
    QPtrList<QSocket> connections;
    MainWindow *mainWindow;

};


Socket::Socket( QObject *parent )
    : QSocket( parent )
{
    connect( this, SIGNAL( error( int ) ), this, SLOT( startOwnWindow() ) );
    connect( this, SIGNAL( connected() ), this, SLOT( sendFile() ) );
    connect( this, SIGNAL( bytesWritten( int ) ), qApp, SLOT( quit() ) );
}

Socket::~Socket()
{
}

void Socket::start()
{
    connectToHost( "127.0.0.1", server_port );
}

void Socket::startOwnWindow()
{
    MainWindow * mw = new MainWindow;
    mw->setIcon( logo_xpm );

    QString keybase("/Qt Assistant/3.0/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    if ( config.readBoolEntry( keybase  + "GeometryMaximized", FALSE ) )
	mw->showMaximized();
    else
	mw->show();

    QString s = qApp->argv()[ 1 ];
    if ( s.left( 2 ) == "d:" )
	s.remove( 0, 2 );
    if ( !s.isEmpty() )
	mw->showLink( s, "" );
    (void)new ServerSocket( mw );
}

void Socket::sendFile()
{
    QString s = qApp->argv()[ 1 ];
    if ( s.left( 2 ) == "d:" ) {
	s.remove( 0, 2 );
	s += "\n";
	writeBlock( s.latin1(), s.length() );
    } else {
	s += "\n";
	writeBlock( s.latin1(), s.length() );
    }
}




ServerSocket::ServerSocket( MainWindow *mw )
    : QServerSocket( server_port, 0, mw ), mainWindow( mw )
{
    connections.setAutoDelete( TRUE );
}

ServerSocket::~ServerSocket()
{
    for ( QSocket *s = connections.first(); s; s = connections.next() )
	s->close();
}

void ServerSocket::newConnection( int socket )
{
    QSocket *s = new QSocket( this );
    s->setSocket( socket );
    connect( s, SIGNAL( readyRead() ),
	     this, SLOT( dataReceived() ) );
    connections.append( s );
}

void ServerSocket::dataReceived()
{
    QSocket *s = (QSocket*)sender();
    if ( !s || !s->inherits( "QSocket" ) )
	return;
    if ( !s->canReadLine() )
	return;
    QString line = s->readLine();
    line = line.simplifyWhiteSpace();
#if 0
    if ( line == "assistant" ) {
	Assistant *a = new Assistant( 0 );
	a->resize( 300, 600 );
	a->show();
	return;
    }
#endif
    mainWindow->showLink( line, "" );
    mainWindow->show();
    mainWindow->raise();
}

#ifdef Q_OS_MACX
#include <stdlib.h>
#include <qdir.h>
#endif

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

#ifdef Q_OS_MACX
    QString qdir = QDir::cleanDirPath(QDir::currentDirPath() + QDir::separator() +
				      ".." + QDir::separator());
    setenv("QTDIR", qdir.latin1(), 0);
#endif

    if ( argc == 1 || QString( argv[1] ).left( 2 ) != "d:" ) {
	MainWindow * mw = new MainWindow(0, "Assistant" );
	mw->setIcon( logo_xpm );

	QString keybase("/Qt Assistant/3.1/");
	QSettings config;
	config.insertSearchPath( QSettings::Windows, "/Trolltech" );
	if ( config.readBoolEntry( keybase  + "GeometryMaximized", FALSE ) )
	    mw->showMaximized();
	else
	    mw->show();

	QString s;
	if ( argc > 1 )
	    s = QString( argv[1] );
	if ( s.left( 2 ) == "d:" )
	    s.remove( 0, 2 );
	if ( !s.isEmpty() )
	    mw->showLink( s, "" );
    } else {
	Socket *s = new Socket( 0 );
	s->start();
    }
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}

#include "main.moc"
