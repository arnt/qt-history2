/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Implementation of the QWorkspace class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qworkspace.h"
#if QT_FEATURE_WIDGETS
#include "qapplication.h"
#include "qobjectlist.h"
#include "qlayout.h"
#include "qtoolbutton.h"
#include "qlabel.h"
#include "qvbox.h"
#include "qaccel.h"
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qguardedptr.h"


// REVISED: arnt

/*!
  \class QWorkspace qworkspace.h
  \brief The QWorkspace widget provides a workspace window that can
  contain decorated windows, e.g. for MDI.

  \ingroup application
  \ingroup organizers

  An MDI application has one main window with a menu bar.  The central
  widget of this window is a workspace.  The workspace itself contains
  zero, one or more document windows, each of which displays a
  document.

  The workspace itself is an ordinary Qt widget.  It has a standard
  constructor that takes a parent widget and an object name.  The
  parent window is usually a QMainWindow, but it need not be.

  Document windows (alias MDI windows) are also ordinary Qt widgets,
  that have the workspace as parent widget.  When you call show(),
  hide(), showMaximized(), setCaption(), etc on a document window, it
  is shown, hidden etc. with a frame, caption, icon and icon text,
  just as you'd expect.

  A document window becomes active when it gets the keyboard focus.
  You can activate it using setFocus(), and the user can activate it
  by moving focus in the normal ways.  The workspace emits a signal
  windowActivated() when it detects the activation change, and the
  function activeWindow() always returns a pointer to the active
  document window.

  The convenience function windowList() returns a list of all document
  windows.  This is useful to create a popup menu "&Windows" on the
  fly, for example.

  QWorkspace provides two built-in layout strategies for child
  windows, cascade() and tile().  Both are slots, so you can easily
  connect menu entries to them.

  In case the toplevel window contains a menu bar and a document
  window is maximized, QWorkspace moves the document window's
  minimize, restore and close buttons from the document window's frame
  to the workspace window's menu bar, and inserts a window operations
  menu on the extreme left of the menu bar.

  \warning User interface research indicates that most users have
  problems with MDI applications.  Use this class cautiously.
*/


#if defined(_WS_WIN_)
#include "qt_windows.h"
const bool win32 = TRUE;
#define TITLEBAR_HEIGHT 18
#define TITLEBAR_SEPARATION 2
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 14
#define BORDER 2
#define RANGE 16
#define OFFSET 20

static const char * const close_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"....##....##....",
".....##..##.....",
"......####......",
".......##.......",
"......####......",
".....##..##.....",
"....##....##....",
"................",
"................",
"................",
"................",
"................"};

static const char * const maximize_xpm[]={
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"...#########....",
"...#########....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#########....",
"................",
"................",
"................",
"................"};


static const char * const minimize_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"....######......",
"....######......",
"................",
"................",
"................",
"................"};

static const char * const normalize_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
".....######.....",
".....######.....",
".....#....#.....",
"...######.#.....",
"...######.#.....",
"...#....###.....",
"...#....#.......",
"...#....#.......",
"...######.......",
"................",
"................",
"................",
"................"};


#else // !_WS_WIN_

const bool win32 = FALSE;
#define TITLEBAR_HEIGHT 18
#define TITLEBAR_SEPARATION 2
#define BUTTON_WIDTH 18
#define BUTTON_HEIGHT 18
#define BORDER 2
#define RANGE 16
#define OFFSET 20

static const char * const close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
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

static const char * const maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
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


static const char * const minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
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

