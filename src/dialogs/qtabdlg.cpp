/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtabdlg.cpp#49 $
**
** Implementation of QTabDialog class
**
** Created : 960825
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtabdlg.h"
#include "qtabbar.h"
#include "qpushbt.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qtabdlg.cpp#49 $");


/*!
  \class QTabDialog qtabdlg.h

  \brief The QTabDialog class provides a tabbed dialog.

  A tabbed dialog is one in which several "pages" are available, and
  the user selects which page to see and use by clicking on its tab,
  or by pressing the indicated Alt-(letter) key combination.

  QTabDialog does not provide more than one row of tabs, and does not
  provide tabs along the sides or bottom of the pages.  It also does
  not offer any way to find out which page is currently visible or to
  set the visible page.

  QTabDialog provides an OK button and optionally Apply, Cancel and
  Defaults buttons.

  The normal way to use QTabDialog is to do the following in the
  constructor: <ol> <li> Create a QTabDialog, <li> create a QWidget
  for each of the pages in the tab dialog, insert children into it,
  set up geometry management for it, and use addTab() to set up a tab
  and keyboard accelerator for it, <li> set up the buttons for the tab
  dialog (Apply, Cancel and so on), and finally <li> connect to the
  signals and slots. </ol>

  The pref.cpp example does all this.

  If you don't call addTab(), the page you have created will not be
  visible.  Please don't confuse the object name you supply to the
  QWidget constructor and the tab label you supply to addTab():
  addTab() takes a name which indicates an accelerator and is
  meaningful and descriptive to the user, while the widget name is
  used primarily for debugging.

  Almost all applications have to connect the applyButtonPressed()
  signal to something.  applyButtonPressed() is emitted when either OK
  or Apply is clicked, and your slot must copy the dialog's state into
  the application.

  There are also several other signals which may be useful. <ul> <li>
  cancelButtonPressed() is emitted when the user clicks Cancel.  <li>
  defaultButtonPressed() is emitted when the user clicks Defaults; the
  slot it is connected to should reset the state of the dialog to the
  application defaults.  <li> aboutToShow() is emitted at the start of
  show(); if there is any chance that the state of the application may
  change between the creation of the tab dialog and the time it show()
  is called, you must connect this signal to a slot which resets the
  state of the dialog. <li> selected() is emitted when the user
  selects some page. </ul>

  Each tab is either enabled or disabled at any given time.  If a tab
  is enabled, the tab text is drawn in black and the user can select
  that tab.  If it is disabled, the tab is drawn in a different way
  and the user can not select that tab.  Note that even though a tab
  is disabled, the page can still be visible, for example if all of
  the tabs happen to be disabled.

  While tab dialogs can be a very good way to split up a complex
  dialog, it's also very easy to make a royal mess out of a tab
  dialog.  Here is some advice (greatly inspired by a USENET posting
  by Jared M. Spool of <a href="http://www.uie.com">User Interface
  Engineering</a>):

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

  <li> Do not use tab dialogs for frequent operations.  The tab dialog
  is probably the most complex widget in common use at the moment, and
  subjecting the user to this complexity during his/her normal use of
  your application is most often a bad idea.

  The tab dialog is good for complex operations which have to be
  performed seldom, like Preferences dialogs.  Not for common
  operations, like setting left/right alignment in a word processor.
  (Often, these common operations are actually independent dialogs and
  should be treated as such.)

  The tab dialog is not a navigational aid, it is an organizational
  aid.  It is a good way to organize aspects of a complex operation
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
  is, it is probably better to use a wizard dialog.

  If some of the pages seem to be ordered and others not, perhaps they
  ought not to be joined in a tab dialog.

  </ol>

  <img src=qtabdlg-m.gif> <img src=qtabdlg-w.gif>
*/

