/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qworkspace.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qhbox.h"
#include "qtoolbutton.h"
#include "qlabel.h"
#include "qvbox.h"

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


static const char * close_xpm[] = {
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

static const char*maximize_xpm[]={
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


static const char * minimize_xpm[] = {
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

static const char * normalize_xpm[] = {
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

static const char * close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
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

static const char * maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
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


static const char * minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
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

static const char * normalize_xpm[] = {
"16 16 3 1",
" 	s None	c None",
".	c white",
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




class Q_EXPORT QWorkspaceChildTitleBar : public QWidget
{
    Q_OBJECT
public:
    QWorkspaceChildTitleBar (QWorkspace* w, QWidget* parent, const char* name=0, bool iconMode = FALSE );
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
    int mode;
    QPoint moveOffset;
    QWorkspace* workspace;
    bool imode;
    bool act;
};


class Q_EXPORT QWorkspaceChild : public QFrame
{
    Q_OBJECT
public:
    QWorkspaceChild( QWidget* client, QWorkspace *parent=0, const char *name=0 );
    ~QWorkspaceChild();

    void setActive( bool );
    bool isActive() const;

    void adjustToFullscreen();

    QWidget* clientWidget() const;
    QWidget* iconWidget() const;

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

    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QWidget* clientw;
    bool buttonDown;
    int mode;
    QPoint moveOffset;
    bool act;
    QWorkspaceChildTitleBar* titlebar;
    QWorkspaceChildTitleBar* iconw;
    QSize clientSize;

};


class QWorkspaceData {
public:
    QWorkspaceChild* active;
    QList<QWorkspaceChild> windows;
    QList<QWidget> icons;
    QWorkspaceChild* maxClient;
    QRect maxRestore;
    QHBox* maxcontrols;
    bool wantsmaxcontrols;

    int px;
    int py;
    QWidget *becomeActive;
};

// NOT REVISED

/*!
  \class QWorkspace qworkspace.h

  \brief The QWorkspace widget provides a workspace that can contain
  decorated windows as opposed to frameless child widgets.  QWorkspace
  makes it easy to implement a multidocument interface (MDI).

  \ingroup realwidgets

  An MDI application has one main window that has a menubar. The
  central widget of this main window is a workspace. The workspace
  itself contains zero, one or several document windows, each of which
  displays a document.

  The menubar and the toolbars act as a roadmap to the
  application. You get to keep the same map all the time, even if you
  open three different documents in three different child windows and
  switch around among them.

  The workspace itself is an ordinary Qt widget. It has a standard
  constructor that takes a parent widget and an object name.  Document
  windows, so-called MDI windows, are also ordinary Qt widgets.  They
  may even be main windows itself, in case your application design
  requires specific toolbars or statusbars for them. The only special
  thing about them is that you create them with the workspace as
  parent widget. The rest of the magic happens behind the scenes. All
  you have to do is call QWidget::show() or QWidget::showMaximized()
  (as you would do with normal toplevel windows) and the document
  window appears as MDI window inside the workspace.

  In addition to show, QWidget::hide(), QWidget::showMaximized(),
  QWidget::showNormal(), QWidget::showMinimized() also work as
  expected for the document windows.

  A document window becomes active when it gets the focus. This can be
  achieved by calling QWidget::setFocus(). The workspace emits a
  signal clientActivated() when it detects the activation change.  The
  active client itself can be obtained with activeClient().

  The convenience function clientList() returns a list of all document
  windows. This is especially useful to create a popup menu "&Windows"
  on the fly.

  If the user clicks on the frame of an MDI window, this window
  becomes active, i.e. it gets the focus. For that reason, all direct
  children of a QWorkspace have to be focus enabled. If your MDI
  window does not handle focus itself, use QWidget::setFocusProxy() to
  install a focus-enabled child widget as focus proxy.

  By default, the workspace generates minimize and restore buttons for
  maximized child windows, and places them in the top-right corner of
  the toplevel widgets, usually inside a menubar. This behaviour can
  be customized with setMaximizeControls().

  In general, modern GUI applications should be document-centric
  rather then application-centric. A single document interface (SDI)
  guarantees a bijective mapping between open documents and open
  windows on the screen. This makes the model very easy to understand
  and therefore the natural choice for applications targeted on
  inexperienced users. Typical examples are modern
  wordprocessors. Although most wordprocessors were MDI applications
  in the past, user interface studies showed that many users never
  really understood the model.

  If an application is supposed to be used mostly by experienced
  users, a multiple document interface may neverthless make sense.  A
  typical example is an integrated development environment (IDE). With
  an IDE, a document is a project. The project itself consists of an
  arbitrary number of subdocuments, mainly code files but also other
  data. MDI offers a good possibility to group these subdocuments
  together in one main window.  The menubar and the toolbars form a
  stable working context for the users to grasp and it is
  crystal-clear which subdocuments belong together. Furthermore, the
  user effort for window management tasks such as positioning,
  stacking and sizing is significantly reduced.

  An alternative to MDI with QWorkspace is a multipane structure. This
  can be achived by tiling the main window into separate panes with a
  \l QSplitter.

*/


/*!
  Creates a workspace with a \a parent and a \a name
 */
QWorkspace::QWorkspace( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QWorkspaceData;
    d->wantsmaxcontrols = TRUE;
    d->maxcontrols = 0;
    d->active = 0;
    d->maxClient = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;

    topLevelWidget()->installEventFilter( this );

}

/*!
  Destructor.
 */
QWorkspace::~QWorkspace()
{
    delete d;
}

/*!\reimp
 */
void QWorkspace::childEvent( QChildEvent * e)
{

    if (e->inserted() && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( w->testWFlags( WStyle_Customize | WStyle_NoBorder )
	      || d->icons.contains( w ) )
	    return; 	    // nothing to do

	QWorkspaceChild* child = new QWorkspaceChild( w, this );
	d->windows.append( child );
	place( child );
	child->raise();
    } else if (e->removed() ) {
	if ( d->windows.contains( (QWorkspaceChild*)e->child() ) ) {
	    d->windows.remove( (QWorkspaceChild*)e->child() );
	    if ( d->windows.isEmpty() )
		hideMaximizeControls();
	    if ( d->icons.contains( (QWidget*)e->child() ) ){
		d->icons.remove( (QWidget*)e->child() );
		layoutIcons();
	    }
	    if( e->child() == d->active )
		d->active = 0;

	    if (  !d->windows.isEmpty() ) {
		if ( e->child() == d->maxClient  ) {
		    d->maxClient = 0;
		    maximizeClient( d->windows.first()->clientWidget() );
		} else {
		    activateClient( d->windows.first()->clientWidget() );
		}
	    } else if ( e->child() == d->maxClient )
		d->maxClient = 0;
	}
    }
}



void QWorkspace::activateClient( QWidget* w)
{
    if ( !isVisible() ) {
	d->becomeActive = w;
	return;
    }

    if ( d->active && d->active->clientWidget() == w )
	return;

    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	c->setActive( c->clientWidget() == w );
	if (c->clientWidget() == w)
	    d->active = c;
    }

    if (!d->active)
	return;

    if ( d->maxClient && d->maxClient != d->active )
	maximizeClient( d->active->clientWidget() );

    d->active->raise();

    emit clientActivated( w );
}


/*!
  Returns the active client, or 0 if no client is active.
 */
QWidget* QWorkspace::activeClient() const
{
    return d->active?d->active->clientWidget():0;
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
	d->px =  maxRect.x() + OFFSET;
    if (d->py > maxRect.height()/2)
	d->py =  maxRect.y() + OFFSET;
    tx = d->px;
    ty = d->py;
    if (tx + w->width() > maxRect.right()){
	tx = maxRect.right() - w->width();
	if (tx < 0)
	    tx = 0;
	d->px =  maxRect.x();
    }
    if (ty + w->height() > maxRect.bottom()){
	ty = maxRect.bottom() - w->height();
	if (ty < 0)
	    ty = 0;
	d->py =  maxRect.y();
    }
    w->move( tx, ty );
}

void QWorkspace::insertIcon( QWidget* w )
{
    if (d->icons.contains(w) )
	return;
    d->icons.append( w );
    if (w->parentWidget() != this )
	w->reparent( this, 0, QPoint(0,0), FALSE);
    layoutIcons();
    if (isVisible())
	w->show();

}

void QWorkspace::removeIcon( QWidget* w)
{
    if (!d->icons.contains( w ) )
	return;
    d->icons.remove( w );
    w->hide();
 }

void QWorkspace::resizeEvent( QResizeEvent * )
{
    if ( d->maxClient )
	d->maxClient->adjustToFullscreen();
    layoutIcons();
}

void QWorkspace::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    if ( d->becomeActive )
	activateClient( d->becomeActive );
    else if ( d->windows.count() > 0 && !d->active )
	activateClient( d->windows.first()->clientWidget() );
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

void QWorkspace::minimizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	c->hide();
	insertIcon( c->iconWidget() );
	if ( d->maxClient == c ) {
	    c->setGeometry( d->maxRestore );
	    d->maxClient = 0;
	    hideMaximizeControls();
	}

    }
}

void QWorkspace::normalizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	if ( c == d->maxClient ) {
	    c->setGeometry( d->maxRestore );
	    d->maxClient = 0;
	}
	else {
	    removeIcon( c->iconWidget() );
	    c->show();
	}
	hideMaximizeControls();
    }
}

