#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
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
	    ->setImage("qtlogo",QImage("qtlogo.png"));
	QMimeSourceFactory::defaultFactory()
	    ->setImage("face",QImage("face.png"));

	vbox->addSpacing(8);
	QLabel *heading = new QLabel(this);
	heading->setText("<center><img src=qtlogo><img src=face></center>");
	heading->setBackgroundColor(white);
	heading->setFixedSize(heading->sizeHint());
	vbox->addWidget( heading );

	info = new QLabel(this);
	info->setMargin(8);
	info->setFont(QFont("helvetica",8));
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
	static const char* infotext[] = {
	"<b><font size=+2>Qt/Embedded - What is it?</font></b><br>

	The Qt/Embedded product provides you with all you need to
	create stunning graphical user interfaces for embedded devices.
	Qt/Embedded installs and runs with a very small memory footprint on any
	device running embedded Linux - without using X11.  ",

	"<b><font size=+2>Qt/Embedded - Trust the promises</font></b><br>

	Qt/Embedded features the same API as the excellent
	Qt/Windows and Qt/X11 versions. Imagine writing your Qt application in
	your favourite desktop environment and just recompiling to move it to
	your embedded device. It saves you heaps of development effort and
	allows you to work productively in your favorite programming
	environment from day one. You can start today.  ",

	"<b><font size=+2>Qt/Embedded - Less is more</font></b><br>

	Qt/Embedded is modular and scalable. You can assemble the
	Qt features you really need and leave the others out. Since Qt/Embedded
	is not based on X11 it has substantially lower memory requirements than
	X11. By picking and choosing features, the memory demands of
	Qt/Embedded can be tuned from 1 Mb to 3 Mb in ROM (Intel x86).
	Furthermore,applications written with Qt are known to have a small
	memory footprint compared to applications written with other toolkits.
	Qt scales from the smallest embedded device to high-end workstations.  ",

	"<b><font size=+2>Qt/Embedded - Use the source</font></b><br>

	You know the value of source code availability. All Qt
	releases are delivered with source code. Qt/Embedded is no exception.
	You get a better understanding of how Qt works and it helps you debug
	and tune your code. Customer feedback allows us to continually improve
	the source code for better usability and performance.  ",

	"<b><font size=+2>Qt/Embedded - Add your touch</font></b><br>

	Qt's clean object oriented design makes it easy to extend and
	enhance the standard widgets. You can create specialized widgets for the
	limited space offered by embedded screen devices. You can benefit from the
	wealth of available third-party Qt software. It is easy to add the killer
	feature you need.",

	"<b><font size=+2>Qt/Embedded - The beauty of it</font></b><br>

	Qt/Embedded features some great additional functionality compared
	to X11. You will be stunned by the beauty of anti-aliased text and alpha
	blended pixmaps. These new features could add an additional touch of class to a
	user interface. Forget about embedded graphics which looks more like old
	alphanumeric terminals. Qt/Embedded can utilize hardware graphics acceleration
	and it is well suited for multimedia and web applications. Let Qt/Embedded
	impress you and your customers.",

	"<b><font size=+2>Qt/Embedded - Getting there faster</font></b><br>

	Qt is one of the most popular GUI toolkits in the world.
	Programmers like the compact code,the powerful API,the ease of use, and the
	excellent support. It is easy to find existing developer skills with Qt and a
	lot of quality Qt based code has already been written. Qt/Embedded allows you
	and your programmers to move seamlessly into the exciting field of embedded
	systems. Your Qt experts don't need to be retrained. Our customers tell us that
	even programmers without Qt experience get up to speed sooner with Qt than with
	other toolkits.",

	"<b><font size=+2>Qt/Embedded - We support you</font></b><br>

	Embedded devices have other requirements than conventional
	computers. The Qt/Embedded team at Trolltech is dedicated to support you with
	new features and widgets. We can offer training, partnerships and first class
	support. It is straightforward to port Qt/Embedded to new hardware. We can
	offer you a port to your special device or the embedded operating system of
	your choice. We look forward to working closely with you to help you succeed.",

	0
	};
	QString t=infotext[i];
	info->setText(t);
	i++;
	if (!infotext[i]) i=0;
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
    QPushButton *kbdChoice;
    QWidget * keyboard;
    enum KeyMode { Pen, Key, Unicode } keyMode;
};