/*! \fn void QTabDialog::selected( const char * tabLabel );

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/
  

// add comments about delete, ok and apply

struct QTabPrivate
{
    QTabBar * tabs;

    QArrayT<QWidget *> children;

    QPushButton * ok;
    QPushButton * cb;
    QPushButton * db;
    QPushButton * ab;
    int bh;

    QCOORD l_marg, r_marg, t_marg, b_marg;
};

/*!
  Constructs a QTabDialog with only an Ok button.
*/

QTabDialog::QTabDialog( QWidget *parent, const char *name, bool modal,
			WFlags f )
    : QDialog( parent, name, modal, f )
{
    d = new QTabPrivate;
    CHECK_PTR( d );

    d->l_marg = 7;
    d->t_marg = 6;
    d->r_marg = d->l_marg;
    d->b_marg = 8;

    d->tabs = 0;
    setTabBar( new QTabBar( this, "tab control" ) );

    d->ab = d->cb = d->db = 0;

    d->ok = new QPushButton( this, "ok" );
    CHECK_PTR( d->ok );
    d->ok->setText( "OK" );
    d->ok->setDefault( TRUE );
    connect( d->ok, SIGNAL(clicked()),
	     this, SIGNAL(applyButtonPressed()) );
    connect( d->ok, SIGNAL(clicked()),
	     this, SLOT(accept()) );
}


/*!
  Destroys the tab dialog.
*/

QTabDialog::~QTabDialog()
{
    delete d;
}


/*!
  Sets the font for the tabs to \e font.

  The weight is forced to QFont::Bold for the active tab and
  QFont::Light for the others.  (QTabDialog::font() returns the
  latter.)

  If the widget is visible, the display is updated with the new font
  immediately.  There may be some geometry changes, depending on the
  size of the old and new fonts.
*/

void QTabDialog::setFont( const QFont & font )
{
    QFont f( font );
    f.setWeight( QFont::Light );
    QDialog::setFont( f );
    setSizes();
}


/*!
  \fn void QTabDialog::applyButtonPressed();

  This signal is emitted when the Apply or OK buttons are clicked.

  It should be connected to a slot (or several slots) which change the
  application's state according to the state of the dialog.

  \sa cancelButtonPressed() defaultButtonPressed() setApplyButton()
*/


/*!
  Returns TRUE if the tab dialog has an Apply button, FALSE if not.

  \sa setApplyButton() applyButtonPressed() hasCancelButton()
  hasDefaultButton()
*/

bool QTabDialog::hasDefaultButton() const
{
     return d->db != 0;
}


/*!
  \fn void QTabDialog::cancelButtonPressed();

  This signal is emitted when the Cancel button is clicked.  It is
  automatically connected to QDialog::reject(), which will hide the
  dialog.

  The Cancel button should not change the application's state in any
  way, so generally you should not need to connect it to any slot.

  \sa applyButtonPressed() defaultButtonPressed() setCancelButton()
*/


/*!
  Returns TRUE if the tab dialog has a Cancel button, FALSE if not.

  \sa setCancelButton() cancelButtonPressed() hasDefaultButton()
  hasApplyButton()
*/

bool QTabDialog::hasCancelButton() const
{
     return d->cb != 0;
}


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
  Returns TRUE if the tab dialog has a Defaults button, FALSE if not.

  \sa setDefaultsButton() defaultButtonPressed() hasApplyButton()
  hasCancelButton()
*/

bool QTabDialog::hasApplyButton() const
{
    return d->ab != 0;
}


/*!
  Returns TRUE if the tab dialog has an OK button, FALSE if not.

  \sa setDefaultsButton() defaultButtonPressed() hasOkButton()
  hasCancelButton()
*/

bool QTabDialog::hasOkButton() const
{
    return d->ok != 0;
}