void QWorkspace::maximizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );

    if ( c ) {
	if (d->icons.contains(c->iconWidget()) )
	    normalizeClient( w );
	QRect r( c->geometry() );
	c->adjustToFullscreen();
	c->show();
	c->raise();
	if ( d->maxClient && d->maxClient != c ) {
	    d->maxClient->setGeometry( d->maxRestore );
	}
	if ( d->maxClient != c ) {
	    d->maxClient = c;
	    d->maxRestore = r;
	}

	activateClient( w);
	showMaximizeControls();
    }
}

void QWorkspace::showClient( QWidget* w)
{
    if ( d->maxClient )
	maximizeClient( w );
    else
	normalizeClient( w );
}


QWorkspaceChild* QWorkspace::findChild( QWidget* w)
{
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	if (c->clientWidget() == w)
	    return c;
    }
    return 0;
}

/*!
  Returns a list of all clients.
 */
QWidgetList QWorkspace::clientList() const
{
    QWidgetList clients;
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	clients.append( c->clientWidget() );
    }
    return clients;
}

/*!\reimp
 */
bool QWorkspace::eventFilter( QObject *o, QEvent * e)
{
    if ( d->maxcontrols && e->type() == QEvent::Resize && o == topLevelWidget() )
	showMaximizeControls();

    return FALSE;
}

