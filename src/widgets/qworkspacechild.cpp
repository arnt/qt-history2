/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspacechild.cpp#1 $
**
** Implementation of the QWorkspace class
**
** Created : 931107
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include <qapplication.h>
#include <qcursor.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include <qlayout.h>
#include "qworkspacechild.h"
#include "qworkspace.h"

static const char * close_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};        

static const char * maximize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};


static const char * minimize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
"       s None  c None",
".      c white",
"X      c #707070",
/* pixels */
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};




#define TITLEBAR_HEIGHT 18
#define BUTTON_SIZE 18
#define BORDER 2
#define RANGE 6

QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent=0, const char *name=0 )
    : QFrame( parent, name )
{
    mode = 0;
    buttonDown = FALSE;
    setMouseTracking( TRUE );
    act = FALSE;
    
    closeB = new QToolButton( this, "close" );
    closeB->setFocusPolicy( NoFocus );
    closeB->setIconSet( QPixmap( close_xpm ) );
    closeB->resize(BUTTON_SIZE, BUTTON_SIZE);
    connect( closeB, SIGNAL( clicked() ), 
	     this, SLOT( close() ) );
    maxB = new QToolButton( this, "maximize" );
    maxB->setFocusPolicy( NoFocus );
    maxB->setIconSet( QPixmap( maximize_xpm ));
    maxB->resize(BUTTON_SIZE, BUTTON_SIZE);
    iconB = new QToolButton( this, "iconify" );
    iconB->setFocusPolicy( NoFocus );
    iconB->setIconSet( QPixmap( minimize_xpm ) );
    iconB->resize(BUTTON_SIZE, BUTTON_SIZE);
    
    titleL = new QLabel( this );
    
    titleL->setMouseTracking( TRUE );
    titleL->installEventFilter( this );
    titleL->setAlignment( AlignVCenter );
    titleL->setFont( QFont("helvetica", 12, QFont::Bold) );
    
    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    setMinimumSize( 128, 96 );
    
    clientw = window;
    if (!clientw) 
	return;
    
    
    clientw->reparent( this, 0, QPoint( contentsRect().x()+BORDER, TITLEBAR_HEIGHT + BORDER + contentsRect().y() ) );
    clientw->show();
 
    resize( clientw->width() + 2*frameWidth() + 2*BORDER, clientw->height() + 2*frameWidth() + TITLEBAR_HEIGHT +2*BORDER);
    
    clientw->installEventFilter( this );
    
    setActive( TRUE );
}

QWorkspaceChild::~QWorkspaceChild()
{
}


void QWorkspaceChild::resizeEvent( QResizeEvent * )
{
    int bo = (BORDER + TITLEBAR_HEIGHT - BUTTON_SIZE) / 2;
    closeB->move( width() - frameWidth() - BUTTON_SIZE - bo, frameWidth()+ bo  );
    maxB->move( closeB->x() - BUTTON_SIZE - bo, closeB->y() );
    iconB->move( maxB->x() - BUTTON_SIZE, maxB->y() );
    
    titleL->setGeometry( QRect( QPoint( frameWidth() + BORDER + BUTTON_SIZE + bo, closeB->y() ),
			 QPoint( iconB->geometry().left() - bo, iconB->geometry().bottom() ) ) );
			 

    
    if (!clientw)
	return;
    
    QRect r = contentsRect();
    clientw->setGeometry( r.x() + BORDER, r.y() + BORDER + TITLEBAR_HEIGHT, 
			r.width() - 2*BORDER, r.height() - 2*BORDER - TITLEBAR_HEIGHT);
}


bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{
    if ( o == titleL ) {
	if ( e->type() == QEvent::MouseButtonPress
	     || e->type() == QEvent::MouseButtonRelease
	     || e->type() == QEvent::MouseMove) {
	    QMouseEvent* me = (QMouseEvent*) e;
	    QMouseEvent ne( me->type(), titleL->mapToParent(me->pos()), me->button(), me->state() );
	    
	    if (e->type() == QEvent::MouseButtonPress )
		mousePressEvent( &ne );
	    else if (e->type() == QEvent::MouseButtonRelease )
		mouseReleaseEvent( &ne );
	    else
		mouseMoveEvent( &ne );
	}
	return FALSE;
    }
    
    if ( !isActive() ) {
	if ( e->type() == QEvent::MouseButtonPress) {
	    raise();
	    setActive( TRUE );
	}
	return FALSE;
    }
    
    if (o != clientw)
	return FALSE;

    titleL->setText( clientw->caption() );
    
    switch ( e->type() ) {
    case QEvent::Show:
	show();
	break;
    case QEvent::Hide:
	hide();
	break;
    case QEvent::Resize:
	{ 
	    QResizeEvent* re = (QResizeEvent*)e;
	    QSize s( re->size().width() + 2*frameWidth() + 2*BORDER, re->size().height() + 2*frameWidth() + TITLEBAR_HEIGHT +2*BORDER );
	    if ( s != size() ){
		resize( s );
	    }
	}
	break;
    case QEvent::Close:
	close();
	return TRUE;
    default:
	break;
    }
    	
    return FALSE;
}


