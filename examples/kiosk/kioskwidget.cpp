/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "kioskwidget.h"
#include "qmpeg.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qfiledialog.h>

#include <qtextbrowser.h>


/*
#include "stop.xpm"
#include "loop.xpm"
#include "play.xpm"
*/

class MovieScreen : public QWidget
{
    Q_OBJECT;
public:
    MovieScreen( const QString& fn, 
		 QWidget *parent = 0, const char *name = 0 );
    ~MovieScreen();
    bool isPlaying() const { return mpeg && mpeg->isPlaying(); }
public slots:    
    void startMovie( const QString& );
signals:
    void finished();
private slots:    
    void play();
    void stop();
    void startStop();
    void movieFinished();
    void setLoop(bool b) { loop = b; }
private:
    QMpegWidget *mpeg;
    //QLabel *label;
    //    QPushButton *playb;
    QPushButton *stopstart;
    QPushButton *loopb;
    QString filename;
    bool loop;
};


#include "kioskwidget.moc"

MovieScreen::MovieScreen( const QString& fn, 
			  QWidget *parent, const char *name )
    :QWidget( parent, name ), filename( fn )
{
    loop = FALSE;
    QVBoxLayout *vbox = new QVBoxLayout( this, 0, 10 );

    mpeg = new QMpegWidget( this );
    mpeg->setBackgroundColor( black );
    mpeg->setFrameStyle( QFrame::NoFrame );
    mpeg->setMinimumHeight( 300 );
    
    
    
    
    
    connect( mpeg, SIGNAL(finished()), this, SLOT( movieFinished()) );
    connect( mpeg, SIGNAL(stopped()), this, SIGNAL( finished()) );
    vbox->addWidget( mpeg );
    
    
    QHBoxLayout *hbox = new QHBoxLayout( vbox );
    //    label = new QLabel( fn, this );
    //    label->setFrameStyle( QFrame::StyledPanel|QFrame::Sunken );
    //    hbox->addWidget( label );

    stopstart = new QPushButton( "Stop", this );
    //    stopb->setPixmap( QPixmap(stop_xpm) );
    //    stopb->setFixedHeight( playb->height() );
    hbox->addWidget( stopstart );
    connect( stopstart, SIGNAL(clicked()), this, SLOT(startStop()) );
    stopstart->setEnabled( FALSE );
    
    loopb = new QPushButton( "Loop", this );
    //    loopb->setPixmap( QPixmap(loop_xpm) );
    // loopb->setFixedHeight( playb->height() );
    loopb->setToggleButton( TRUE );
    hbox->addWidget( loopb );
    connect( loopb, SIGNAL(toggled(bool)), this, SLOT(setLoop(bool)) );
}


MovieScreen::~MovieScreen()
{
}


void MovieScreen::movieFinished()
{
    if ( !loop ) {
	stopstart->setText( "Start" );
	//	playb->setEnabled( TRUE );
	emit finished();
    } else {
	mpeg->play( filename );
    }
}    
    
void MovieScreen::play()
{
    mpeg->erase();
    mpeg->play( filename );
    stopstart->setText( "Stop" );
    stopstart->setEnabled( TRUE );
}


void MovieScreen::startStop()
{
    if ( mpeg->isPlaying() )
	stop();
    else
	play();
}
	    

void MovieScreen::stop()
{
    mpeg->stop();
    stopstart->setText( "Start" );
}
 
void MovieScreen::startMovie( const QString &fn )
{
    filename = fn;
    //    label->setText( fn );
    play();
}



KioskWidget::KioskWidget( QStringList files,
			  QWidget *parent, const char *name )
    :QWidget(parent,name,WStyle_Tool | WStyle_Customize)
{
    setFont( QFont( "babelfish", 72 ) );

    QHBoxLayout *hbox = new QHBoxLayout( this, 10 );
    QVBoxLayout *moviebox = new QVBoxLayout( hbox );

    screen = new MovieScreen( files[0],this );
    moviebox->addWidget( screen );

    
    browser = new QTextBrowser( this );
    browser->mimeSourceFactory()->setFilePath( "." );
    moviebox->addWidget( browser );
    browser->setFont( QFont( "babelfish", 24 ) );
    browser->setSource( "about.html" );    
    browser->setHScrollBarMode( QListBox::AlwaysOff );    

    QVBoxLayout *rightbox = new QVBoxLayout( hbox );
    lb = new QListBox( this );
    lb->setVScrollBarMode( QListBox::AlwaysOff );
    lb->setHScrollBarMode( QListBox::AlwaysOff );
    lb->insertStringList( files );
    rightbox->addWidget( lb );
    connect( lb, SIGNAL(mouseButtonClicked(int, QListBoxItem*, const QPoint&)),
	     this, SLOT(handleClick(int,QListBoxItem*)) );
    connect( lb, SIGNAL(returnPressed( QListBoxItem*)),
	     this, SLOT(play(QListBoxItem*)) );
    lb->setFocus();
    
    QHBoxLayout *buttonbox = new QHBoxLayout( rightbox );

    QPushButton *browse = new QPushButton( tr("Browse..."), this );
    buttonbox->addWidget( browse );
    connect ( browse, SIGNAL(clicked()), this, SLOT(browse()) );

    QPushButton *quit = new QPushButton( tr("Quit"), this );
    buttonbox->addWidget( quit);
    connect ( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}


/*! Destroys the object and frees any allocated resources.

*/

KioskWidget::~KioskWidget()
{
    
}

void KioskWidget::handleClick(int button, QListBoxItem* lbi )
{
    if ( button == LeftButton ) {
	play( lbi );
    }
}


void KioskWidget::play(QListBoxItem* lbi )
{
    if ( lbi ) {
	play( lbi->text() );
    }
}



void KioskWidget::browse()
{
    QString newFile = QFileDialog::getOpenFileName( QString::null,
						    "MPEG files(*.mpg *.mpeg)"
						    ";;Text files(*.txt)"
						    ";;HTML files(*.html)",
						    0 );
    if ( newFile.isNull())
	return;
    QFileInfo fi( newFile );
    if ( fi.extension() == "mpg" ) {
	lb->insertItem( newFile );
	play( newFile );
    } else {
	browser->setSource( fi.filePath() );
    }
}


void KioskWidget::play( const QString &fn )
{
    QFileInfo f( fn );
    QString s = f.dirPath() + "/" + f.baseName();
    QFileInfo f1( s+".html" );
    if ( f1.exists() ) {
	browser->setSource( f1.filePath() );
    } else {
	QFileInfo f2( s+".txt" );
	if ( f2.exists() ) 
	    browser->setSource( f2.filePath() );
	else
	    browser->setSource( "about.html" );
    }
    
    screen->startMovie( fn );
}