void QWorkspace::showMaximizeControls()
{
    if ( !d->wantsmaxcontrols )
	return;

    if ( !d->maxcontrols ) {
	d->maxcontrols = new QHBox( topLevelWidget() );
	if ( !win32 )
	    d->maxcontrols->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	QToolButton* restoreB = new QToolButton( d->maxcontrols, "restore" );
	restoreB->setFocusPolicy( NoFocus );
	restoreB->setIconSet( QPixmap( normalize_xpm ));
 	restoreB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( restoreB, SIGNAL( clicked() ), this, SLOT( normalizeActiveClient() ) );
	QToolButton* closeB = new QToolButton( d->maxcontrols, "close" );
	closeB->setFocusPolicy( NoFocus );
	closeB->setIconSet( QPixmap( close_xpm ) );
 	closeB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( closeB, SIGNAL( clicked() ), this, SLOT( closeActiveClient() ) );

	if ( !win32 ) {
	    restoreB->setAutoRaise( TRUE );
	    closeB->setAutoRaise( TRUE );
	}
	//d->maxcontrols->adjustSize();

	//### layout doesn't work
	d->maxcontrols->setFixedSize( 2* BUTTON_WIDTH+2*d->maxcontrols->frameWidth(),
				    BUTTON_HEIGHT+2*d->maxcontrols->frameWidth() );
    }

    d->maxcontrols->move ( topLevelWidget()->width() - d->maxcontrols->width() - 4, 4 );
    d->maxcontrols->show();
    d->maxcontrols->raise();
}