/*!
  \fn void QTabDialog::aboutToShow()

  This signal is emitted by show() when it's time to set the state of
  the dialog's contents.  The dialog should reflect the current state
  of the application when if appears; if there is any chance that the
  state of the application can change between the time you call
  QTabDialog::QTabDialog() and QTabDialog::show(), you should set the
  dialog's state in a slot and connect this signal to it.

  This applies mainly to QTabDialog objects that are kept around
  hidden rather than being created, show()n and deleted afterwards.

  \sa applyButtonPressed(), show(), cancelButtonPressed()
*/


/*!
  Shows the tab view and its children.  Reimplemented in order to
  delay show()'ing of every page except the initially visible one, and
  in order to emit the aboutToShow() signal.

  \sa hide(), aboutToShow()
*/

void QTabDialog::show()
{
    d->tabs->setFocus();
    emit aboutToShow();
    setSizes();

    // force one page to be shown now and the rest to be show only on demand
    int c = d->tabs->currentTab();
    for( int i=0; i < (int)d->children.size(); i++ )
	if ( c != i )
	    d->children[i]->hide();
    if ( c >= 0 )
	showTab( c );

    QDialog::show();
}


/*!
  Ensure that the selected tab's page is visible and appropriately sized.
*/

void QTabDialog::showTab( int i )
{
    if ( d && (uint)i < d->children.size() ) {
	d->children[i]->setGeometry( childRect() );
	d->children[i]->show();
	QWidget * f = qApp ? qApp->focusWidget() : 0;
	while ( f && f->parent() != this )
	    f = f->parentWidget();
	for ( int t = 0; t< (int)d->children.size(); t++ )
	    if ( i != t ) {
		if ( f == d->children[t] ) {
		    // move focus to first object on the incoming page,
		    // without ugly flicker.
		    bool b = d->tabs->isUpdatesEnabled();
		    d->tabs->setUpdatesEnabled( FALSE );
		    d->tabs->setFocus();
		    d->children[t]->hide();
		    focusNextPrevChild( TRUE );
		    d->tabs->setUpdatesEnabled( b );
		} else if ( d->children[t]->isVisible() ) {
		    d->children[t]->hide();
		}
	    }
	emit selected( d->tabs->tab( i )->label );
    }
}


/*!
  Add another tab and page to the tab view.

  The tab will be labelled \a label and \a child constitutes the new
  page.  Note the difference between the widget name (which you supply
  to widget constructors and to e.g. setTabEnabled()) and the tab
  label: The name is internal to the program and invariant, while the
  label is shown on screen and may vary according to e.g. language.

  \a label is written in the QButton style, where &P makes Qt create
  an accelerator key on Alt-P for this page.  For example:  

  \code
    td->addTab( graphicsPane, "&Graphics" );
    td->addTab( soundPane, "&Sound" );
  \endcode

  If the user presses Alt-S the sound page of the tab dialog is shown,
  if the user presses Alt-P the graphics page is shown.

  If you call addTab() after show(), the screen will flicker and the
  user will be confused.
*/

void QTabDialog::addTab( QWidget * child, const char * label )
{
    QTab * t = new QTab();
    CHECK_PTR( t );
    t->label = label;
    addTab( child, t );
}

/*!
  This is a lower-level method for adding tabs, similar to
  the method above.  It is useful if you are using setTabBar()
  to set a QTabBar subclass with an overridden
  QTabBar::paint() routine for a subclass of QTab.
*/
void QTabDialog::addTab( QWidget * child, QTab* tab )
{
    tab->enabled = TRUE;
    int id = d->tabs->addTab( tab );
    if ( id ==(int)d->children.size() ) {
	d->children.resize( id+1 );
	d->children[id] = child;
    } else {
#if defined(CHECK_RANGE)
	warning( "QTabDialog::addTab: Unexpected tab id %d (expected %d), "
		 "ignoring", id, d->children.size() );
#endif
    }
}

