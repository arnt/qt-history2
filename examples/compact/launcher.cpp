#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>


#define PEN_INPUT
#ifdef PEN_INPUT
#include "input_pen.h"
#endif

#include "keyboard.h"
#include "unikeyboard.h"

#include <qwindowsystem_qws.h>




#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __MIPSEL__
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


struct {
    const char* label;
    const char* dir;
    const char* file;
    const char* arg1;
    const char* arg2;
} command[] = {
    //    { "Info Kiosk - MPEGs", "( cd ../kiosk; exec ./kiosk %1)" },
    { "Help Text Browser", "../helpviewer", "helpviewer", 0, 0 },
    { "Canvas - alpha-blending", "../canvas", "canvas", 0, 0 },
    { "Text Editor", "../qwerty", "qwerty", "unicode.txt", 0 },
    { "Scribble Editor", "../scribble", "scribble", 0, 0 },
    { "Internationalization", "../i18n", "i18n", 0, 0 },
    { "Magnifier", "../qmag", "qmag", "-geometry",  "100x100" },
    { 0, 0, 0, 0, 0 }
};


class Clock : public QLabel
{
public:
    Clock( QWidget *parent );
protected:
    void timerEvent( QTimerEvent * );
};

Clock::Clock( QWidget *parent )
 : QLabel( parent ) 
{
    timerEvent(0);
    //startTimer( 5000 );
}


void Clock::timerEvent( QTimerEvent *e )
{
    QTime tm = QDateTime::currentDateTime().time();
    QString s;
    s.sprintf( "%2d:%02d", tm.hour(), tm.minute() );
    setText( s );
    if ( e )
	QLabel::timerEvent( e );
}

class Background : public QWidget {
    Q_OBJECT
public:
    Background()
	:QWidget( 0, 0, WStyle_Tool | WStyle_Customize )
    {
	QVBoxLayout *vbox = new QVBoxLayout( this );

	QMimeSourceFactory::defaultFactory()
	    ->setImage("logo",QImage("qtlogo.gif"));
	info = new QLabel(this);
	info->setMargin(2);
	info->setFont(QFont("helvetica",10));
	info->setBackgroundColor(white);
	info->setAlignment(AlignTop);
	nextInfo();
	QTimer* infotimer = new QTimer(this);
	connect(infotimer,SIGNAL(timeout()),this,SLOT(nextInfo()));
	infotimer->start(20000);
	setBackgroundColor(white);

	vbox->addWidget( info );
	
	
    }	    
private slots:
    void nextInfo()
    {
	static int i = 0;
	static const int ninfotext = 3;
	static const char* infotext[ninfotext] = {
"<h2>No X11 or MS-Windows</h2>
Since Qt/Embedded runs directly on the device display (such as the
Linux Console in this example), huge amounts of
RAM and ROM are saved on the embedded device.
<p>
<table border=1>
<tr><th><th>RAM<th>ROM
<tr><th>Qt Application<td align=right>0.1M<td align=right>0.1M
<tr><th>Qt/Embedded<td align=right>1M<td align=right>5M
<tr><th>X11<td align=right>8M<td align=right>10M
</table>
<p>
Qt applications are smaller because they share and reuse functionality
from the library. Without the added cost of the X11 server and client
libraries, this means a suite of Qt/Embedded applications is light
on memory.
",

"<h2>Faster Custom Graphics</h2>
Because clients have direct access to the video display, advanced graphics
operations are easily implemented. Qt/Embedded therefore has support for
features not available on X11, such as:
<ul>
 <li>Anti-aliased text
 <li>Alpha-blended pixmaps
 <li>Color cursors
</ul>
Client applications can make their own use of direct display access,
such as:
<ul>
 <li>MPEG directly to the display
 <li>Real-time games
</ul>
The Qt/Embedded graphics kernel is structured for adding 
hardware acceleration for additional devices.
",

"<h2>Scalability</h2>
Qt/Embedded has exactly the same API as Qt/X11 and Qt/Windows, so your
applications can be on X11, Windows, and the embedded device.
<p>
<ul>
<li>Develop on X11 or Windows.
<li>Make software demonstrations that anyone can run.
</ul>

<p>
Qt is highly modular, and so can be customized to smaller
devices. The minimal configuration is about 4M RAM and 4M ROM - including
applications, Linux and Qt/Embedded.
",
	};
	QString t=infotext[i];
	if ( i==1 && getenv("QWS_NOACCEL") )
	    t += " This display has acceleration turned off.";
	
	info->setText( t );

	i = (i+1)%ninfotext;
    }
private:
    QLabel* info;

};

class TaskBar : public QFrame {
    Q_OBJECT
public:
    TaskBar();
private slots:
    void launch();
 
    void quit3()
    {
	qApp->exit(3);
    }

    void execute( int i );

    void showKbd( bool );
    void chooseKbd();
    
private:
    QLabel* clock;
    QPopupMenu *launchMenu;
    QPushButton *launchButton;
    QPushButton *kbdButton;
    QWidget * keyboard;
    enum KeyMode { Pen, Key, Unicode } keyMode;
};

#include "launcher.moc"

