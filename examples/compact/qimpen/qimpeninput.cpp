/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method
**
** Created : 20000414
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include "qimpenwidget.h"
#include "qimpensetup.h"
#include "qimpeninput.h"

// We'll use little pixmaps for the buttons to save screen space.

/* XPM */
static const char * const pen_xpm[] = {
"12 16 4 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #808080",
"            ",
"            ",
"         .  ",
"        .+. ",
"       ..@@.",
"      .+@.. ",
"     .+@@.  ",
"    .+@@.   ",
"   .+@@.    ",
"  .@.@.     ",
"  .@@.      ",
" ....       ",
" ..         ",
"            ",
"            ",
"            "};


/* XPM */
static const char * const updown_xpm[] = {
"12 16 4 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #7F7F7F",
"            ",
"     .      ",
"    .+.     ",
"   .+@@.    ",
"  ...+...   ",
"    .+.     ",
"    .+.     ",
"    ...     ",
"            ",
"    ...     ",
"    .+.     ",
"    .+.     ",
"  ...+...   ",
"   .+@@.    ",
"    .@.     ",
"     .      "};


/* XPM */
static const char * const hide_xpm[] = {
"12 16 5 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #BFBFBF",
"#	c #7F7F7F",
"            ",
"        ..  ",
"          . ",
"        ... ",
"       .  . ",
"     . .  . ",
"    ..  .. .",
"   .+.      ",
"  .+@.      ",
" .+@@.      ",
".+@@@.      ",
" .#@@.      ",
"  .#@.      ",
"   .#.      ",
"    ..      ",
"     .      "};


/* XPM */
static const char * const show_xpm[] = {
"12 16 5 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #BFBFBF",
"#	c #7F7F7F",
"            ",
"        ..  ",
"          . ",
"        ... ",
"  .    .  . ",
"  ..   .  . ",
"  .+.   .. .",
"  .@+.      ",
"  .@@+.     ",
"  .@@@+.    ",
"  .@@#.     ",
"  .@#.      ",
"  .#.       ",
"  ..        ",
"  .         ",
"            "};

/*
  Simple widget that we can use to move the input window around.
*/
class GrabWidget : public QLabel
{
public:
    GrabWidget( QWidget *parent ) : QLabel( parent )
    {
	QPixmap pm( (const char **)updown_xpm );
	setPixmap( pm );
	setFrameStyle( Panel | Raised );
	setLineWidth( 2 );
	moving = FALSE;
	tid = 0;
	count = 0;
	newx = 0;
	newy = 0;
    }

protected:
    virtual void mousePressEvent( QMouseEvent *e )
    {
	moving = TRUE;
	pos = e->globalPos();
	newx = topLevelWidget()->x();
	newy = topLevelWidget()->y();
    }

    virtual void mouseMoveEvent( QMouseEvent *e )
    {
	if ( moving ) {
	    int dy = e->globalPos().y() - pos.y();
	    QWidget *tl = topLevelWidget();
	    int y = newy + dy;
	    if ( y < 0 )
		y = 0;
	    if ( y + tl->height() > qApp->desktop()->height() )
		y = qApp->desktop()->height() - tl->height();
	    pos = e->globalPos();
	    pos.ry() -= dy - (y - newy );
	    newx = tl->x();
	    newy = y;
	    count++;
	    if ( count < 5 ) {
		if ( tid != -1 )
		    killTimer( tid );
		tid = startTimer( 20 );
	    }
	}
    }

    virtual void mouseReleaseEvent( QMouseEvent * )
    {
	moving = FALSE;
    }

    virtual void timerEvent( QTimerEvent *e )
    {
	if ( e->timerId() == tid )
	{
	    QWidget *tl = topLevelWidget();
	    topLevelWidget()->setGeometry( newx, newy, tl->width(), tl->height() );
	    if ( tid != -1 )
		killTimer( tid );
	    tid = -1;
	    count = 0;
	}
    }

protected:
    bool moving;
    int newx;
    int newy;
    int tid;
    int count;
    QPoint pos;
};

/*!
  \class QIMPenInput qimpeninput.h

  Pen input widget.
*/
QIMPenInput::QIMPenInput( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
    strokes.setAutoDelete( TRUE );
    charSets.setAutoDelete( TRUE );

    hidden = FALSE;
    QGridLayout *gl = new QGridLayout( this, 3, 2, 1, 1 );
    gl->setColStretch( 1, 1 );

    moveWidget = (QWidget *)new GrabWidget( this );
    gl->addWidget( moveWidget, 0, 0 );

    QPixmap pm( (const char **)pen_xpm );
    setupBtn = new QPushButton( this );
    setupBtn->setPixmap( pm );
    connect( setupBtn, SIGNAL(clicked()), SLOT(setup()));
    gl->addWidget( setupBtn, 1, 0 );

    QPixmap pm1( (const char **)hide_xpm );
    hideBtn = new QPushButton( this );
    hideBtn->setPixmap( pm1 );
    connect( hideBtn, SIGNAL(clicked()), SLOT(hideShow()));
    gl->addWidget( hideBtn, 2, 0 );

    pw = new QIMPenWidget( this );
    gl->addMultiCellWidget( pw, 0, 2, 1, 1 );
    connect( pw, SIGNAL(changeCharSet( QIMPenCharSet * )),
                 SLOT(selectCharSet( QIMPenCharSet * )) );
    connect( pw, SIGNAL(stroke( QIMPenStroke * )),
                 SLOT(strokeEntered( QIMPenStroke * )) );
    connect( pw, SIGNAL(beginStroke()), SLOT(beginStroke()) );

    charSets.setAutoDelete( TRUE );

    pw->setMinimumHeight( pw->sizeHint().height() );
    gl->activate();

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(processMatches()) );

    matchChar = 0;
    currCharSet = 0;
    strokeMode = Backspace;
    lastKey = 0;
}

