/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspacechild.cpp#5 $
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
#include "qvbox.h"

//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

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


static const char * normalize_xpm[] = {
/* width height num_colors chars_per_pixel */
"16 16 3 1",
/* colors */
" 	s None	c None",
".	c white",
"X	c #707070",
/* pixels */
"                ",
"                ",
"     ........   ",
"     .XXXXXXXX  ",
"     .X     .X  ",
"     .X     .X  ",
"  ....X...  .X  ",
"  .XXXXXXXX .X  ",
"  .X     .XXXX  ",
"  .X     .X     ",
"  .X     .X     ",
"  .X......X     ",
"  .XXXXXXXX     ",
"                ",
"                ",
"                "};


#define TITLEBAR_HEIGHT 18
#define TITLEBAR_SEPARATION 2
#define BUTTON_SIZE 18
#define TITLE_HEIGHT 18
#define BORDER 2
#define RANGE 6


QWorkspaceChildTitelBar::QWorkspaceChildTitelBar (QWorkspace* w, QWidget* parent, const char* name, bool iconMode )
    : QWidget( parent, name )
{
    workspace = w;
    buttonDown = FALSE;
    imode = iconMode;
    act = FALSE;

    closeB = new QToolButton( this, "close" );
    closeB->setFocusPolicy( NoFocus );
    closeB->setIconSet( QPixmap( close_xpm ) );
    closeB->resize(BUTTON_SIZE, BUTTON_SIZE);
    connect( closeB, SIGNAL( clicked() ),
	     this, SIGNAL( doClose() ) ) ;
    maxB = new QToolButton( this, "maximize" );
    maxB->setFocusPolicy( NoFocus );
    maxB->setIconSet( QPixmap( maximize_xpm ));
    maxB->resize(BUTTON_SIZE, BUTTON_SIZE);
    connect( maxB, SIGNAL( clicked() ),
	     this, SIGNAL( doMaximize() ) );
    iconB = new QToolButton( this, "iconify" );
    iconB->setFocusPolicy( NoFocus );
    iconB->resize(BUTTON_SIZE, BUTTON_SIZE);
    
    if ( imode ) {
	iconB->setIconSet( QPixmap( normalize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doNormal() ) );
    }
    else {
	iconB->setIconSet( QPixmap( minimize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doMinimize() ) );
    }

    titleL = new QLabel( this );

    titleL->setMouseTracking( TRUE );
    titleL->installEventFilter( this );
    titleL->setAlignment( AlignVCenter );
    titleL->setFont( QFont("helvetica", 12, QFont::Bold) );

    resize( 256, TITLEBAR_HEIGHT );

}

QWorkspaceChildTitelBar::~QWorkspaceChildTitelBar()
{
}

void QWorkspaceChildTitelBar::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = TRUE;
	moveOffset = mapToParent( e->pos() );
	emit doActivate();
    }
}

void QWorkspaceChildTitelBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
    }
}

void QWorkspaceChildTitelBar::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown )
	return;
    QPoint p = workspace->mapFromGlobal( e->globalPos() );
    if ( !workspace->rect().contains(p) ) {
	if ( p.x() < 0 )
	    p.rx() = 0;
	if ( p.y() < 0 )
	    p.ry() = 0;
	if ( p.x() > workspace->width() )
	    p.rx() = workspace->width();
	if ( p.y() > workspace->height() )
	    p.ry() = workspace->height();
    }
	
    QPoint pp = p - moveOffset;

    parentWidget()->move( pp );
}



bool QWorkspaceChildTitelBar::eventFilter( QObject * o, QEvent * e)
{
    titleL->setText( caption() );
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
	else if ( e->type() == QEvent::MouseButtonDblClick )
	    emit doNormal();
    }
    return FALSE;
}


void QWorkspaceChildTitelBar::resizeEvent( QResizeEvent * )
{
    int bo = ( height()- BUTTON_SIZE) / 2;
    closeB->move( width() - BUTTON_SIZE - bo, bo  );
    maxB->move( closeB->x() - BUTTON_SIZE - bo, closeB->y() );
    iconB->move( maxB->x() - BUTTON_SIZE, maxB->y() );

    int to = ( height()- TITLE_HEIGHT) / 2;

    if (imode && !isActive() )
	titleL->setGeometry( QRect( QPoint( BUTTON_SIZE + bo, 0 ),
				    rect().bottomRight() ) );
    else
	titleL->setGeometry( QRect( QPoint( BUTTON_SIZE + bo, to ),
				    QPoint( iconB->geometry().left() - bo, to + TITLE_HEIGHT ) ) );

}