/*!
  Replaces the QTabBar heading the dialog by the given tab bar.
  Note that this must be called \e before any tabs have been added,
  or the behavior is undefined.
  \sa tabBar()
*/
void QTabDialog::setTabBar( QTabBar* tb )
{
    delete d->tabs;
    d->tabs = tb;
    connect( d->tabs, SIGNAL(selected(int)),
	     this,    SLOT(showTab(int)) );
    d->tabs->move( d->l_marg, d->t_marg );
}

/*!
  Returns the currently set QTabBar.
  \sa setTabBar()
*/
QTabBar* QTabDialog::tabBar() const
{
    return d->tabs;
}

/*!  Ensures that \a w is shown.  This is useful mainly for accelerators.

  If you use this function, take care not to surprise or confuse the
  user.

  \sa QTabBar::setCurrentTab()
*/

void QTabDialog::showPage( QWidget * w )
{
    int i;
    for( i=0; i<(int)d->children.size(); i++ )
	if ( d->children[i] == w )
	    d->tabs->setCurrentTab( i );
}


/*!
  Returns TRUE if the page with object name \a name is enabled, and
  false if it is disabled.

  If \a name is 0 or not the name of any of the pages, isTabEnabled()
  returns FALSE.

  \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabDialog::isTabEnabled( const char *name ) const
{
    int i;
    for( i=0; i<(int)d->children.size(); i++ )
	if ( qstrcmp( d->children[i]->name(), name ) == 0 )
	    return d->tabs->isTabEnabled( i );
    return FALSE;
}


/*! Finds the page with object name \a name, enables/disables it
  according to the value of \a enable, and redraws the page's tab
  appropriately.

  QTabDialog uses QWidget::setEnabled() internally, rather than keep a
  separate flag.

  Note that even a disabled tab/page may be visible.  If the page is
  visible already, QTabDialog will not hide it, and if all the pages
  are disabled, QTabDialog will show one of them.

  The object name is used (rather than the tab label) because the tab
  text may not be invariant in multi-language applications.

  \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabDialog::setTabEnabled( const char * name, bool enable )
{
    if ( !name || !*name )
	return;
    int i;
    for( i=0; i<(int)d->children.size(); i++ ) {
	if ( qstrcmp( d->children[i]->name(), name ) == 0 ) {
	    d->tabs->setTabEnabled( i, enable );
	    return;
	}
    }
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
    if ( !text ) {
	delete d->ab;
	d->ab = 0;
	setSizes();
    } else {
	if ( !d->ab ) {
	    d->ab = new QPushButton( this, "apply settings" );
	    connect( d->ab, SIGNAL(clicked()),
		     this, SIGNAL(applyButtonPressed()) );
	}
	d->ab->setText( text );
	setSizes();
	d->ab->show();
    }
}


/*!
  Add a Defaults button to the dialog.  The button's text is set to \e
  text (and defaults to "Defaults").

  The Defaults button should set the dialog (but not the application)
  back to the application defaults.

  When Defaults is clicked, the defaultButtonPressed() signal is emitted.

  \sa setApplyButton() setCancelButton() defaultButtonPressed()
*/