static const char * const normalize_xpm[] = {
"16 16 3 1",
" 	s None	c None",
".	c #ffffff",
"X	c #707070",
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

#endif // !_WS_WIN_




static bool resizeHorizontalDirectionFixed = FALSE;
static bool resizeVerticalDirectionFixed = FALSE;

class Q_EXPORT QWorkspaceChildTitleBar : public QWidget
{
    Q_OBJECT
public:
    QWorkspaceChildTitleBar (QWorkspace* w, QWidget* window, QWidget* parent, const char* name=0, bool iconMode = FALSE );
    ~QWorkspaceChildTitleBar();

    bool isActive() const;

    QSize sizeHint() const;

 public slots:
    void setActive( bool );
    void setText( const QString& title );
    void setIcon( const QPixmap& icon );

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

protected:
    void resizeEvent( QResizeEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QToolButton* closeB;
    QToolButton* maxB;
    QToolButton* iconB;
    QLabel* titleL;
    QLabel* iconL;
    bool buttonDown;
    QPoint moveOffset;
    QWorkspace* workspace;
    bool imode;
    bool act;
};


#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QGuardedPtr<QWorkspaceChildTitleBar>;
// MOC_SKIP_END
#endif

class Q_EXPORT QWorkspaceChild : public QFrame
{
    Q_OBJECT
public:
    QWorkspaceChild( QWidget* window,
		     QWorkspace *parent=0, const char *name=0 );
    ~QWorkspaceChild();

    void setActive( bool );
    bool isActive() const;

    void adjustToFullscreen();

    QWidget* windowWidget() const;
    QWidget* iconWidget() const;

    void doResize();
    void doMove();

signals:
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void childEvent( QChildEvent* );
    void keyPressEvent( QKeyEvent * );

    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QWidget * childWidget;
    QWidget * lastfocusw;
    bool buttonDown;
    enum MousePosition {
	Nowhere,
	TopLeft, BottomRight, BottomLeft, TopRight,
	Top, Bottom, Left, Right,
	Center
    };
    MousePosition mode;
    bool moveResizeMode;
    void setMouseCursor( MousePosition m );
    bool isMove() const {
	return moveResizeMode && mode == Center;
    }
    bool isResize() const {
	return moveResizeMode && !isMove();
    }
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    bool act;
    QWorkspaceChildTitleBar* titlebar;
    QGuardedPtr<QWorkspaceChildTitleBar> iconw;
    QSize windowSize;

};


class QWorkspaceData {
public:
    QWorkspaceChild* active;
    QList<QWorkspaceChild> windows;
    QList<QWorkspaceChild> focus;
    QList<QWidget> icons;
    QWorkspaceChild* maxWindow;
    QRect maxRestore;
    QFrame* maxcontrols;

    int px;
    int py;
    QWidget *becomeActive;
    QPopupMenu* popup;
    int menuId;
    int controlId;
};

/*!
  Constructs a workspace with a \a parent and a \a name
 */
QWorkspace::QWorkspace( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QWorkspaceData;
    d->maxcontrols = 0;
    d->active = 0;
    d->maxWindow = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;
    d->popup = new QPopupMenu;
    d->menuId = -1;
    d->controlId = -1;
    connect( d->popup, SIGNAL( aboutToShow() ), this, SLOT(operationMenuAboutToShow() ));
    connect( d->popup, SIGNAL( activated(int) ), this, SLOT( operationMenuActivated(int) ) );
    d->popup->insertItem(tr("&Restore"), 1);
    d->popup->insertItem(tr("&Move"), 2);
    d->popup->insertItem(tr("&Size"), 3);
    d->popup->insertItem(tr("Mi&nimize"), 4);
    d->popup->insertItem(tr("Ma&ximize"), 5);
    d->popup->insertSeparator();
    d->popup->insertItem(tr("Close")+"\t"+QAccel::keyToString( CTRL+Key_W),
		  this, SLOT( closeActiveWindow() ) );

    QAccel* a = new QAccel( this );
    a->connectItem( a->insertItem( ALT + Key_Minus),
		    this, SLOT( showOperationMenu() ) );
    a->connectItem( a->insertItem( ALT + Key_W),
		    this, SLOT( closeActiveWindow() ) );
    a->connectItem( a->insertItem( ALT + Key_F6),
		    this, SLOT( activateNextWindow() ) );
    a->connectItem( a->insertItem( CTRL + Key_Tab),
		    this, SLOT( activateNextWindow() ) );
    a->connectItem( a->insertItem( CTRL +  ALT + Key_Tab),
		    this, SLOT( activateNextWindow() ) );
    a->connectItem( a->insertItem( ALT + SHIFT + Key_F6),
		    this, SLOT( activatePreviousWindow() ) );
    a->connectItem( a->insertItem( CTRL + SHIFT + Key_Tab),
		    this, SLOT( activatePreviousWindow() ) );
    a->connectItem( a->insertItem( CTRL +  ALT + SHIFT + Key_Tab),
		    this, SLOT( activatePreviousWindow() ) );

    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
}



/*!  Destroys the object and frees any allocated resources. */

QWorkspace::~QWorkspace()
{
    delete d;
}

/*!\reimp
*/
QSizePolicy QWorkspace::sizePolicy() const
{
    //### removeme 3.0
    return QWidget::sizePolicy();
}

/*!\reimp
 */
QSize QWorkspace::sizeHint() const
{
    QSize s( QApplication::desktop()->size() );
    return QSize( s.width()*2/3, s.height()*2/3);
}


/*! \reimp */

void QWorkspace::childEvent( QChildEvent * e)
{
    if (e->inserted() && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( !w->testWFlags( WStyle_NormalBorder | WStyle_DialogBorder )
	     || d->icons.contains( w ) )
	    return; 	    // nothing to do

	bool hasBeenHidden = w->testWState( WState_ForceHide );
	QWorkspaceChild* child = new QWorkspaceChild( w, this );
	child->installEventFilter( this );
	connect( child, SIGNAL( popupOperationMenu( const QPoint& ) ),
		 this, SLOT( popupOperationMenu( const QPoint& ) ) );
	connect( child, SIGNAL( showOperationMenu() ),
		 this, SLOT( showOperationMenu() ) );
	d->windows.append( child );
	d->focus.append( child );
	place( child );
	child->raise();
	if ( hasBeenHidden )
	    w->hide();
	else if ( !isVisible() ) // that's a case were we don't receive a showEvent in time. Tricky.
		child->show();
	activateWindow( w );
    } else if (e->removed() ) {
	if ( d->windows.contains( (QWorkspaceChild*)e->child() ) ) {
	    d->windows.removeRef( (QWorkspaceChild*)e->child() );
	    d->focus.removeRef( (QWorkspaceChild*)e->child() );
	    if ( d->windows.isEmpty() )
		hideMaximizeControls();
	    if( e->child() == d->active )
		d->active = 0;

	    if (  !d->windows.isEmpty() ) {
		if ( e->child() == d->maxWindow  ) {
		    d->maxWindow = 0;
		    maximizeWindow( d->windows.first()->windowWidget() );
		} else {
		    activateWindow( d->windows.first()->windowWidget() );
		}
	    } else if ( e->child() == d->maxWindow )
		d->maxWindow = 0;
	}
    }
}



void QWorkspace::activateWindow( QWidget* w, bool change_focus )
{
    if ( !isVisible() ) {
	d->becomeActive = w;
	return;
    }

    if ( d->active && d->active->windowWidget() == w )
	return;

    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	c->setActive( c->windowWidget() == w );
	if (c->windowWidget() == w)
	    d->active = c;
    }

    if (!d->active)
	return;

    if ( d->maxWindow && d->maxWindow != d->active )
	maximizeWindow( d->active->windowWidget() );

    d->active->raise();

    if ( change_focus ) {
	d->focus.removeRef( d->active );
	d->focus.append( d->active );
    }
    emit windowActivated( w );
}


/*!
  Returns the active window, or 0 if no window is active.
 */
QWidget* QWorkspace::activeWindow() const
{
    return d->active?d->active->windowWidget():0;
}


void QWorkspace::place( QWidget* w)
{
    int tx,ty;
    QRect maxRect = rect();
    if (d->px < maxRect.x())
	d->px = maxRect.x();
    if (d->py < maxRect.y())
	d->py = maxRect.y();

    d->px += OFFSET;
    d->py += 2*OFFSET;

    if (d->px > maxRect.width()/2)
	d->px = maxRect.x() + OFFSET;
    if (d->py > maxRect.height()/2)
	d->py = maxRect.y() + OFFSET;
    tx = d->px;
    ty = d->py;
    if (tx + w->width() > maxRect.right()){
	tx = maxRect.right() - w->width();
	if (tx < 0)
	    tx = 0;
	d->px = maxRect.x();
    }
    if (ty + w->height() > maxRect.bottom()){
	ty = maxRect.bottom() - w->height();
	if (ty < 0)
	    ty = 0;
	d->py = maxRect.y();
    }
    w->move( tx, ty );
}


void QWorkspace::insertIcon( QWidget* w )
{
    if ( !w || d->icons.contains( w ) )
	return;
    d->icons.append( w );
    if (w->parentWidget() != this )
	w->reparent( this, 0, QPoint(0,0), FALSE);
    layoutIcons();
    if ( isVisibleTo( parentWidget() ) )
	w->show();

}


void QWorkspace::removeIcon( QWidget* w)
{
    if ( !d->icons.contains( w ) )
	return;
    d->icons.remove( w );
    w->hide();
 }

/*! \reimp  */
void QWorkspace::resizeEvent( QResizeEvent * )
{
    if ( d->maxWindow )
	d->maxWindow->adjustToFullscreen();
    layoutIcons();
}

/*! \reimp */
void QWorkspace::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    if ( d->becomeActive )
	activateWindow( d->becomeActive );
    else if ( d->windows.count() > 0 && !d->active )
	activateWindow( d->windows.first()->windowWidget() );
}