TaskBar::TaskBar()
    	:QFrame( 0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop )
{
    keyMode = Key;
    keyboard = 0;
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    QHBoxLayout *hbox = new QHBoxLayout( this, 2 );
    launchButton = new QPushButton( "Launch", this );
    hbox->addWidget( launchButton );
    connect( launchButton, SIGNAL(clicked()), this, SLOT(launch()) );
	
    launchMenu = new QPopupMenu( this );
    for (int i=0; command[i].label; i++) {
	launchMenu->insertItem( command[i].label, i );
    }
    connect( launchMenu, SIGNAL(activated(int)), this, SLOT(execute(int)));

    kbdButton = new QPushButton( "Kbd", this );
    kbdButton->setToggleButton( TRUE );
    hbox->addWidget( kbdButton );
    connect( kbdButton, SIGNAL(toggled(bool)), this, SLOT(showKbd(bool)) );

    QPushButton *kbdChoice = new QPushButton( "^", this );
    hbox->addWidget( kbdChoice );
    connect( kbdChoice, SIGNAL(clicked()), this, SLOT(chooseKbd()) );
    
    launchMenu->insertSeparator();
    launchMenu->insertItem("Quit", qApp, SLOT(quit()));
	
    hbox->addStretch( 1 );
    clock = new Clock( this );
    hbox->addWidget( clock );
}



void TaskBar::execute( int i )
{
    if ( !fork() ) {
	for ( int fd = 0; fd < 100; fd++ ) 
	    ::close( fd );
	chdir( command[i].dir );
	execl( command[i].file, command[i].file, command[i].arg1, 
	       command[i].arg2, 0 );
    }
}

void TaskBar::launch() 
{
    int y = launchButton->mapToGlobal(QPoint()).y() - launchMenu->sizeHint().height();
    launchMenu->popup( QPoint(0,y ) );
}


void TaskBar::chooseKbd()
{
    QPopupMenu pop( this );
#ifdef PEN_INPUT    
    pop.insertItem( "Handwriting", Pen );
#endif
    pop.insertItem( "Keyboard", Key );
    pop.insertItem( "Unicode", Unicode );
    pop.setItemChecked( keyMode, TRUE );
    int h = pop.sizeHint().height();
    int i = pop.exec( mapToGlobal(QPoint(0,-h)));
    if ( i == -1 )
	return;
    if ( i != keyMode && keyboard && keyboard->isVisible() )
	keyboard->hide();
    keyMode = (KeyMode)i;
    if ( !kbdButton->isOn() )
	kbdButton->setOn( TRUE );
    else
	showKbd( TRUE );
}

void TaskBar::showKbd( bool on ) 
{
    if ( !on ) {
	if ( keyboard )
	    keyboard->hide();
	return;
    }

#ifdef PEN_INPUT    
    static QWSPenInput *pi;
#endif
    static Keyboard *kbd;
    static UniKeyboard *uni;
    
    switch ( keyMode ) {
    case Pen:
#ifdef PEN_INPUT    
	if ( !pi ) {
	    pi = new QWSPenInput( 0, 0, WStyle_Customize 
				  //| WStyle_NoBorder | WStyle_StaysOnTop
				  | WStyle_Tool | WStyle_StaysOnTop
				  );
	    pi->setFrameStyle( QFrame::Box | QFrame::Plain );
	    pi->setLineWidth( 1 );
	    pi->addCharSet( "qimpen/asciilower.qpt" );
	    pi->addCharSet( "qimpen/numeric.qpt" );
	    pi->resize( pi->width(), pi->sizeHint().height() + 1 );
	    int h = y();
	    pi->move( 0,  h - pi->height() );
	}
	keyboard = pi;
	pi->show();

#endif    
	break;
    case Key:
	if ( !kbd ) {
	    kbd = new Keyboard( 0, 0, WStyle_Customize 
				  //| WStyle_NoBorder | WStyle_StaysOnTop
				  | WStyle_Tool | WStyle_StaysOnTop
				  );
	    kbd->resize( kbd->sizeHint().width(), kbd->sizeHint().height() + 1 );
	    int h = y();
	    kbd->move( 0,  h - kbd->height() );
	}
	keyboard = kbd;
	kbd->show();
	break;
    case Unicode:
	if ( !uni ) {
	    uni = new UniKeyboard( 0, 0, WStyle_Customize 
				  //| WStyle_NoBorder | WStyle_StaysOnTop
				  | WStyle_Tool | WStyle_StaysOnTop
				  );
	    uni->resize( qApp->desktop()->width(), 
			 qApp->desktop()->height()/2  );
	    int h = y();
	    uni->move( 0,  h - uni->height() );
	}
	keyboard = uni;
	uni->show();
	break;
    }
    
}



#if 0
static
void handleSignal(int sig)
{
    QWSServer::emergency_cleanup();

    if (sig == SIGSEGV) {
	abort();
    }
    exit(0);
}
#endif

void silent(QtMsgType, const char *)
{
}


main(int argc, char** argv)
{

#ifdef __MIPSEL__
    // MIPSEL-specific init - make sure /proc exists for shm
    if ( mount("none","/proc","proc",0,0) ) {
	perror("Mounting - /proc");
    } else {
	fprintf( stderr, "mounted /proc\n" );
    }
    if ( mount("none","/mnt","shm",0,0) ) {
	perror("Mounting - shm");
    }
#endif

    QApplication app(argc,argv, QApplication::GuiServer );
 
    qInstallMsgHandler(silent);

    app.setFont(QFont("helvetica",12));
    
/*
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    signal(SIGQUIT, handleSignal);
    signal(SIGTERM, handleSignal);
    signal(SIGSEGV, handleSignal);
*/
    int w = app.desktop()->width();
    int h = app.desktop()->height();

    Background b;

    TaskBar t;
    t.resize( w, t.sizeHint().height() );
    t.move( 0, h - t.height() );

    
    b.setGeometry( 0, 0, w, h - t.height() );
    b.show();

    
    t.show();
    app.exec();
}

