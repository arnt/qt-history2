/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.cpp#2 $
**
** Implementation of tab dialog
**
*****************************************************************************/

#include "qtabdlg.h"
#include "qrect.h"
#include "qfontmet.h"
#include "qpushbt.h"
#include "qpainter.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qtabdialog.cpp#2 $");


// a small private class to show the tabs on top

class QTab : public QWidget
{
    // note no Q_OBJECT -- no signals, no slots, so no necessity
public:
    QTab( QWidget *, const char *, QTabDialog *, const char * );
    ~QTab();

    QTabDialog * daddy;
    QTab * next;
    QWidget * w;
    const char * name;

protected:
    void paintEvent( QPaintEvent * );
};


QTab::QTab( QWidget * child, const char * tabString,
	    QTabDialog * parent, const char * objectName )
    : QWidget( parent, objectName )
{
    w = child;
    name = qstrdup( tabString );
    daddy = parent;
}


QTab::~QTab()
{
    delete w;
    delete[] name;
}


void QTab::paintEvent( QPaintEvent * )
{
    QPainter p;

    p.begin( this );
    p.setPen( white );
    if ( daddy->currentTab == this ) {
	p.drawLine( 0, height() - 1, 0, 2 );
	p.drawPoint( 1, 1 );
	p.drawLine( 2, 0, width() - 5, 0 );
	p.setPen( colorGroup().dark() );
	p.drawPoint( width() - 4, 1 );
	p.drawLine( width() - 3, 2,
		    width() - 3, height() - 2);
	p.setPen( black );
	p.drawPoint( width() - 3, 1 );
	p.drawLine( width() - 2, 2,
		     width() - 2, height() - 2);
	p.setPen( colorGroup().background() );
	p.drawLine( 1, height() - 1, width() - 4, height() - 1 );
    } else {
	p.drawLine( 1, height() - 2, 1, 4 );
	p.drawPoint( 2, 3 );
	p.drawLine( 3, 2, width() - 4, 2 );
	p.setPen( colorGroup().dark() );
	p.drawPoint( width() - 3, 3 );
	p.drawLine( width() - 2, 4,
		     width() - 2, height() - 2);
	p.setPen( black );
	p.drawPoint( width() - 2, 3 );
	p.drawLine( width() - 1, 4,
		     width() - 1, height() - 2);
	p.setPen( colorGroup().background() );
    }
    p.drawText( 2, 0, width() - 6, height(), AlignCenter, name );
    p.end();
}
	


/*! \class QTabDialog qtabdlg.h

  \brief The QTabDialog class provides a tabbed dialog.

  A tabbed dialog is one in which several "pages" are available, and the
  user selects which page to see and use by clicking on its tab.  No
  keyboard shortcuts are available.

  Tabbed dialogs provide an easy way for to cram four times as much
  information into a window as a normal dialog does.  No more than that,
  however, because once there is more than one row of tabs, it becomes much
  harder to navigate for most people.

  QTabDialog does not provide more than one row of tabs, and does not provide
  tabs along the sides or bottom of the dialog.


  QTabDialog provides an OK button and optionally Cancel and Defaults
  buttons.  In order to 
  
  */



/*! Constructs a QTabDialog without cancel or default buttons. */