void QWorkspaceChildTitelBar::setActive( bool active )
{
    titleL->setText( caption() );
    act = active;
    if ( active ) {
	if ( imode ){
	    iconB->show();
	    maxB->show();
	    closeB->show();
	}
	QColorGroup g = colorGroup();
	g.setBackground( darkBlue );
	g.setText( white );
	titleL->setPalette( QPalette( g, g, g), TRUE );
	titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    }
    else {
	if ( imode ){
	    iconB->hide();
	    closeB->hide();
	    maxB->hide();
	}
	QColorGroup g = colorGroup();
	titleL->setPalette( QPalette( g, g, g), TRUE );
	titleL->setFrameStyle( QFrame::NoFrame );
    }
    if ( imode )
	resizeEvent(0);
}

bool QWorkspaceChildTitelBar::isActive() const
{
    return act;
}

QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent=0, const char *name=0 )
    : QFrame( parent, name )
{
    mode = 0;
    buttonDown = FALSE;
    setMouseTracking( TRUE );
    act = FALSE;
    iconw = 0;

    titlebar = new QWorkspaceChildTitelBar( parent, this );
    connect( titlebar, SIGNAL( doActivate() ),
	     this, SLOT( activate() ) );
    connect( titlebar, SIGNAL( doClose() ),
	     this, SLOT( close() ) );
    connect( titlebar, SIGNAL( doMinimize() ),
	     this, SLOT( showMinimized() ) );

    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    setMinimumSize( 128, 96 );

    clientw = window;
    if (!clientw)
	return;


    connect( clientw, SIGNAL( destroyed() ), this, SLOT( clientDestroyed() ) );

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

    QRect r = contentsRect();
    titlebar->setGeometry( r.x() + BORDER, r.y() + BORDER, r.width() - 2*BORDER, TITLEBAR_HEIGHT+1);

    if (!clientw)
	return;

    QRect cr( r.x() + BORDER, r.y() + BORDER + TITLEBAR_SEPARATION + TITLEBAR_HEIGHT,
			r.width() - 2*BORDER,
			  r.height() - 2*BORDER - TITLEBAR_SEPARATION - TITLEBAR_HEIGHT);
    clientSize = cr.size();
    clientw->setGeometry( cr );
}

void QWorkspaceChild::activate()
{
    setActive( TRUE );
}


bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{

    titlebar->setCaption( clientw->caption() );

    if ( !isActive() ) {
	if ( e->type() == QEvent::MouseButtonPress) {
	    setActive( TRUE );
	}
	return FALSE;
    }

    if (o != clientw)
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Show:
	show();
	break;
    case QEvent::Hide:
	if (iconw) {
	    delete iconw->parentWidget();
	    iconw = 0;
	}
	hide();
	break;
    case QEvent::Resize:
	{
	    QResizeEvent* re = (QResizeEvent*)e;
	    if ( re->size() != clientSize ){
		QSize s( re->size().width() + 2*frameWidth() + 2*BORDER,
			 re->size().height() + 3*frameWidth() + TITLEBAR_HEIGHT +TITLEBAR_SEPARATION+2*BORDER );
		resize( s );
	    }
	}
	break;
    default:
	break;
    }
    	
    return FALSE;
}


void QWorkspaceChild::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	activate();
	mouseMoveEvent( e );
	buttonDown = TRUE;
	moveOffset = e->pos();
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


    QPoint p = parentWidget()->mapFromGlobal( e->globalPos() );

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

    titlebar->setActive( act );
    if (iconw )
	iconw->setActive( act );

    if (act) {
	QObjectList* ol = clientw->queryList( "QWidget" );
	for (QObject* o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	delete ol;
	((QWorkspace*)parentWidget())->activateClient( clientWidget() );
    }
    else {
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


QWidget* QWorkspaceChild::iconWidget() const
{
    if ( !iconw ) {
	QWorkspaceChild* that = (QWorkspaceChild*) this;
	QVBox* vbox = new QVBox;
	vbox->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	vbox->resize( 196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth() );
	that->iconw = new QWorkspaceChildTitelBar( (QWorkspace*)parentWidget(), vbox, 0, TRUE );
	iconw->setActive( isActive() );
	connect( iconw, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( iconw, SIGNAL( doClose() ),
		 this, SLOT( close() ) );
	connect( iconw, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
    }
    iconw->setCaption( clientWidget()->caption() );
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    ((QWorkspace*)parentWidget())->minimizeClient( clientWidget() );
}

void QWorkspaceChild::showNormal()
{
    ((QWorkspace*)parentWidget())->normalizeClient( clientWidget() );
}

bool QWorkspaceChild::close( bool forceKill )
{
    if (iconw) {
	delete iconw->parentWidget();
	iconw = 0;
    }
    return QWidget::close( forceKill );
}

void QWorkspaceChild::clientDestroyed()
{
    close( TRUE );
}