void QWorkspace::layoutIcons()
{
    int x = 0;
    int y = height();
    for (QWidget* w = d->icons.first(); w ; w = d->icons.next() ) {

	if ( x > 0 && x + w->width() > width() ){
	    x = 0;
	    y -= w->height();
	}

	w->move(x, y-w->height());
	x = w->geometry().right();
    }
}

void QWorkspace::minimizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	c->hide();
	insertIcon( c->iconWidget() );
    }
}

void QWorkspace::normalizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	if ( c == d->maxWindow ) {
	    c->setGeometry( d->maxRestore );
	    d->maxWindow = 0;
	}
	else {
	    removeIcon( c->iconWidget() );
	    c->show();
	}
	hideMaximizeControls();
    }
}

void QWorkspace::maximizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );

    if ( c ) {
	if (d->icons.contains(c->iconWidget()) )
	    normalizeWindow( w );
	QRect r( c->geometry() );
	c->adjustToFullscreen();
	c->show();
	c->raise();
	if ( d->maxWindow && d->maxWindow != c ) {
	    d->maxWindow->setGeometry( d->maxRestore );
	}
	if ( d->maxWindow != c ) {
	    d->maxWindow = c;
	    d->maxRestore = r;
	}

	activateWindow( w);
	showMaximizeControls();
    }
}

void QWorkspace::showWindow( QWidget* w)
{
    if ( d->maxWindow )
	maximizeWindow( w );
    else
	normalizeWindow( w );
}


QWorkspaceChild* QWorkspace::findChild( QWidget* w)
{
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	if (c->windowWidget() == w)
	    return c;
    }
    return 0;
}

/*!
  Returns a list of all windows.
 */
QWidgetList QWorkspace::windowList() const
{
    QWidgetList windows;
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	if ( c->windowWidget()->isVisibleTo( c ) )
	    windows.append( c->windowWidget() );
    }
    return windows;
}

/*!\reimp*/
bool QWorkspace::eventFilter( QObject *o, QEvent * e)
{
    switch ( e->type() ) {
    case QEvent::Hide:
    case QEvent::HideToParent:
	if ( d->maxWindow == o && d->maxWindow->testWState(WState_ForceHide)) {
	    d->maxWindow->setGeometry( d->maxRestore );
	    d->maxWindow = 0;
	    hideMaximizeControls();
	}
	break;
    default:
	break;
    }
    return QWidget::eventFilter( o, e);
}

void QWorkspace::showMaximizeControls()
{

    QObjectList * l = topLevelWidget()->queryList( "QMenuBar", 0,
						   FALSE, TRUE );
    QMenuBar * b = 0;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    delete l;

    if ( !b )
	return;

    if ( !d->maxcontrols ) {
	d->maxcontrols = new QFrame( topLevelWidget() );
	if ( !win32 )
	    d->maxcontrols->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	QHBoxLayout* l = new QHBoxLayout( d->maxcontrols,
					  d->maxcontrols->frameWidth(), 0 );
	QToolButton* iconB = new QToolButton( d->maxcontrols, "iconify" );
	l->addWidget( iconB );
	iconB->setFocusPolicy( NoFocus );
	iconB->setIconSet( QPixmap( (const char **)minimize_xpm ));
 	iconB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( iconB, SIGNAL( clicked() ),
		 this, SLOT( minimizeActiveWindow() ) );
	QToolButton* restoreB = new QToolButton( d->maxcontrols, "restore" );
	l->addWidget( restoreB );
	restoreB->setFocusPolicy( NoFocus );
	restoreB->setIconSet( QPixmap( (const char **)normalize_xpm ));
 	restoreB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( restoreB, SIGNAL( clicked() ),
		 this, SLOT( normalizeActiveWindow() ) );

	l->addSpacing( 2 );
	QToolButton* closeB = new QToolButton( d->maxcontrols, "close" );
	l->addWidget( closeB );
	closeB->setFocusPolicy( NoFocus );
	closeB->setIconSet( QPixmap( (const char **)close_xpm ) );
 	closeB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( closeB, SIGNAL( clicked() ),
		 this, SLOT( closeActiveWindow() ) );

	if ( !win32 ) {
	    iconB->setAutoRaise( TRUE );
	    restoreB->setAutoRaise( TRUE );
	    closeB->setAutoRaise( TRUE );
	}

	d->maxcontrols->setFixedSize( 3* BUTTON_WIDTH+2+2*d->maxcontrols->frameWidth(),
				      BUTTON_HEIGHT+2*d->maxcontrols->frameWidth());
    }

    if ( d->controlId == -1 ) {
	QFrame* dmaxcontrols = d->maxcontrols;
	d->controlId = b->insertItem( dmaxcontrols, -1, b->count() );
    }
    if ( d->active && d->menuId == -1 ) {
	if ( d->active->windowWidget()->icon() ) {
	    const QPixmap* pm = d->active->windowWidget()->icon();
	    QPopupMenu* pu = d->popup;
	    d->menuId = b->insertItem( *pm, pu, -1, 0 );
	} else {
	    QPixmap pm(10,12);
	    pm.fill( white );
	    QPopupMenu* pu = d->popup;
	    d->menuId = b->insertItem( pm, pu, -1, 0 );
	}
    }
}