void QWorkspace::hideMaximizeControls()
{
    delete d->maxcontrols;
    d->maxcontrols = 0;
}

void QWorkspace::closeActiveClient()
{
    QWidget* w = activeClient();
    if ( w )
	w->close();
}

void QWorkspace::normalizeActiveClient()
{
    if  ( d->active )
	d->active->showNormal();
}



/*!
  Defines whether the workspace shows control buttons for windows in
  maximized state.

  If \a enabled is TRUE, the workspace will generate buttons to
  iconify or restore maximized windows and place them in the top-right
  corner of the toplevel widget.

  By default, maximize controls are enabled.

  \sa maximizeControls();

 */
void QWorkspace::setMaximizeControls( bool enabled )
{
    d->wantsmaxcontrols = enabled;
    if ( !enabled && d->maxcontrols ) {
	delete d->maxcontrols;
	d->maxcontrols = 0;
    }
}


/*!
  Returns whether the workspace shows control buttons for
  windows in maximized state or not.

  The default is TRUE.

  \sa setMaximizeControls()
 */
bool QWorkspace::maximizeControls() const
{
    return d->wantsmaxcontrols;
}

/*!
  \fn void QWorkspace::clientActivated( QWidget* w )

  This signal is emitted when the client widget \a w becomes active.

  \sa activeClient(), clientList()
*/



QWorkspaceChildTitleBar::QWorkspaceChildTitleBar (QWorkspace* w, QWidget* parent, const char* name, bool iconMode )
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder )
{
    workspace = w;
    buttonDown = FALSE;
    imode = iconMode;
    act = FALSE;

    titleL = new QLabel( this, "__workspace_child_title_bar" );
    titleL->setTextFormat( PlainText );

    closeB = new QToolButton( this, "close" );
    closeB->setFocusPolicy( NoFocus );
    closeB->setIconSet( QPixmap( close_xpm ) );
    closeB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( closeB, SIGNAL( clicked() ),
	     this, SIGNAL( doClose() ) ) ;
    maxB = new QToolButton( this, "maximize" );
    maxB->setFocusPolicy( NoFocus );
    maxB->setIconSet( QPixmap( maximize_xpm ));
    maxB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( maxB, SIGNAL( clicked() ),
	     this, SIGNAL( doMaximize() ) );
    iconB = new QToolButton( this, "iconify" );
    iconB->setFocusPolicy( NoFocus );
    iconB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);

    if ( !win32 ) {
	closeB->setAutoRaise( TRUE );
	maxB->setAutoRaise( TRUE );
	iconB->setAutoRaise( TRUE );
    }
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

    titleL->setMouseTracking( TRUE );
    titleL->installEventFilter( this );
    titleL->setAlignment( AlignVCenter | SingleLine );
    QFont fnt = font();
    fnt.setBold( TRUE );
    titleL->setFont( fnt );

    iconL = new QLabel( this, "left" );
    iconL->setFocusPolicy( NoFocus );
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