void QTabDialog::setDefaultButton( const char * text )
{
    if ( !text ) {
	delete d->db;
	d->db = 0;
	setSizes();
    } else {
	if ( !d->db ) {
	    d->db = new QPushButton( this, "back to default" );
	    connect( d->db, SIGNAL(clicked()),
		     this, SIGNAL(defaultButtonPressed()) );
	}
	d->db->setText( text );
	setSizes();
	d->db->show();
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
    if ( !text ) {
	delete d->cb;
	d->cb = 0;
	setSizes();
    } else {
	if ( !d->cb ) {
	    d->cb = new QPushButton( this, "cancel dialog" );
	    connect( d->cb, SIGNAL(clicked()),
		     this, SIGNAL(cancelButtonPressed()) );
	    connect( d->cb, SIGNAL(clicked()),
		     this, SLOT(reject()) );
	}
	d->cb->setText( text );
	setSizes();
	d->cb->show();
    }
}


/*!
  Set the appropriate size and position for each of the fixed children.

  Finally set the minimum and maximum sizes for the dialog.

  \sa setApplyButton() setCancelButton() setDefaultButton()
*/

void QTabDialog::setSizes()
{
    d->tabs->move( d->l_marg, d->t_marg );

    // compute largest button size
    int bw;
    QWidget* first_button = d->ok ?
		    d->ok : d->db ?
		    d->db : d->ab ?
		    d->ab : d->cb ? 
		    d->cb : 0;
    QSize s( first_button ? d->ok->sizeHint() : QSize(0,0) );
    bw = s.width();
    d->bh = s.height(); // private member, used for GM

    if ( d->ab && d->ab != first_button ) {
	s = d->ab->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > d->bh )
	    d->bh = s.height();
    }

    if ( d->db && d->ab != first_button ) {
	s = d->db->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > d->bh )
	    d->bh = s.height();
    }

    if ( d->cb && d->ab != first_button ) {
	s = d->cb->sizeHint();
	if ( s.width() > bw )
	    bw = s.width();
	if ( s.height() > d->bh )
	    d->bh = s.height();
    }

    if ( style() == WindowsStyle && bw < 75 )
	bw = 75;

    // and set all the buttons to that size
    if ( d->ok )
	d->ok->resize( bw, d->bh );
    if ( d->ab )
	d->ab->resize( bw, d->bh );
    if ( d->db )
	d->db->resize( bw, d->bh );
    if ( d->cb )
	d->cb->resize( bw, d->bh );

    // look at the pages and find the largest minimum and smallest
    // maximum size
    QSize min( d->tabs->sizeHint() );
    d->tabs->resize( min ); // resize it just in case
    QSize max(QCOORD_MAX,QCOORD_MAX);
    int th = min.height();
    int i;
    for ( i=0; i<(int)d->children.size(); i++ ) {
	// maximum sizes
	if ( d->children[i]->maximumSize().height() < max.height() )
	    max.setHeight( d->children[i]->maximumSize().height() );
	if ( d->children[i]->maximumSize().width() < max.width() )
	    max.setWidth( d->children[i]->maximumSize().width() );
	// minimum sizes
	if ( d->children[i]->minimumSize().height() > min.height() )
	    min.setHeight( d->children[i]->minimumSize().height() );
	if ( d->children[i]->minimumSize().width() > min.width() )
	    min.setWidth( d->children[i]->minimumSize().width() );
    }
    min.setHeight( min.height() + th );

    // factor in the buttons.  six pixels between each button, seven
    // (6+1) to the right and seven to the left.
    bw = (bw + 6) * ( (d->ab ? 1 : 0) + (d->db ? 1 : 0) + (d->cb ? 1 : 0)
	 + (d->ok ? 1 : 0 ) ) + 1 + d->l_marg;
    if ( min.width() < bw )
	min.setWidth( bw );

    // max must be >= min
    if ( max.width() < min.width() )
	max.setWidth( min.width() );
    if ( max.height() < min.height() )
	max.setHeight( min.height() );

    // allow for own borders, buttons and tabs, and set own sizes
    min.setWidth( QMIN( min.width() + 17, 32767 ) );
    min.setHeight( QMIN( d->t_marg + 1 + th + min.height()
		       + d->t_marg + d->bh + 8, 32767 ) );
    max.setWidth( QMIN( max.width() + 17, 32767 ) );
    max.setHeight( QMIN( d->t_marg + 1 + th + max.height()
		       + d->t_marg + d->bh + 8, 32767 ) );
    setMinimumSize( min );
    setMaximumSize( max );

    // fake a resize event to trigger child widget moves
    QResizeEvent r( size(), size() );
    resizeEvent( &r );

    // fiddle the tab chain so the buttons are in their natural order
    QWidget * w = first_button;
    if ( d->db ) {
	setTabOrder( w, d->db );
	w = d->db;
    }
    if ( d->ab ) {
	setTabOrder( w, d->ab );
	w = d->ab;
    }
    if ( d->cb ) {
	setTabOrder( w, d->cb );
	w = d->cb;
    }
    setTabOrder( w, d->tabs );
}