void QWorkspace::hideMaximizeControls()
{
    QObjectList * l = topLevelWidget()->queryList( "QMenuBar", 0, FALSE, TRUE );
    QMenuBar * b = 0;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    delete l;
    if ( b ) {
	int mi = d->menuId;
	if ( mi != -1 )
	    b->removeItem( mi );
	int ci = d->controlId;
	if ( ci != -1 )
	    b->removeItem( ci );
    }
    d->maxcontrols = 0;
    d->menuId = -1;
    d->controlId = -1;
}

void QWorkspace::closeActiveWindow()
{
    bool isMax = ( d->maxWindow != 0);

    QWidget* w = activeWindow();
    if ( w )
	w->close();

    activateNextWindow();

    if (d->active && isMax && !d->active->testWState( WState_ForceHide ))
	d->active->showMaximized();
}

void QWorkspace::normalizeActiveWindow()
{
    if  ( d->active )
	d->active->showNormal();
}

void QWorkspace::minimizeActiveWindow()
{
    if  ( d->active )
	d->active->showMinimized();
}

void QWorkspace::showOperationMenu()
{
    if  ( !d->active )
	return;
    QPoint p( d->active->windowWidget()->mapToGlobal( QPoint(0,0) ) );
    if ( !d->active->isVisible() ) {
	p = d->active->iconWidget()->mapToGlobal( QPoint(0,0) );
	p.ry() -= d->popup->sizeHint().height();
    }
    popupOperationMenu( p );
}

void QWorkspace::popupOperationMenu( const QPoint&  p)
{
    d->popup->popup( p );
}


void QWorkspace::operationMenuAboutToShow()
{
    for ( int i = 1; i < 6; i++ ) {
	bool enable = d->active != 0;
	d->popup->setItemEnabled( i, enable );
    }

    if ( !d->active )
	return;

    if ( d->active == d->maxWindow ) {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 5, FALSE );
    } else if ( d->active->isVisible() ){
	d->popup->setItemEnabled( 1, FALSE );
    } else {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 4, FALSE );
    }

    if (  !d->active->windowWidget()->testWFlags( WStyle_MinMax ) ) {
	d->popup->setItemEnabled( 4, FALSE );
	d->popup->setItemEnabled( 5, FALSE );
    }
}

void QWorkspace::operationMenuActivated( int a )
{
    if ( !d->active )
	return;
    switch ( a ) {
    case 1:
	d->active->showNormal();
	break;
    case 2:
	d->active->doMove();
	break;
    case 3:
	d->active->doResize();
	break;
    case 4:
	d->active->showMinimized();
	break;
    case 5:
	d->active->showMaximized();
	break;
    default:
	break;
    }
}

void QWorkspace::activateNextWindow()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	activateWindow( d->focus.first()->windowWidget(), FALSE );
	return;
    }

    int a = d->focus.find( d->active );
    if ( a <= 0 )
	a = d->focus.count()-1;
    else
	a--;
    activateWindow( d->focus.at( a )->windowWidget(), FALSE );
}

void QWorkspace::activatePreviousWindow()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	activateWindow( d->focus.last()->windowWidget(), FALSE );
	return;
    }

    int a = d->focus.find( d->active );
    if ( a < 0  || a >= int(d->focus.count())-1 )
	a = 0;
    else
	a++;
    activateWindow( d->focus.at( a )->windowWidget(), FALSE );
}


/*!
  \fn void QWorkspace::windowActivated( QWidget* w )

  This signal is emitted when the window widget \a w becomes active.

  \sa activeWindow(), windowList()
*/



/*!
  Arranges all child windows in a cascade pattern

  \sa tile()
 */
void QWorkspace::cascade()
{
    const int xoffset = TITLEBAR_HEIGHT * 2/3;
    const int yoffset = TITLEBAR_HEIGHT+2;

    int w = width() - d->windows.count() * xoffset;
    int h = height() - d->windows.count() * yoffset;
    int x = 0;
    int y = 0;

    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	c->showNormal();
	c->setGeometry( x, y, w, h );
	x += xoffset;
	y +=yoffset;
	c->raise();
    }
}

/*!
  Arranges all child windows in a tile pattern

  \sa cascade()
 */
