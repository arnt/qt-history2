/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdialog.cpp#17 $
**
** Implementation of tab dialog
**
*****************************************************************************/

#include "qtabdlg.h"
#include "qrect.h"
#include "qfontmet.h"
#include "qpushbt.h"
#include "qpainter.h"
#include "qstring.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qtabdialog.cpp#17 $");

// a small private class to show the tabs on top

class QTab : public QWidget
{
    // note no Q_OBJECT - no signals, no slots, so no necessity
public:
    QTab( QWidget *, const char *, QTabDialog *, const char * );
    ~QTab();

    QTab * next;
    QWidget * w;
    QString name;
};


QTab::QTab( QWidget * child, const char * tabString,
	    QTabDialog * parent, const char * objectName )
    : QWidget( parent, objectName )
{
    w = child;
    next = 0;
    name = tabString;
    setFont( QFont("helvetica" ) );
}


QTab::~QTab()
{
    delete w;
}


/*!
  \class QTabDialog qtabdlg.h

  \brief The QTabDialog class provides a tabbed dialog.

  A tabbed dialog is one in which several "pages" are available, and
  the user selects which page to see and use by clicking on its tab.
  No keyboard shortcuts are available.	Tabbed dialogs provide an easy
  way to cram more information into a window than a normal dialog
  does.

  While tab dialogs can be a very good way to split up a complex
  dialog, it's also very easy to make a hash of it.  Here is some
  advice:

  <ol><li> Make sure that each page forms a logical whole which is
  adequately described by the label on the tab.

  If two related functions are on different pages, users will often
  not find one of the functions, or will spend far too long searching
  for it.

  <li> Do not join several independent dialogs into one tab dialog.
  Several aspects of one complex dialog is acceptable (such as the
  various aspects of "preferences") but a tab dialog is no substitute
  for a pop-up menu leading to several smaller dialogs.

  The OK button (and the other buttons) apply to the \e entire dialog.
  If the tab dialog is really several independent smaller dialogs,
  users often press Cancel to cancel just the changes he/she has made
  on the current page: Many users will treat that page as independent
  of the other pages.

  <li> Do not use tab dialogs for frequent operations.	The tab dialog
  is probably the most complex widget in common use at the moment, and
  subjecting the user to this complexity during his/her normal use of
  your application is most often a bad idea.

  The tab dialog is good for complex operations which have to be
  performed seldom, like Preferences dialogs.  Not for common
  operations, like setting left/right alignment in a word processor.
  (Often, these common operations are actually independent dialogs and
  should be treated as such.)

  The tab dialog is not a navigational aid, it is an organizational
  aid.	It is a good way to organize aspects of a complex operation
  (such as setting up caching and proxies in a WWW browser), but a bad
  way to navigate towards a simple operation (such as emptying the
  cache in a WWW browser - emptying the cache is \e not part of
  setting up the cache, it is a separate and independent operation).

  <li> The changes should take effect when the user presses Apply or
  OK.  Not before.

  Providing Apply, Cancel or OK buttons on the individual pages is
  likely to weaken the users' mental model of how tab dialogs work.
  If you think a page needs its own buttons, consider making it a
  separate dialog.

  <li> There should be no implicit ordering of the pages.  If there
  is, it is probably better to use a \link QWizardDialog wizard dialog.
  \endlink

  If some of the pages seem to be ordered and others not, perhaps they
  ought not to be joined in a tab dialog.

  </ol>

  QTabDialog does not provide more than one row of tabs, and does not
  provide tabs along the sides or bottom of the pages.

  QTabDialog provides an OK button and optionally Apply, Cancel and
  Defaults buttons.

*/

// add comments about delete, ok and apply



/*!
  Constructs a QTabDialog with only an OK button.
*/