void QWorkspaceChild::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	raise();
	setActive( TRUE );
	mouseMoveEvent( e );
	buttonDown = TRUE;
	moveOffset = e->pos();
	grabMouse( cursor() );
    }
}

void QWorkspaceChild::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
    }
}

void QWorkspaceChild::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown ) {
	if ( e->pos().y() <= RANGE && e->pos().x() <= RANGE) {
	    setCursor(  sizeFDiagCursor );
	    mode = 1;
	}
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() >= width()-RANGE) {
	    setCursor(  sizeFDiagCursor );
	    mode = 2;
	}
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() <= RANGE) {
	    setCursor(  sizeBDiagCursor );
	    mode = 3;
	}
	else if ( e->pos().y() <= RANGE && e->pos().x() >= width()-RANGE) {
	    setCursor(  sizeBDiagCursor );
	    mode = 4;
	}
	else if ( e->pos().y() <= RANGE || e->pos().y() >= height()-RANGE ) {
	    setCursor(  sizeVerCursor );
	    mode = 5;
	}
	else if ( e->pos().x() <= RANGE || e->pos().x() >= width()-RANGE ) {
	    setCursor(  sizeHorCursor );
	    mode = 6;
	}
	else {
	    setCursor( arrowCursor );
	    mode = 7;
	}
	return;
    }
    
    
    QPoint p = e->globalPos() - parentWidget()->mapToGlobal(QPoint(0,0));
    
    if ( !parentWidget()->rect().contains(p) ) {
	if ( p.x() < 0 )
	    p.rx() = 0;
	if ( p.y() < 0 )
	    p.ry() = 0;
	if ( p.x() > parentWidget()->width() )
	    p.rx() = parentWidget()->width();
	if ( p.y() > parentWidget()->height() )
	    p.ry() = parentWidget()->height();
    }
	
    
    QPoint pp = p - moveOffset;
    QPoint mp( QMIN( pp.x(), geometry().right() - minimumWidth() ),
	       QMIN( pp.y(), geometry().bottom() - minimumHeight() ) );
    

    switch ( mode ) {
    case 1:
	setGeometry( QRect( mp, geometry().bottomRight() ) );
	break;
    case 2:
	setGeometry( QRect( geometry().topLeft(), p ) );
	break;
    case 3:
	setGeometry( QRect( QPoint(mp.x(), geometry().y() ), QPoint( geometry().right(), p.y()) ) );
	break;
    case 4:
	setGeometry( QRect( QPoint(geometry().x(), mp.y() ), QPoint( p.x(), geometry().bottom()) ) );
	break;
    case 5:
	if (moveOffset.y() < RANGE+2) {
	    setGeometry( QRect( QPoint( geometry().left(), mp.y() ), geometry().bottomRight() ) );
	} else {
	    setGeometry( QRect( geometry().topLeft(), QPoint( geometry().right(), p.y() ) ) );
	}
	break;
    case 6:
	if (moveOffset.x() < RANGE+2) {
	    setGeometry( QRect( QPoint( mp.x(), geometry().top() ), geometry().bottomRight() ) );
	} else {
	    setGeometry( QRect( geometry().topLeft(), QPoint( p.x(), geometry().bottom() ) ) );
	}
	break;
    case 7:
	move( pp );
	break;
    default:
	break;
    }
}

void QWorkspaceChild::enterEvent( QEvent * )
{
}

void QWorkspaceChild::leaveEvent( QEvent * )
{
    setCursor( arrowCursor );
}


void QWorkspaceChild::setActive( bool b)
{
    if ( b == act || !clientw)
	return;
    
    act = b;
    if (act) {
	QColorGroup g = colorGroup();
	g.setBackground( darkBlue );
	g.setText( white );
	titleL->setPalette( QPalette( g, g, g), TRUE );
	titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	
	QObjectList* ol = clientw->queryList( "QWidget" );
	for (QObject* o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	delete ol;
	((QWorkspace*)parentWidget())->activateClient( clientWidget() );
    }
    else {
	QColorGroup g = colorGroup();
	titleL->setPalette( QPalette( g, g, g), TRUE );
	titleL->setFrameStyle( QFrame::NoFrame );
	QObjectList* ol = clientw->queryList( "QWidget" );
	for (QObject* o = ol->first(); o; o = ol->next() ) {
	    o->removeEventFilter( this );
	    o->installEventFilter( this );
	}
	delete ol;
    }
}

bool QWorkspaceChild::isActive() const
{
    return act;
}

QWidget* QWorkspaceChild::clientWidget() const
{
    return clientw;
}