void QWorkspace::tile()
{
    int rows = 1;
    int cols = 1;
    int n = (int) d->windows.count();
    while ( rows * cols < n ) {
	if ( cols <= rows )
	    cols++;
	else
	    rows++;
    }
    int add = cols * rows - n;
    bool* used = new bool[ cols*rows ];
    for ( int i = 0; i < rows*cols; i++ )
	used[i] = FALSE;

    int row = 0;
    int col = 0;
    int w = width() / cols;
    int h = height() / rows;
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	c->showNormal();
	used[row*cols+col] = TRUE;
	if ( add ) {
	    c->setGeometry( col*w, row*h, w, 2*h );
	    used[(row+1)*cols+col] = TRUE;
	    add--;
	} else {
	    c->setGeometry( col*w, row*h, w, h );
	}
	while( row < rows && col < cols && used[row*cols+col] ) {
	    col++;
	    if ( col == cols ) {
		col = 0;
		row++;
	    }
	}
    }
    delete [] used;
}


QWorkspaceChildTitleBar::QWorkspaceChildTitleBar (QWorkspace* w, QWidget* window, QWidget* parent,
						  const char* name, bool iconMode )
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder )
{
    workspace = w;
    buttonDown = FALSE;
    imode = iconMode;
    act = FALSE;

    titleL = new QLabel( this, "__workspace_child_title_bar" );
    titleL->setTextFormat( PlainText );
    titleL->setIndent( 2 );

    closeB = new QToolButton( this, "close" );
    closeB->setFocusPolicy( NoFocus );
    closeB->setIconSet( QPixmap( (const char **)close_xpm ) );
    closeB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( closeB, SIGNAL( clicked() ),
	     this, SIGNAL( doClose() ) ) ;
    maxB = new QToolButton( this, "maximize" );
    maxB->setFocusPolicy( NoFocus );
    maxB->setIconSet( QPixmap( (const char **)maximize_xpm ));
    maxB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( maxB, SIGNAL( clicked() ),
	     this, SIGNAL( doMaximize() ) );
    iconB = new QToolButton( this, "iconify" );
    iconB->setFocusPolicy( NoFocus );
    iconB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);

    if ( window && !window->testWFlags( WStyle_MinMax ) ) {
	iconB->hide();
	maxB->hide();
    }


    if ( !win32 ) {
	closeB->setAutoRaise( TRUE );
	maxB->setAutoRaise( TRUE );
	iconB->setAutoRaise( TRUE );
    }
    if ( imode ) {
	iconB->setIconSet( QPixmap( (const char **)normalize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doNormal() ) );
    }
    else {
	iconB->setIconSet( QPixmap( (const char **)minimize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doMinimize() ) );
    }

//     titleL->setMouseTracking( TRUE );
    titleL->installEventFilter( this );
    titleL->setAlignment( AlignLeft | AlignVCenter | SingleLine );
    QFont f = font();
    f.setBold( TRUE );
    titleL->setFont( f );

    iconL = new QLabel( this, "icon" );
    iconL->setAlignment( AlignCenter );
    iconL->setFocusPolicy( NoFocus );
    iconL->installEventFilter( this );
}

QWorkspaceChildTitleBar::~QWorkspaceChildTitleBar()
{
}

void QWorkspaceChildTitleBar::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = TRUE;
	moveOffset = mapToParent( e->pos() );
	emit doActivate();
    } else if ( e->button() == RightButton ) {
	emit doActivate();
	emit popupOperationMenu( e->globalPos() );
    }
}

void QWorkspaceChildTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
    }
}

void QWorkspaceChildTitleBar::mouseMoveEvent( QMouseEvent * e)
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


void QWorkspaceChildTitleBar::setText( const QString& title )
{
    titleL->setText( title );
}


void QWorkspaceChildTitleBar::setIcon( const QPixmap& icon )
{
    iconL->setPixmap( icon );
}


bool QWorkspaceChildTitleBar::eventFilter( QObject * o, QEvent * e)
{
    if ( o == titleL ) {
	if ( e->type() == QEvent::MouseButtonPress
	     || e->type() == QEvent::MouseButtonRelease
	     || e->type() == QEvent::MouseMove) {
	    QMouseEvent* me = (QMouseEvent*) e;
	    QMouseEvent ne( me->type(), titleL->mapToParent(me->pos()), me->globalPos(),
			    me->button(), me->state() );

	    if (e->type() == QEvent::MouseButtonPress )
		mousePressEvent( &ne );
	    else if (e->type() == QEvent::MouseButtonRelease )
		mouseReleaseEvent( &ne );
	    else
		mouseMoveEvent( &ne );
	}
	else if ( (e->type() == QEvent::MouseButtonDblClick) &&
		  ((QMouseEvent*)e)->button() == LeftButton ) {
	    if ( imode )
		emit doNormal();
	    else
		emit doMaximize();
	    return TRUE;
	}
    } else if ( o == iconL ) {
	if ( e->type() == QEvent::MouseButtonPress ) {
	    emit doActivate();
	    emit showOperationMenu();
	}
    }
    return FALSE;
}


void QWorkspaceChildTitleBar::resizeEvent( QResizeEvent * )
{ // NOTE: called with 0 pointer
    int bo = ( height()- BUTTON_HEIGHT) / 2;
    closeB->move( width() - BUTTON_WIDTH - bo, bo  );
    maxB->move( closeB->x() - BUTTON_WIDTH - bo, closeB->y() );
    iconB->move( maxB->x() - BUTTON_WIDTH, maxB->y() );
    iconL->setGeometry( 0, 0, BUTTON_WIDTH, height() );
    if ( win32 || (imode && !isActive()) ) {
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0 ),
				    rect().bottomRight() ) );
    } else {
	QWidget* ref = iconB->isVisibleTo( this ) ? iconB : closeB;
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0),
				    QPoint( ref->geometry().left() - 1, rect().bottom() ) ) );
    }

}