QTabDialog::QTabDialog( QWidget *parent, const char *name, WFlags f )
    : QDialog( parent, name, f )
{
    tabs = currentTab = 0;
    cb = db = 0;

    ok = new QPushButton( this, "ok" );
    CHECK_PTR( ok );
    ok->setText( "OK" );
    ok->setDefault( TRUE );
    connect( ok, SIGNAL(clicked()),
	     this, SIGNAL(okButtonPressed()) );
    connect( ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
}


/*! Destroys the tab view and all its children. */

QTabDialog::~QTabDialog()
{
    delete ok;
    delete db;
    delete cb;
}


/*!  Shows the tab view and its children.  Reimplemented in order to delay
  show()'ing of every page except the initially visible one. \sa hide() */

void QTabDialog::show()
{
    setSizes();
    showTab();

    // now hide the others so QWidget::show won't pop them up
    for ( QTab * tab = tabs; tab; tab = tab->next )
	if ( !tab->w->testWFlags(WState_Visible) )
	    tab->w->hide();

    QDialog::show();
}


/*!  Ensure that there is a current tab, and that its page is visible on
  screen. */

void QTabDialog::showTab()
{
    if ( tabs && !currentTab )
	currentTab = tabs;

    if ( !currentTab )
	return;

    if ( !(currentTab->w->isVisible()) ) {
	currentTab->w->setGeometry( childRect );
	currentTab->w->show();
    }
    currentTab->w->raise();
}

    
/*! Add another tab and page to the tab view.

  It's a fairly bad idea to do this after show(). */

void QTabDialog::addTab( QWidget * child, const char * name )
{
    QTab ** t = &tabs;
    while ( t && *t )
	t = &((**t).next);

    *t = new QTab ( child, name, this, name );
    CHECK_PTR( *t );
    (**t).installEventFilter( this );

    update();

    if ( isVisible() && !currentTab )
	showTab();
}


/*! If \e enable is TRUE, a Defaults button is added to the dialog.  If \e
  enable is FALSE, the button is removed.

  The Defaults button should set the dialog to back to the application
  defaults.

  When Defaults is clicked, the defaultButtonPressed() signal is emitted.

  \sa setCancelButton() defaultButtonPressed() */

void QTabDialog::setDefaultButton( bool enable )
{
    if ( !!db == enable )
	return;

    if ( enable ) {
	db = new QPushButton( this, "back to default" );
	if ( isVisible() ) {
	    setSizes();
	    db->show();
	}
	connect( db, SIGNAL(clicked()),
		 this, SIGNAL(defaultButtonPressed()) );
    } else {
	delete db;
	db = 0;
    }
}


/*! If \e enable is TRUE, a Cancel button is added to the dialog.  If \e
  enable is FALSE, the button is removed.

  The cancel button should always return the application to the state it
  was in before the tab view popped up.

  When Cancel is clicked, the cancelButtonPressed() signal is emitted.

  \sa setDefaultButton() cancelButtonPressed() */

void QTabDialog::setCancelButton( bool enable )
{
    if ( !!cb == enable )
	return;

    if ( enable ) {
	cb = new QPushButton( this, "cancel dialog" );
	if ( isVisible() ) {
	    setSizes();
	    cb->show();
	}
	connect( db, SIGNAL(clicked()),
		 this, SIGNAL(cancelButtonPressed()) );
	connect( cb, SIGNAL(clicked()),
		 this, SLOT(reject()) );
    } else {
	delete cb;
	cb = 0;
    }
}


/*! Set the appropriate size for each of the fixed children, and if
  the widget is visible, their positions too.

  Finally set the minimum and maximum sizes for the dialog.

  This function does not resize or move the panes - only resizeEvent()
  does that.

  \sa setCancelButton() setDefaultButton() */

void QTabDialog::setSizes()
{
    int w,h;
    ok->adjustSize();
    w = ok->width();
    h = ok->height();

    if ( db ) {
	db->adjustSize();
	if ( db->width() > w )
	    w = db->width();
	if ( db->height() > h )
	    h = db->height();
    }

    if ( cb ) {
	cb->adjustSize();
	if ( cb->width() > w )
	    w = cb->width();
	if ( cb->height() > h )
	    h = cb->height();
    }

    ok->resize( w, h );
    if ( db )
	db->resize( w, h );
    if ( cb )
	cb->resize( w, h );

    QTab * t = tabs;
    QFontMetrics fm( fontMetrics() );
    QSize min(0,0), max(QCOORD_MAX,QCOORD_MAX);
    int th = fm.height() + 10;
    int tw = 5;
    while ( t ) {
	t->resize( fm.width( t->name )+10, th );
	tw += t->width();
	if ( t->w->minimumSize().height() < max.height() )
	    max.setHeight( t->w->minimumSize().height() );
	if ( t->w->minimumSize().width() < max.width() )
	    max.setWidth( t->w->minimumSize().width() );
	if ( t->w->minimumSize().height() > min.height() )
	    min.setHeight( t->w->minimumSize().height() );
	if ( t->w->minimumSize().height() > min.height() )
	    min.setHeight( t->w->minimumSize().height() );
	t = t->next;
    }

    if ( min.width() < tw )
	min.setWidth( tw );
    if ( max.width() < min.width() )
	max.setWidth( min.width() );
    if ( max.height() < min.height() )
	max.setHeight( min.height() );

    setMinimumSize( min.width() + 13, min.height() + th + h + 18 );
    setMinimumSize( max.width() + 13, max.height() + th + h + 18 );

    if ( isVisible() ) {
	// need to set the appropriate positions too
	int x = width();

	if ( cb ) {
	    cb->move( x - 5 - cb->width(), height() - 5 - cb->height() );
	    x = cb->geometry().x();
	}

	if ( db ) {
	    db->move( x - 5 - db->width(), height() - 5 - db->height() );
	    x = db->geometry().x();
	}

	if ( ok ) {
	    ok->move( x - 5 - ok->width(), height() - 5 - ok->height() );
	}

	t = tabs;
	x = 0;
	while ( t ) {
	    t->move( x, 0 );
	    x += t->width();
	    t = t->next;
	}
    }
}

    


/*! Handles resize events for the tab dialog.

  All of the pages are resized, even the invisible ones.

 */

void QTabDialog::resizeEvent( QResizeEvent * )
{
    if ( tabs ) {
	int x;
	x = width();

	if ( cb ) {
	    cb->move( x - 5 - cb->width(), height() - 5 - cb->height() );
	    x = cb->geometry().x();
	}

	if ( db ) {
	    db->move( x - 5 - db->width(), height() - 5 - db->height() );
	    x = db->geometry().x();
	}

	if ( ok ) {
	    ok->move( x - 5 - ok->width(), height() - 5 - ok->height() );
	}

	childRect.setRect( 6, tabs->height() + 5, width() - 12,
			   height() - ok->height() - tabs->height() - 18 );
	x = 5;
	for ( QTab * tab = tabs; tab; tab = tab->next ) {
	    if ( tab->w && tab->w->rect() != childRect )
		tab->w->setGeometry( childRect );
	    tab->move( x, 5 );
	    x += tab->width();
	}
    }
}


/*! Handles paint events for the tabbed dialog */

void QTabDialog::paintEvent( QPaintEvent * )
{
    if ( !tabs )
	return;

    QPainter p;
    p.begin( this );

    QCOORD t = childRect.top() - 1;
    QCOORD b = childRect.bottom() + 2;
    QCOORD r = childRect.right() + 2;
    QCOORD l = childRect.left() - 1;

    p.setPen( white );
    p.drawLine( l, t, r - 1, t );
    p.drawLine( l, t + 1, l, b );
    p.setPen( black );
    p.drawLine( r, b, l,b );
    p.drawLine( r, b-1, r, t );
    p.setPen( colorGroup().dark() );
    p.drawLine( l+1, b-1, r-1, b-1 );
    p.drawLine( r-1, b-2, r-1, t+1 );

    p.end();
}

/*! Intercepts and processes mouse events for the tabs.

  \internal

  Better an event filter than a friend! */

bool QTabDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( e->type() != Event_MouseButtonRelease )
	return FALSE;

    // simply assume o is a QTab
    QTab * t = (QTab *) o;
    QMouseEvent * me = (QMouseEvent *) e;

    if ( t->rect().contains( me->pos() ) &&
	 t != currentTab ) {
	// a tab was clicked, and not the current one
	QTab * tab = tabs;
	while ( tab != t && tab )
	    tab = tab->next;

	if ( tab ) {
	    QTab * prevCurrent = currentTab;
	    currentTab = tab;
	    prevCurrent->repaint();
	    currentTab->repaint();
	    showTab();
	    return TRUE;
	}
    }
    return FALSE;
}