/*!
  Handles resize events for the tab dialog.

  All of the pages are resized, and the buttons moved into position.
*/

void QTabDialog::resizeEvent( QResizeEvent * )
{
    if ( d->tabs ) {
	QSize ts = d->tabs->sizeHint();
	if ( ts.width() > width() ) {
	    ts.setWidth( width() );
	}
	d->tabs->resize( ts );

	int x;
	x = width();

	if ( d->cb ) {
	    d->cb->move( x - d->l_marg - d->cb->width(), height() - d->b_marg - d->bh );
	    x = d->cb->geometry().x();
	}

	if ( d->ab ) {
	    d->ab->move( x - d->l_marg - d->ab->width(), height() - d->b_marg - d->bh );
	    x = d->ab->geometry().x();
	}

	if ( d->db ) {
	    d->db->move( x - d->l_marg - d->db->width(), height() - d->b_marg - d->bh );
	    x = d->db->geometry().x();
	}

	if ( d->ok ) {
	    d->ok->move( x - d->l_marg - d->ok->width(), height() - d->b_marg - d->bh );
	}

	int i;
	for ( i=0; i<(int)d->children.size(); i++ )
	    d->children[i]->setGeometry( childRect() );
    }
}


/*!
  Handles paint events for the tabbed dialog
*/

void QTabDialog::paintEvent( QPaintEvent * )
{
    if ( !d->tabs )
	return;

    QPainter p;
    p.begin( this );

    QCOORD t = childRect().top() - 1;
    QCOORD b = childRect().bottom() + 2;
    QCOORD r = childRect().right() + 2;
    QCOORD l = childRect().left() - 1;

    p.setPen( white );
    // note - this line overlaps the bottom line drawn by QTabBar
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


QRect QTabDialog::childRect() const
{
    int y = d->tabs->height() + d->tabs->y();
    return QRect( d->l_marg + 1, y,
	    width() - d->l_marg - d->l_marg - 1 - 2,
	    height() - d->t_marg - 2 - d->bh - d->b_marg - y );
}


/*!
  Set the OK button's text to \a text (which defaults to "OK").

  The OK button should apply the current settings in the dialog box to
  the application and then close the dialog.

  When Apply is clicked, the applyButtonPressed() signal is emitted.

  \sa setCancelButton() setDefaultButton() applyButtonPressed()
*/

void QTabDialog::setOkButton( const char * text )
{
    if ( !text ) {
	delete d->ok;
	d->ok = 0;
	setSizes();
    } else {
	if ( !d->ok ) {
	    d->ok = new QPushButton( this, "ok" );
	    connect( d->ok, SIGNAL(clicked()),
		     this, SIGNAL(applyButtonPressed()) );
	}
	d->ok->setText( text );
	setSizes();
	d->ok->show();
    }
}

/*!
  Old version of setOkButton(), provided for backward compatibility.
 */
void QTabDialog::setOKButton( const char * text )
{
    setOkButton( text );
}


/*!  Returns the text in the tab for page \a w.
*/

const char * QTabDialog::tabLabel( QWidget * w )
{
    int i;
    for( i=0; i<(int)d->children.size(); i++ ) {
	if ( d->children[i] == w ) {
	    QTab * t = d->tabs->tab(i);
	    if ( t )
		return t->label;
	}
    }
    return 0;
}	    


/*!  Reimplemented to hndle a change of GUI style while on-screen.
*/

void QTabDialog::styleChange( GUIStyle s )
{
    QDialog::styleChange( s );
    setSizes();
}