void QWorkspaceChildTitleBar::setActive( bool active )
{
    act = active;
    if ( active ) {
	if ( imode ){
	    iconB->show();
	    maxB->show();
	    closeB->show();
	}
	QColorGroup g = colorGroup();
	g.setColor( QColorGroup::Background,  colorGroup().color( QColorGroup::Highlight ) );
	g.setColor( QColorGroup::Foreground,  colorGroup().color( QColorGroup::HighlightedText) );
	if ( win32 ) {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	}
    } else {
	if ( imode ){
	    iconB->hide();
	    closeB->hide();
	    maxB->hide();
	}
	QColorGroup g = colorGroup();
	if ( win32 ) {
	    g.setColor( QColorGroup::Background,  colorGroup().color( QColorGroup::Dark ) );
	    g.setColor( QColorGroup::Foreground,  colorGroup().color( QColorGroup::Background) );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	    titleL->setFrameStyle( QFrame::NoFrame );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	}
    }
    if ( imode )
	resizeEvent( 0 );
}

bool QWorkspaceChildTitleBar::isActive() const
{
    return act;
}


QSize QWorkspaceChildTitleBar::sizeHint() const
{
    constPolish();
    return QSize( 196, QMAX( TITLEBAR_HEIGHT, fontMetrics().lineSpacing() ) );
}


QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent,
				  const char *name )
    : QFrame( parent, name,
	      WStyle_Customize | WStyle_NoBorder  | WDestructiveClose )
{
    mode = Nowhere;
    buttonDown = FALSE;
    setMouseTracking( TRUE );
    act = FALSE;
    iconw = 0;
    lastfocusw = 0;

    titlebar = new QWorkspaceChildTitleBar( parent, window, this );
    connect( titlebar, SIGNAL( doActivate() ),
	     this, SLOT( activate() ) );
    connect( titlebar, SIGNAL( doClose() ),
	     window, SLOT( close() ) );
    connect( titlebar, SIGNAL( doMinimize() ),
	     this, SLOT( showMinimized() ) );
    connect( titlebar, SIGNAL( doMaximize() ),
	     this, SLOT( showMaximized() ) );
    connect( titlebar, SIGNAL( popupOperationMenu( const QPoint& ) ),
	     this, SIGNAL( popupOperationMenu( const QPoint& ) ) );
    connect( titlebar, SIGNAL( showOperationMenu() ),
	     this, SIGNAL( showOperationMenu() ) );

    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    setMinimumSize( 128, 96 );

    childWidget = window;
    if (!childWidget)
	return;

    titlebar->setText( childWidget->caption() );
    if( childWidget->icon() )
	titlebar->setIcon( *childWidget->icon() );

    int th = titlebar->sizeHint().height();

    bool hasBeenResized = childWidget->testWState( WState_Resized );
    childWidget->reparent( this, QPoint( contentsRect().x()+BORDER,
					 th + BORDER + TITLEBAR_SEPARATION +
					 contentsRect().y() ) );
    if ( !hasBeenResized ) {
	QSize cs = childWidget->sizeHint();
	QSize s( cs.width() + 2*frameWidth() + 2*BORDER,
		 cs.height() + 3*frameWidth() + th +TITLEBAR_SEPARATION +
		 2*BORDER );
	resize( s );
    } else {
	resize( childWidget->width() + 2*frameWidth() + 2*BORDER,
		childWidget->height() + 2*frameWidth() + th +2*BORDER);
    }

    childWidget->installEventFilter( this );
}

QWorkspaceChild::~QWorkspaceChild()
{
}


void QWorkspaceChild::resizeEvent( QResizeEvent * )
{
    QRect r = contentsRect();
    int th = titlebar->sizeHint().height();
    titlebar->setGeometry( r.x() + BORDER, r.y() + BORDER,
			   r.width() - 2*BORDER, th+1);

    if (!childWidget)
	return;

    QRect cr( r.x() + BORDER, r.y() + BORDER + TITLEBAR_SEPARATION + th,
	      r.width() - 2*BORDER,
	      r.height() - 2*BORDER - TITLEBAR_SEPARATION - th);
    windowSize = cr.size();
    childWidget->setGeometry( cr );
}

void QWorkspaceChild::activate()
{
    ((QWorkspace*)parentWidget())->activateWindow( windowWidget() );
}


bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{

    if ( !isActive() && ( e->type() == QEvent::MouseButtonPress ||
			  e->type() == QEvent::FocusIn ) )
	activate();

    // for all widgets except the window, we that's the only thing we
    // process, and if we have no childWidget we skip totally
    if ( o != childWidget || childWidget == 0 )
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Show:
	if ( isVisibleTo( parentWidget() ) )
	    break;
	if (( (QShowEvent*)e)->spontaneous() )
	    break;
	// FALL THROUGH
    case QEvent::ShowToParent:
	((QWorkspace*)parentWidget())->showWindow( windowWidget() );
	break;
    case QEvent::ShowMaximized:
	showMaximized();
	break;
    case QEvent::ShowMinimized:
	showMinimized();
	break;
    case QEvent::ShowNormal:
	showNormal();
	break;
    case QEvent::Hide:
    case QEvent::HideToParent:
	if ( !childWidget->isVisibleTo( this ) ) {
	    QWidget * w = iconw;
	    if ( w )
		w = w->parentWidget();
	    if ( w ) {
		((QWorkspace*)parentWidget())->removeIcon( w );
		delete w;
	    }
	    hide();
	}
	break;
    case QEvent::CaptionChange:
	titlebar->setText( childWidget->caption() );
	break;
    case QEvent::IconChange:
	if ( childWidget->icon() ) {
	    titlebar->setIcon( *childWidget->icon() );
	} else {
	    QPixmap pm;
	    titlebar->setIcon( pm );
	}
	break;
    case QEvent::LayoutHint:
	//layout()->activate();
	break;
    case QEvent::Resize:
	{
	    QResizeEvent* re = (QResizeEvent*)e;
	    if ( re->size() != windowSize ) {
		int th = titlebar->sizeHint().height();
		QSize s( re->size().width() + 2*frameWidth() + 2*BORDER,
			 re->size().height() + 2*frameWidth() + th +
			 TITLEBAR_SEPARATION+2*BORDER );
		resize( s );
	    }
	}
	break;
    default:
	break;
    }

    return FALSE;
}