QTabDialog::QTabDialog( QWidget *parent, const char *name, WFlags f )
    : QDialog( parent, name, f )
{
    tabs = currentTab = 0;
    ab = cb = db = 0;

    ok = new QPushButton( this, "ok" );
    CHECK_PTR( ok );
    ok->setText( "OK" );
    ok->setDefault( TRUE );
    connect( ok, SIGNAL(clicked()),
	     this, SIGNAL(applyButtonPressed()) );
    connect( ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
}


/*!
  Destroys the tab view and all its children.
*/

QTabDialog::~QTabDialog()
{
    // no cleanup necessary
}


/*!
  Sets the font for the tabs to \e font.

  The weight is forced to QFont::Bold for the active tab and
  QFont::Light for the others.	(QTabDialog::font() returns the
  latter.)

  If the widget is visible, the display is updated with the new font
  immediately.	There may be some geometry changes, depending on the
  size of the old and new fonts.
*/

void QTabDialog::setFont( const QFont & font )
{
    QFont f( font );
    f.setWeight( QFont::Light );
    QDialog::setFont( f );
    for ( QTab * t = tabs; t; t=t->next )
	t->setFont( f );
    if ( isVisible() ) {
	setSizes();
	showTab();
    }
}


/*!
  \fn void QTabDialog::applyButtonPressed();

  This signal is emitted when the Apply or OK buttons are clicked.

  It should be connected to a slot (or several slots) which change the
  application's state according to the state of the dialog.

  \sa cancelButtonPressed() defaultButtonPressed() setApplyButton()
*/


/*!
  \fn bool QTabDialog::hasApplyButton() const

  Returns TRUE if the tab dialog has an Apply button, FALSE if not.

  \sa setApplyButton() applyButtonPressed() hasCancelButton()
  hasDefaultButton()
*/


/*!
  \fn void QTabDialog::cancelButtonPressed();

  This signal is emitted when the Cancel button is clicked.  It should
  not change the application's state in any way, so generally you
  should not need to connect it to any slot.

  \sa applyButtonPressed() defaultButtonPressed() setCancelButton()
*/


/*!
  \fn bool QTabDialog::hasCancelButton() const

  Returns TRUE if the tab dialog has a Cancel button, FALSE if not.

  \sa setCancelButton() cancelButtonPressed() hasDefaultButton()
  hasApplyButton()
*/


/*!
  \fn void QTabDialog::defaultButtonPressed();

  This signal is emitted when the Defaults button is pressed.  It
  should reset the dialog (but not the application) to the "factory
  defaults."

  The application's state should not be changed until the user clicks
  Apply or OK.

  \sa applyButtonPressed() cancelButtonPressed() setDefaultButton()
*/


/*!
  \fn bool QTabDialog::hasDefaultButton() const

  Returns TRUE if the tab dialog has a Defaults button, FALSE if not.

  \sa setDefaultsButton() defaultButtonPressed() hasApplyButton()
  hasCancelButton()
*/


/*!
  \fn void QTabDialog::aboutToShow()

  This signal is emitted by show() when it's time to set the state of
  the dialog's contents.  The dialog should reflect the current state
  of the application when if appears; if there is any chance that the
  state of the application can change between the time you call
  QTabDialog::QTabDialog() and QTabDialog::show(), you should set the
  dialog's state in a slot and connect this signal to it.

  \sa applyButtonPressed() show() cancelButtonPressed()
*/


/*!
  Shows the tab view and its children.	Reimplemented in order to
  delay show()'ing of every page except the initially visible one, and
  in order to emit the aboutToShow() signal.

  \sa hide() aboutToShow()
*/

void QTabDialog::show()
{
    if ( isVisible() )
	return;

    emit aboutToShow();
    setSizes();
    showTab();

    // now hide the others so QWidget::show won't pop them up
    for ( QTab * tab = tabs; tab; tab = tab->next )
	if ( !tab->w->testWFlags(WState_Visible) )
	    tab->w->hide();

    QDialog::show();
}


/*!
  Ensure that there is a current tab, and that its page is visible on
  screen.
*/

void QTabDialog::showTab()
{
    if ( tabs && !currentTab )
	currentTab = tabs;

    if ( !currentTab )
	return;

    if ( !(currentTab->w->isVisible()) ) {
	currentTab->w->setGeometry( childRect() );
	currentTab->w->show();
    }
    currentTab->w->raise();
}


/*!
  Add another tab and page to the tab view.

  The tab will be labelled \e name and \e child constitutes the new
  page.

  If you do this after show(), you'll confuse the user terribly.
*/

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


/*!
  Add an Apply button to the dialog.  The button's text is set to \e
  text (and defaults to "Apply").

  The Apply button should apply the current settings in the dialog box
  to the application, while keeping the dialog visible.

  When Apply is clicked, the applyButtonPressed() signal is emitted.

  \sa setCancelButton() setDefaultButton() applyButtonPressed()
*/

void QTabDialog::setApplyButton( const char * text )
{
    if ( !ab ) {
	ab = new QPushButton( this, "apply settings" );
	connect( ab, SIGNAL(clicked()),
		 this, SIGNAL(applyButtonPressed()) );
    }
    ab->setText( text );
    if ( isVisible() ) {
	setSizes();
	ab->show();
    }
}


/*!
  Add a Defaults button to the dialog.	The button's text is set to \e
  text (and defaults to "Defaults").

  The Defaults button should set the dialog (but not the application)
  back to the application defaults.

  When Defaults is clicked, the defaultButtonPressed() signal is emitted.

  \sa setApplyButton() setCancelButton() defaultButtonPressed()
*/

void QTabDialog::setDefaultButton( const char * text )
{
    if ( !db ) {
	db = new QPushButton( this, "back to default" );
	connect( db, SIGNAL(clicked()),
		 this, SIGNAL(defaultButtonPressed()) );
    }
    db->setText( text );
    if ( isVisible() ) {
	setSizes();
	db->show();
    }
}


/*!
  Add a Cancel button to the dialog.  The button's text is set to \e
  text (and defaults to "Cancel").

  The cancel button should always return the application to the state
  it was in before the tab view popped up, or if the user has clicked
  Apply, back the the state immediately after the last Apply.

  When Cancel is clicked, the cancelButtonPressed() signal is emitted.
  The dialog is closed at the same time.

  \sa setApplyButton setDefaultButton() cancelButtonPressed()
*/

void QTabDialog::setCancelButton( const char * text )
{
    if ( !cb ) {
	cb = new QPushButton( this, "cancel dialog" );
	connect( cb, SIGNAL(clicked()),
		 this, SIGNAL(cancelButtonPressed()) );
	connect( cb, SIGNAL(clicked()),
		 this, SLOT(reject()) );
    }
    cb->setText( text );
    if ( isVisible() ) {
	setSizes();
	cb->show();
    }
}


/*!
  Set the appropriate size for each of the fixed children, and if
  the widget is visible, their positions too.

  Finally set the minimum and maximum sizes for the dialog.

  This function does not resize or move the pages - only resizeEvent()
  does that.

  \sa setApplyButton() setCancelButton() setDefaultButton()
*/

void QTabDialog::setSizes()
{

    // compute largest button size
    int bw;
    QSize s( ok->sizeHint() );
    bw = s.width();
    bh = s.height(); // private member, used for GM

    if ( ab ) {
	s = ab->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( db ) {
	s = db->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    if ( cb ) {
	s = cb->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > bh )
	    bh = s.height();
    }

    // and set all the buttons to that size
    ok->resize( bw, bh );
    if ( ab )
	ab->resize( bw, bh );
    if ( db )
	db->resize( bw, bh );
    if ( cb )
	cb->resize( bw, bh );

    // look at the pages and find the largest minimum and smalles
    // maximum size
    QTab * t = tabs;
    QFontMetrics fm( fontMetrics() );
    QSize min(0,0);
    QSize max(QCOORD_MAX,QCOORD_MAX);
    int th = fm.height() + 10;
    int tw = 0;
    while ( t ) {
	// tabs - move and update remembe the width
	t->resize( fm.width( t->name )+20, th );
	tw += t->width() + 1;
	// maximum sizes
	if ( t->w->maximumSize().height() < max.height() )
	    max.setHeight( t->w->maximumSize().height() );
	if ( t->w->maximumSize().width() < max.width() )
	    max.setWidth( t->w->maximumSize().width() );
	// minimum sizes
	if ( t->w->minimumSize().height() > min.height() )
	    min.setHeight( t->w->minimumSize().height() );
	if ( t->w->minimumSize().width() > min.width() )
	    min.setWidth( t->w->minimumSize().width() );
	t = t->next;
    }

    // widen the minimum to make space for all of the tabs
    if ( min.width() < tw )
	min.setWidth( tw );

    // max must be >= min
    if ( max.width() < min.width() )
	max.setWidth( min.width() );
    if ( max.height() < min.height() )
	max.setHeight( min.height() );

    // bang in the pages' minimum and maximum sizes, to avoid ugly
    // size mismatches
    for( t = tabs; t; t=t->next ) {
	t->w->setMinimumSize( min );
	t->w->setMaximumSize( max );
    }

    // allow for own borders, buttons and tabs, and set own sizes
    min.setWidth( QMIN( min.width() + 13, 32767 ) );
    min.setHeight( QMIN( min.height() + bh + th + 18, 32767 ) );
    max.setWidth( QMIN( max.width() + 13, 32767 ) );
    max.setHeight( QMIN( max.height() + bh + th + 18, 32767 ) );
    setMinimumSize( min );
    setMaximumSize( max );

    if ( isVisible() ) {
	// fake a resize event to trigger child widget moves
	QResizeEvent r( size(), size() );
	resizeEvent( &r );
    }
}




/*!
  Handles resize events for the tab dialog.

  All of the pages are resized, even the invisible ones.
*/

void QTabDialog::resizeEvent( QResizeEvent * )
{
    if ( tabs ) {
	int x;
	x = width();

	if ( cb ) {
	    cb->move( x - 5 - cb->width(), height() - 5 - bh );
	    x = cb->geometry().x();
	}

	if ( ab ) {
	    ab->move( x - 5 - cb->width(), height() - 5 - bh );
	    x = ab->geometry().x();
	}

	if ( db ) {
	    db->move( x - 5 - db->width(), height() - 5 - bh );
	    x = db->geometry().x();
	}

	if ( ok ) {
	    ok->move( x - 5 - ok->width(), height() - 5 - bh );
	}

	x = 5;
	for ( QTab * tab = tabs; tab; tab = tab->next ) {
	    if ( tab->w && tab->w->isVisible() &&
		 tab->w->rect() != childRect() )
		tab->w->setGeometry( childRect() );
	    tab->move( x, 5 );
	    x += tab->width()+1;
	}
    }
}


/*!
  Handles paint events for the tabbed dialog
*/

void QTabDialog::paintEvent( QPaintEvent * )
{
    if ( !tabs )
	return;

    QPainter p;
    p.begin( this );

    QCOORD t = childRect().top() - 1;
    QCOORD b = childRect().bottom() + 2;
    QCOORD r = childRect().right() + 2;
    QCOORD l = childRect().left() - 1;

    p.setPen( white );
    // note - this line overlaps the bottom line drawn by QTab
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

/*!
  Intercepts and processes mouse events for the tabs.

  \internal

  Better an event filter than a friend!
*/

bool QTabDialog::eventFilter( QObject * o, QEvent * e )
{
    // simply assume o is a QTab
    QTab * t = (QTab *) o;
    if ( e->type() == Event_MouseButtonRelease ) {
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
    } else if ( e->type() == Event_Paint ) {
	QPainter p;

	p.begin( t );
	p.setPen( white );
	if ( currentTab == t ) {
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
	    p.setPen( white );
	    p.drawLine( width()-3, height() - 1, width(), height() - 1 );
	    QFont bold( font() );
	    bold.setBold( TRUE );
	    p.setFont( bold );
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
	    p.setPen( white );
	    p.drawLine( 0, height() - 1, width(), height() - 1 );
	}
	p.setPen( black );
	p.drawText( 2, 0, t->width() - 6, t->height(), AlignCenter, t->name );
	p.end();
    }

    return FALSE;
}

QRect QTabDialog::childRect()
{
    return QRect( 6, tabs->height() + 5, width() - 12,
		  height() - bh - tabs->height() - 18 );
}