#include "launcher.moc"


/* XPM */
static const char * const tri_xpm[]={
"9 9 2 1",
"a c #000000",
". c None",
".........",
".........",
".........",
"....a....",
"...aaa...",
"..aaaaa..",
".aaaaaaa.",
".........",
"........."};



/* XPM */
static const char * const kb_xpm[]={
"32 17 2 1",
"# c #303030",
"  c None",
" ############################## ",
" #   #   #   #   #   #   #    # ",
" #   #   #   #   #   #   #    # ",
" #   #   #   #   #   #   #    # ",
" ############################## ",
" #     #   #   #   #   #      # ",
" #     #   #   #   #   #      # ",
" #     #   #   #   #   #      # ",
" ############################## ",
" #      #   #   #   #   #     # ",
" #      #   #   #   #   #     # ",
" #      #   #   #   #   #     # ",
" ############################## ",
" #    #                  #    # ",
" #    #                  #    # ",
" #    #                  #    # ",
" ############################## "};



/* XPM */
static const char * const pen_xpm[] = {
"32 16 4 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"@      c #808080",
"                                ",
"                                ",
"                             .  ",
"                            .+. ",
"                           ..@@.",
"                          .+@.. ",
"        ....             .+@@.  ",
"      .......           .+@@.   ",
"     ..     ..         .+@@.    ",
"    ..       ..       .@.@.     ",
"    ..       ..       .@@.      ",
"    ..       ..      ....       ",
"   ..         .....  ..         ",
"               .....            ",
"                                ",
"                                "};




/* XPM */
static const char * const uni_xpm[]={
"32 16 2 1",
"# c #000000",
". c None",
"................................",
"...####.....#####.......####....",
"...####.....######......####....",
"...####.....#######.............",
"...####.....########....####....",
"...####.....#########...####....",
"...####.....####.#####..####....",
"...####.....####..#####.####....",
"...####.....####...#########....",
"...####.....####....########....",
"...####.....####.....#######....",
"...#####...#####......######....",
"....###########........#####....",
".....#########..........####....",
".......######............###....",
"................................"};




TaskBar::TaskBar()
    	:QFrame( 0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop )
{
    keyMode = Key;
    keyboard = 0;
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    QHBoxLayout *hbox = new QHBoxLayout( this, 2, 0 );
    launchButton = new QPushButton( "Launch", this );
    hbox->addWidget( launchButton );
    connect( launchButton, SIGNAL(clicked()), this, SLOT(launch()) );
	
    launchMenu = new QPopupMenu( this );
    for (int i=0; command[i].label; i++) {
	launchMenu->insertItem( command[i].label, i );
    }

    hbox->addSpacing( 2 );
    
    connect( launchMenu, SIGNAL(activated(int)), this, SLOT(execute(int)));

    kbdButton = new QPushButton( this );
    kbdButton->setPixmap( QPixmap( (const char **)kb_xpm ) );
    kbdButton->setToggleButton( TRUE );
    kbdButton->setFixedHeight( launchButton->sizeHint().height() );
    hbox->addWidget( kbdButton );
    connect( kbdButton, SIGNAL(toggled(bool)), this, SLOT(showKbd(bool)) );
    
    kbdChoice = new QPushButton( this );
    kbdChoice->setPixmap( QPixmap( (const char **)tri_xpm ) );
    kbdChoice->setFixedHeight( launchButton->sizeHint().height() );
    
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

    QPoint pt = mapToGlobal(kbdChoice->geometry().topRight());
    QSize s = pop.sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    int i = pop.exec( pt );
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
	    pi->resize( qApp->desktop()->width(), pi->sizeHint().height() + 1 );
	    int h = y();
	    pi->move( 0,  h - pi->height() );
	}
	keyboard = pi;
	pi->show();
	kbdButton->setPixmap( QPixmap( (const char **)pen_xpm ) );

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
	kbdButton->setPixmap( QPixmap( (const char **)kb_xpm ));
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
	kbdButton->setPixmap( QPixmap( (const char **)uni_xpm ));
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