void QWorkspaceChild::childEvent( QChildEvent*  e)
{
    if ( e->type() == QEvent::ChildRemoved && e->child() == childWidget ) {
	childWidget = 0;
	if ( iconw ) {
	    ((QWorkspace*)parentWidget())->removeIcon( iconw->parentWidget() );
	    delete iconw->parentWidget();
	}
	close();
    }
}

void QWorkspaceChild::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	activate();
	mouseMoveEvent( e );
	buttonDown = TRUE;
	moveOffset = e->pos();
	invertedMoveOffset = rect().bottomRight() - e->pos();
    }
}

void QWorkspaceChild::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
	releaseKeyboard();
    }
}

void QWorkspaceChild::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown ) {
	if ( e->pos().y() <= RANGE && e->pos().x() <= RANGE)
	    mode = TopLeft;
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() >= width()-RANGE)
	    mode = BottomRight;
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() <= RANGE)
	    mode = BottomLeft;
	else if ( e->pos().y() <= RANGE && e->pos().x() >= width()-RANGE)
	    mode = TopRight;
	else if ( e->pos().y() <= RANGE )
	    mode = Top;
	else if ( e->pos().y() >= height()-RANGE )
	    mode = Bottom;
	else if ( e->pos().x() <= RANGE )
	    mode = Left;
	else if (  e->pos().x() >= width()-RANGE )
	    mode = Right;
	else
	    mode = Center;
	setMouseCursor( mode );
	return;
    }

    if ( testWState(WState_ConfigPending) )
 	return;

    QPoint globalPos = parentWidget()->mapFromGlobal( e->globalPos() );
    QPoint p = globalPos + invertedMoveOffset;
    QPoint pp = globalPos - moveOffset;

    int mw = QMAX(childWidget->minimumSizeHint().width(),
		  childWidget->minimumWidth()) + 2 * BORDER + 2 * frameWidth();
    int mh = QMAX(childWidget->minimumSizeHint().height(),
		  childWidget->minimumHeight())
	     + 2 * BORDER +  2 * frameWidth() + TITLEBAR_SEPARATION
	     + titlebar->sizeHint().height();

    QSize mpsize( geometry().right() - pp.x() + 1,
		  geometry().bottom() - pp.y() + 1 );
    mpsize = mpsize.expandedTo( minimumSize() ).expandedTo( QSize(mw, mh) );
    QPoint mp( geometry().right() - mpsize.width() + 1,
	       geometry().bottom() - mpsize.height() + 1 );

    QRect geom = geometry();

    switch ( mode ) {
    case TopLeft:
	geom =  QRect( mp, geometry().bottomRight() ) ;
	break;
    case BottomRight:
	geom =  QRect( geometry().topLeft(), p ) ;
	break;
    case BottomLeft:
	geom =  QRect( QPoint(mp.x(), geometry().y() ), QPoint( geometry().right(), p.y()) ) ;
	break;
    case TopRight:
	geom =  QRect( QPoint(geometry().x(), mp.y() ), QPoint( p.x(), geometry().bottom()) ) ;
	break;
    case Top:
	geom =  QRect( QPoint( geometry().left(), mp.y() ), geometry().bottomRight() ) ;
	break;
    case Bottom:
	geom =  QRect( geometry().topLeft(), QPoint( geometry().right(), p.y() ) ) ;
	break;
    case Left:
	geom =  QRect( QPoint( mp.x(), geometry().top() ), geometry().bottomRight() ) ;
	break;
    case Right:
	geom =  QRect( geometry().topLeft(), QPoint( p.x(), geometry().bottom() ) ) ;
	break;
    case Center:
	geom.moveTopLeft( pp );
	break;
    default:
	break;
    }

    geom = QRect( geom.topLeft(), geom.size().expandedTo( minimumSize() ).expandedTo( QSize(mw,mh) ) );
    if ( parentWidget()->rect().intersects( geom ) )
	setGeometry( geom );

#ifdef _WS_WIN_
    MSG msg;
    while(PeekMessage( &msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ))
	;
#endif
    QApplication::syncX();
}



void QWorkspaceChild::enterEvent( QEvent * )
{
}

void QWorkspaceChild::leaveEvent( QEvent * )
{
    if ( !buttonDown )
	setCursor( arrowCursor );
}


static bool isChildOf( QWidget * child, QWidget * parent )
{
    if ( !parent || !child )
	return FALSE;
    QWidget * w = child;
    while( w && w != parent )
	w = w->parentWidget();
    return w != 0;
}