void QWorkspaceChildTitleBar::resizeEvent( QResizeEvent * )
{
    int bo = ( height()- BUTTON_HEIGHT) / 2;
    closeB->move( width() - BUTTON_WIDTH - bo, bo  );
    maxB->move( closeB->x() - BUTTON_WIDTH - bo, closeB->y() );
    iconB->move( maxB->x() - BUTTON_WIDTH, maxB->y() );
    iconL->setGeometry( 0, 0, BUTTON_WIDTH, height() );

    if ( win32 || (imode && !isActive()) )
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0 ),
				    rect().bottomRight() ) );
    else
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0),
				    QPoint( iconB->geometry().left() - 1, rect().bottom() ) ) );

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
	g.setColor( QColorGroup::Text,  colorGroup().color( QColorGroup::HighlightedText) );
	if ( win32 ) {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	}
    }
    else {
	if ( imode ){
	    iconB->hide();
	    closeB->hide();
	    maxB->hide();
	}
	QColorGroup g = colorGroup();
	if ( win32 ) {
	    g.setColor( QColorGroup::Background,  colorGroup().color( QColorGroup::Dark ) );
	    g.setColor( QColorGroup::Text,  colorGroup().color( QColorGroup::Background) );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	    titleL->setFrameStyle( QFrame::NoFrame );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	}
    }
    if ( imode )
	resizeEvent(0);
}

bool QWorkspaceChildTitleBar::isActive() const
{
    return act;
}


QSize QWorkspaceChildTitleBar::sizeHint() const
{
    return QSize( 196, QMAX( TITLEBAR_HEIGHT, fontMetrics().lineSpacing() ) );
}

class QWorkSpaceChildProtectedWidget : public QWidget
{
public:
    void reasonableFocus() { if ( !isFocusEnabled() )
	(void) focusNextPrevChild( TRUE );
    }
};


QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent,
				  const char *name )
    : QFrame( parent, name,
	      WStyle_Customize | WStyle_NoBorder  | WDestructiveClose )
{
    mode = 0;
    buttonDown = FALSE;
    setMouseTracking( TRUE );
    act = FALSE;
    iconw = 0;

    titlebar = new QWorkspaceChildTitleBar( parent, this );
    connect( titlebar, SIGNAL( doActivate() ),
	     this, SLOT( activate() ) );
    connect( titlebar, SIGNAL( doClose() ),
	     window, SLOT( close() ) );
    connect( titlebar, SIGNAL( doMinimize() ),
	     this, SLOT( showMinimized() ) );
    connect( titlebar, SIGNAL( doMaximize() ),
	     this, SLOT( showMaximized() ) );

    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    setMinimumSize( 128, 96 );

    clientw = window;
    if (!clientw)
	return;

    titlebar->setText( clientw->caption() );
    if( clientw->icon() )
	titlebar->setIcon( *clientw->icon() );

    int th = titlebar->sizeHint().height();

    bool hasBeenResize = clientw->testWState( WState_Resized );
    clientw->reparent( this, QPoint( contentsRect().x()+BORDER, th + BORDER + TITLEBAR_SEPARATION + contentsRect().y() ), TRUE  );

    if ( !hasBeenResize ) {
	QSize cs = clientw->sizeHint();
	QSize s( cs.width() + 2*frameWidth() + 2*BORDER,
		 cs.height() + 3*frameWidth() + th +TITLEBAR_SEPARATION+2*BORDER );
	resize( s );
    } else {
	resize( clientw->width() + 2*frameWidth() + 2*BORDER, clientw->height() + 2*frameWidth() + th +2*BORDER);
    }

    clientw->installEventFilter( this );
}

QWorkspaceChild::~QWorkspaceChild()
{
    if (iconw) {
	delete iconw->parentWidget();
	iconw = 0;
    }
}


void QWorkspaceChild::resizeEvent( QResizeEvent * )
{
    QRect r = contentsRect();
    int th = titlebar->sizeHint().height();
    titlebar->setGeometry( r.x() + BORDER, r.y() + BORDER, r.width() - 2*BORDER, th+1);

    if (!clientw)
	return;

    QRect cr( r.x() + BORDER, r.y() + BORDER + TITLEBAR_SEPARATION + th,
			r.width() - 2*BORDER,
			  r.height() - 2*BORDER - TITLEBAR_SEPARATION - th);
    clientSize = cr.size();
    clientw->setGeometry( cr );
}

void QWorkspaceChild::activate()
{
    ((QWorkspace*)parentWidget())->activateClient( clientWidget() );
}


bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{

    if ( !isActive() ) {
	if ( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::FocusIn ) {
	    activate();
	}
    }

    if (o != clientw)
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Show:
	if ( isVisible() )
	    break;
	if (( (QShowEvent*)e)->spontaneous() )
	    break;
	((QWorkspace*)parentWidget())->showClient( clientWidget() );
	break;
    case QEvent::ShowMaximized:
	((QWorkspace*)parentWidget())->maximizeClient( clientWidget() );
	break;
    case QEvent::ShowMinimized:
	((QWorkspace*)parentWidget())->minimizeClient( clientWidget() );
	break;
    case QEvent::ShowNormal:
	((QWorkspace*)parentWidget())->normalizeClient( clientWidget() );
	break;
    case QEvent::Hide:
	if ( !clientw->isVisibleTo( this ) ) {
	    if (iconw) {
		delete iconw->parentWidget();
		iconw = 0;
	    }
	    hide();
	}
	break;
#if QT_VERSION >= 210
    case QEvent::CaptionChange:
	titlebar->setText( clientw->caption() );
	break;
    case QEvent::IconChange:
	if ( clientw->icon() )
	    titlebar->setIcon( *clientw->icon() );
	else {
	    QPixmap pm;
	    titlebar->setIcon( pm );
	}
	break;
#endif
    case QEvent::LayoutHint:
	//layout()->activate();
	break;
    case QEvent::Resize:
	{
	    QResizeEvent* re = (QResizeEvent*)e;
	    if ( re->size() != clientSize ){
		int th = titlebar->sizeHint().height();
		QSize s( re->size().width() + 2*frameWidth() + 2*BORDER,
			 re->size().height() + 3*frameWidth() + th +TITLEBAR_SEPARATION+2*BORDER );
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
    if ( e->type() == QEvent::ChildRemoved && e->child() == clientw ) {
	clientw = 0;
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

    if ( testWState(WState_ConfigPending) )
	return;

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
    QPoint mp( QMIN( pp.x(), geometry().right() - minimumWidth() +1 ),
	       QMIN( pp.y(), geometry().bottom() - minimumHeight() + 1 ) );
    mp = QPoint( QMAX( mp.x(), geometry().right() - maximumWidth() +1 ),
		 QMAX( mp.y(), geometry().bottom() -maximumHeight() +1) );


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

#ifdef _WS_WIN_
    MSG msg;
    while( PeekMessage( &msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
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
	QObject *o;
	for ( o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	bool hasFocus = FALSE;
	for ( o = ol->first(); o; o = ol->next() ) {
	    hasFocus |= ((QWidget*)o)->hasFocus();
	}
	if ( !hasFocus ) {
	    clientw->setFocus(); // insufficient, need toplevel semantics ########
	    ( (QWorkSpaceChildProtectedWidget*)clientw)->reasonableFocus();
	}
	delete ol;

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
	that->iconw = new QWorkspaceChildTitleBar( (QWorkspace*)parentWidget(), vbox, 0, TRUE );
	iconw->setActive( isActive() );
	connect( iconw, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( iconw, SIGNAL( doClose() ),
		 clientWidget(), SLOT( close() ) );
	connect( iconw, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
	connect( iconw, SIGNAL( doMaximize() ),
		 this, SLOT( showMaximized() ) );
    }
    iconw->setText( clientWidget()->caption() );
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    ((QWorkspace*)parentWidget())->minimizeClient( clientWidget() );
}

void QWorkspaceChild::showMaximized()
{
    ((QWorkspace*)parentWidget())->maximizeClient( clientWidget() );
}

void QWorkspaceChild::showNormal()
{
    ((QWorkspace*)parentWidget())->normalizeClient( clientWidget() );
}



void QWorkspaceChild::adjustToFullscreen()
{
    setGeometry( -clientw->x(), -clientw->y(),
		 parentWidget()->width() + width() - clientw->width(),
		 parentWidget()->height() + height() - clientw->height() );
}


#include "qworkspace.moc"