QIMPenInput::~QIMPenInput()
{
}

/*!
  Add a character set from file \a fn.
*/
void QIMPenInput::addCharSet( const QString &fn )
{
    QIMPenCharSet *cs = new QIMPenCharSet( fn );
    if ( !cs->isEmpty() ) {
       charSets.append( cs );
       pw->addCharSet( cs );
       if ( !currCharSet )
	   currCharSet = cs;
    } else
       delete cs;
}

void QIMPenInput::selectCharSet( QIMPenCharSet *cs )
{
    processMatches();
    currCharSet = cs;
}

void QIMPenInput::beginStroke()
{
    timer->stop();
}

/*!
  Attempts to match the character entered and emit's key();
*/
void QIMPenInput::strokeEntered( QIMPenStroke *st )
{
    strokes.append( new QIMPenStroke( *st ) );
    testChar.addStroke( st );

    const QIMPenCharMatchList &ml = currCharSet->match( &testChar );
    if ( ml.getFirst() ) {
        matchChar = ml.getFirst()->penChar;
    }

    if ( !matchChar ) {
        qDebug( "Unmatched strokes - panic" );
        strokes.clear();
        testChar.clear();
        pw->clear();
	return;
    }

    if ( strokeMode == Backspace ) {
	processMatches();
    } else if ( strokeMode == Delay ) {
	if ( strokes.count() >= currCharSet->maximumStrokes()
		    || !ml.getFirst() ) {
	    processMatches();
	} else {
	    timer->start( 500, TRUE );
	}
    }
}

void QIMPenInput::processMatches()
{
    if ( matchChar && strokeMode == Delay ) {
        if ( matchChar->penStrokes().count() <= strokes.count() ) {
            for ( unsigned i = 0; i < matchChar->penStrokes().count(); i++ ) {
                strokes.removeFirst();
                pw->removeStroke();
            }
            emit key( matchChar->character() );
            qDebug( "** match \'%c\' ***", matchChar->character() );
            matchChar = 0;
            testChar.clear();
            QIMPenStroke *st = strokes.first();
            while ( st ) {
                testChar.addStroke( st );
                const QIMPenCharMatchList &ml = currCharSet->match( &testChar );
                if ( ml.getFirst() )
                    matchChar = ml.getFirst()->penChar;
                st = strokes.next();
            }
            
            if ( matchChar )
                timer->start( 500, TRUE );
        }
    } else if ( matchChar && strokeMode == Backspace ) {
        if ( matchChar->penStrokes().count() == strokes.count() ) {
            if ( strokes.count() > 1 && lastKey
                 && (lastKey >> 16) != Qt::Key_Backspace) {
                qDebug( "deleting last" );
                emit key( Qt::Key_Backspace << 16 );
            }
            emit key( matchChar->character() );
            qDebug( "** match \'%c\' ***", matchChar->character() );
            lastKey = matchChar->character();
        }
        pw->removeStroke();
        if ( matchChar->penStrokes().count() < strokes.count() ) {
            for ( unsigned i = 0; i < matchChar->penStrokes().count(); i++ ) {
                strokes.removeFirst();
            }
            lastKey = 0;
            matchChar = 0;
            testChar.clear();
            QIMPenStroke *st = strokes.first();
            if ( st ) {
                testChar.addStroke( st );
                const QIMPenCharMatchList &ml = currCharSet->match( &testChar );
                if ( ml.getFirst() ) {
                    matchChar = ml.getFirst()->penChar;
                    if ( matchChar->penStrokes().count() == 1 ) {
                        emit key( matchChar->character() );
                        qDebug( "** match \'%c\' ***", matchChar->character() );
                        lastKey = matchChar->character();
                    }
                }
            }
        }
    }
    if ( !matchChar && strokes.count() ) {
        qDebug( "Unmatched strokes - panic" );
        strokes.clear();
        testChar.clear();
        pw->clear();
    }
}

/*!
  Open the setup dialog
*/
void QIMPenInput::setup()
{
    // ### we are working with our copy of the char sets here.
    QIMPenSetup ps( &charSets, 0, 0, TRUE );
    if ( ps.exec() ) {
	QListIterator<QIMPenCharSet> it(charSets);
	for ( ; it.current(); ++it )
	    it.current()->save();
    }
}

/*!
  Toggle whether the full input window is visible, or just the little
  expand icon
*/
void QIMPenInput::hideShow()
{
    if ( !hidden ) {
	prefRect = geometry();
	pw->hide();
	setupBtn->hide();
	moveWidget->hide();
	layout()->activate();
	setGeometry( x(), y() + height() - hideBtn->height() - 4,
		     hideBtn->width(), hideBtn->height() );
	/*
	move( x(), y() + height() - hideBtn->height() - 4 );
	resize( hideBtn->width(), hideBtn->height() );
	*/
	QPixmap pm( (const char **)show_xpm );
	hideBtn->setPixmap( pm );
	hidden = TRUE;
    } else {
	pw->show();
	setupBtn->show();
	moveWidget->show();
	layout()->activate();
	setGeometry( prefRect );
	/*
	move( prefRect.x(), prefRect.y() );
	resize( prefRect.width(), prefRect.height() );
	*/
	QPixmap pm( (const char **)hide_xpm );
	hideBtn->setPixmap( pm );
	hidden = FALSE;
    }
}