void QWorkspaceChild::setActive( bool b )
{
    act = b;

    titlebar->setActive( act );
    if ( iconw )
	iconw->setActive( act );

    QObjectList* ol = childWidget->queryList( "QWidget" );
    if ( act ) {
	QObject *o;
	for ( o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	bool hasFocus = isChildOf( focusWidget(), childWidget );
	if ( !hasFocus ) {
	    if ( lastfocusw && ol->contains( lastfocusw ) &&
		 lastfocusw->focusPolicy() != NoFocus ) {
		// this is a bug if lastfocusw has been deleted, a new
		// widget has been created, and the new one is a child
		// of the same window as the old one.  but even though
		// it's a bug the behaviour is reasonable :)
		lastfocusw->setFocus();
	    } else if ( childWidget->focusPolicy() != NoFocus ) {
		childWidget->setFocus();
	    } else {
		// find something, anything, that accepts focus, and use that.
		o = ol->first();
		while( o && ((QWidget*)o)->focusPolicy() == NoFocus )
		    o = ol->next();
		if ( o )
		    ((QWidget*)o)->setFocus();
	    }
	}
    } else {
	lastfocusw = 0;
	if ( isChildOf( focusWidget(), childWidget ) )
	    lastfocusw = focusWidget();
	QObject * o;
	for ( o = ol->first(); o; o = ol->next() ) {
	    o->removeEventFilter( this );
	    o->installEventFilter( this );
	}
    }
    delete ol;
}

bool QWorkspaceChild::isActive() const
{
    return act;
}

QWidget* QWorkspaceChild::windowWidget() const
{
    return childWidget;
}


QWidget* QWorkspaceChild::iconWidget() const
{
    if ( !iconw ) {
	QWorkspaceChild* that = (QWorkspaceChild*) this;
	QVBox* vbox = new QVBox;
	vbox->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	vbox->resize( 196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth() );
	that->iconw = new QWorkspaceChildTitleBar( (QWorkspace*)parentWidget(), 0, vbox, 0, TRUE );
	iconw->setActive( isActive() );
	connect( iconw, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( iconw, SIGNAL( doClose() ),
		 windowWidget(), SLOT( close() ) );
	connect( iconw, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
	connect( iconw, SIGNAL( doMaximize() ),
		 this, SLOT( showMaximized() ) );
	connect( iconw, SIGNAL( popupOperationMenu( const QPoint& ) ),
		 this, SIGNAL( popupOperationMenu( const QPoint& ) ) );
	connect( iconw, SIGNAL( showOperationMenu() ),
		 this, SIGNAL( showOperationMenu() ) );
    }
    iconw->setText( windowWidget()->caption() );
    if ( windowWidget()->icon() )
	iconw->setIcon( *windowWidget()->icon() );
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    ((QWorkspace*)parentWidget())->minimizeWindow( windowWidget() );
}

void QWorkspaceChild::showMaximized()
{
    ((QWorkspace*)parentWidget())->maximizeWindow( windowWidget() );
}

void QWorkspaceChild::showNormal()
{
    ((QWorkspace*)parentWidget())->normalizeWindow( windowWidget() );
    if (iconw) {
	((QWorkspace*)parentWidget())->removeIcon( iconw->parentWidget() );
	delete iconw->parentWidget();
    }
}


void QWorkspaceChild::adjustToFullscreen()
{
    setGeometry( -childWidget->x(), -childWidget->y(),
		 parentWidget()->width() + width() - childWidget->width(),
		 parentWidget()->height() + height() - childWidget->height() );
}


/*!
  Sets an appropriate cursor shape for the logical mouse position \a m

  \sa QWidget::setCursor()
 */
void QWorkspaceChild::setMouseCursor( MousePosition m )
{
    switch ( m ) {
    case TopLeft:
    case BottomRight:
	setCursor( sizeFDiagCursor );
	break;
    case BottomLeft:
    case TopRight:
	setCursor( sizeBDiagCursor );
	break;
    case Top:
    case Bottom:
	setCursor( sizeVerCursor );
	break;
    case Left:
    case Right:
	setCursor( sizeHorCursor );
	break;
    default:
	setCursor( arrowCursor );
	break;
    }
}

void QWorkspaceChild::keyPressEvent( QKeyEvent * e )
{
    if ( !isMove() && !isResize() )
	return;
    bool is_control = e->state() & ControlButton;
    int delta = is_control?1:8;
    QPoint pos = QCursor::pos();
    switch ( e->key() ) {
    case Key_Left:
	pos.rx() -= delta;
	if ( pos.x() <= QApplication::desktop()->geometry().left() ) {
	    if ( mode == TopLeft || mode == BottomLeft ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomRight )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = TopLeft;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Right:
	pos.rx() += delta;
	if ( pos.x() >= QApplication::desktop()->geometry().right() ) {
	    if ( mode == TopRight || mode == BottomRight ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = BottomRight;
	    else if ( mode == TopLeft )
		mode = TopRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Up:
	pos.ry() -= delta;
	if ( pos.y() <= QApplication::desktop()->geometry().top() ) {
	    if ( mode == TopLeft || mode == TopRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = TopLeft;
	    else if ( mode == BottomRight )
		mode = TopRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Down:
	pos.ry() += delta;
	if ( pos.y() >= QApplication::desktop()->geometry().bottom() ) {
	    if ( mode == BottomLeft || mode == BottomRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == TopLeft )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = BottomRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Space:
    case Key_Return:
    case Key_Enter:
	moveResizeMode = FALSE;
	releaseMouse();
	releaseKeyboard();
	buttonDown = FALSE;
	break;
    default:
	return;
    }
    QCursor::setPos( pos );
}


void QWorkspaceChild::doResize()
{
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = mapFromGlobal( QCursor::pos() );
    if ( moveOffset.x() < width()/2) {
	if ( moveOffset.y() < height()/2)
	    mode = TopLeft;
	else
	    mode = BottomLeft;
    } else {
	if ( moveOffset.y() < height()/2)
	    mode = TopRight;
	else
	    mode = BottomRight;
    }
    invertedMoveOffset = rect().bottomRight() - moveOffset;
    setMouseCursor( mode );
    grabMouse( cursor()  );
    grabKeyboard();
    resizeHorizontalDirectionFixed = FALSE;
    resizeVerticalDirectionFixed = FALSE;
}

void QWorkspaceChild::doMove()
{
    mode = Center;
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = mapFromGlobal( QCursor::pos() );
    invertedMoveOffset = rect().bottomRight() - moveOffset;
    grabMouse( arrowCursor );
    grabKeyboard();
}

#include "qworkspace.moc"
#endif
