/****************************************************************************
** $Id: //depot/qt/main/examples/compact/launcher.cpp#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qapplication.h>
#include <qcompactstyle.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>


#define PEN_INPUT
#ifdef PEN_INPUT
#include "inputpen.h"
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
#ifndef QT_NO_TEXTBROWSER
    { "Help Text Browser", "../helpviewer", "helpviewer", 0, 0 },
#endif
#ifndef QT_NO_CANVAS
    { "Canvas - alpha-blending", "../canvas", "canvas", 0, 0 },
#endif
#ifndef QT_NO_WIDGETS
    { "Text Editor", "../qwerty", "qwerty", "unicode.txt", 0 },
#endif
#ifndef QT_NO_FILEDIALOG
    { "Scribble Editor", "../scribble", "scribble", 0, 0 },
#endif
#ifndef QT_NO_TRANSLATION
    { "Internationalization", "../i18n", "i18n", 0, 0 },
#endif
#ifndef QT_NO_TRANSFORMATIONS
    { "Magnifier", "../qmag", "qmag", "-geometry",  "100x100" },
#endif
#ifdef QT_NO_WIDGETS // last-resort examples
    { "Hello world", "../hello", "hello", "-geometry",  "100x100" },
    { "Draw lines", "../drawlines", "drawlines", "-geometry",  "100x100" },
#endif
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
    startTimer( 5000 );
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
	if ( QPixmap::defaultDepth() < 12 ) {
	    setBackgroundColor(QColor(0x20, 0xb0, 0x50));
	} else {
	    setBackgroundPixmap(QPixmap("bg.xpm"));
	}
    }	    
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
"28 13 2 1",
"# c #303030",
"  c None",
" ########################## ",
" #   #   #   #   #   #    # ",
" #   #   #   #   #   #    # ",
" ########################## ",
" #     #   #   #   #      # ",
" #     #   #   #   #      # ",
" ########################## ",
" #      #   #   #   #     # ",
" #      #   #   #   #     # ",
" ########################## ",
" #    #              #    # ",
" #    #              #    # ",
" ########################## "};



/* XPM */
static const char * const pen_xpm[] = {
"28 13 4 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"@      c #808080",
"                            ",
"                         .  ",
"                        .+. ",
"                       ..@@.",
"                      .+@.. ",
"      ...            .+@@.  ",
"    .......         .+@@.   ",
"   ..     ..       .@.@.    ",
"   ..     ..       .@@.     ",
"   ..     ..      ....      ",
"  ..       ...  ....        ",
"            ......          ",
"                            "};




/* XPM */
static const char * const uni_xpm[]={
"28 13 2 1",
"# c #000000",
". c None",
"............................",
"...####....#####.....####...",
"...####....######....####...",
"...####....#######..........",
"...####....########..####...",
"...####....####.####.####...",
"...####....####..########...",
"...####....####...#######...",
"...####....####....######...",
"...#####..#####.....#####...",
"....##########.......####...",
"......######..........###...",
"............................"};




TaskBar::TaskBar()
    	:QFrame( 0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop )
{
    keyMode = Key;
    keyboard = 0;
    //setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    QHBoxLayout *hbox = new QHBoxLayout( this, 0, 0 );
    launchButton = new QPushButton( "Launch", this );
    hbox->addWidget( launchButton );
    connect( launchButton, SIGNAL(clicked()), this, SLOT(launch()) );
	
    launchMenu = new QPopupMenu( this );
    for (int i=0; command[i].label; i++) {
	launchMenu->insertItem( command[i].label, i );
    }

    // hbox->addSpacing( 2 );
    
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
	    QString baseDir( "/usr/local/qt-embedded/etc" );
	    const char *qtdir = getenv( "QTDIR" );
	    if ( qtdir )
		baseDir = QString( qtdir ) + "/etc";
	    pi->addCharSet( baseDir + "/qimpen/asciilower.qpt" );
	    pi->addCharSet( baseDir + "/qimpen/numeric.qpt" );
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
 
    //qInstallMsgHandler(silent);

    app.setFont(QFont("helvetica",10));
    
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
