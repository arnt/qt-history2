/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabwidget.cpp#6 $
**
** Implementation of QTabWidget class
**
** Created : 990318
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

#include "qtabwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qtabbar.h"
#include "qapplication.h"
#include "qwidgetstack.h"
/*!
  \class QTabWidget qtabwidget.h

  \brief The QTabWidget class provides a stack of tabbed widgets.

  \ingroup realwidgets

  A tabbed widget is one in which several "pages" are available, and
  the user selects which page to see and use by clicking on its tab,
  or by pressing the indicated Alt-(letter) key combination.

  QTabWidget does not provide more than one row of tabs, and does not
  provide tabs along the sides or bottom of the pages.  It also does
  not offer any way to find out which page is currently visible or to
  set the visible page.


  The normal way to use QTabWidget is to do the following in the
  constructor: <ol> <li> Create a QTabWidget. <li> Create a QWidget
  for each of the pages in the tab dialog, insert children into it,
  set up geometry management for it, and use addTab() to set up a tab
  and keyboard accelerator for it. <li> Connect to the
  signals and slots. </ol>

  If you don't call addTab(), the page you have created will not be
  visible.  Please don't confuse the object name you supply to the
  QWidget constructor and the tab label you supply to addTab():
  addTab() takes a name which indicates an accelerator and is
  meaningful and descriptive to the user, while the widget name is
  used primarily for debugging.

  A signal selected() is emitted when the user selects some page.

  Each tab is either enabled or disabled at any given time.  If a tab
  is enabled, the tab text is drawn in black and the user can select
  that tab.  If it is disabled, the tab is drawn in a different way
  and the user can not select that tab.  Note that even though a tab
  is disabled, the page can still be visible, for example if all of
  the tabs happen to be disabled.

  While tab widgets can be a very good way to split up a complex
  dialog, it's also very easy to make a royal mess out of it. See
  QTabDialog for some design hints.

  Most of the functionality in QTabWidget is provided by a QTabBar (at
  the top, providing the tabs) and a QWidgetStack (most of the area,
  organizing the individual pages).

  <img src=qtabdlg-m.gif> <img src=qtabdlg-w.gif>

  \sa QTabDialog
*/

/*! \fn void QTabWidget::selected( const QString &tabLabel );

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/

class QTabWidgetData
{
public:
    QTabWidgetData()
	: tabs(0), stack(0), dirty( TRUE ), pos( QTabWidget::Top )
	{};
    ~QTabWidgetData(){};
    QTabBar* tabs;
    QWidgetStack* stack;
    bool dirty;
    QTabWidget::TabPosition pos;
};

/*!
  Constructs a QTabWidget
*/
QTabWidget::QTabWidget( QWidget *parent, const char *name)
    : QWidget( parent, name )
{
    d = new QTabWidgetData;

    d->stack = new QWidgetStack( this, "tab pages" );
    setTabBar( new QTabBar( this, "tab control" ) );
    d->tabs->setShape( QTabBar::TriangularAbove );

    d->stack->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    d->stack->setLineWidth( style().defaultFrameWidth() );
}

QTabWidget::~QTabWidget()
{
}

/*!
  Adds another tab and page to the tab view.

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
void QTabWidget::addTab( QWidget *child, const QString &label)
{
    QTab * t = new QTab();
    CHECK_PTR( t );
    t->label = label;
    addTab( child, t );
}

/*!
  This is a lower-level method for adding tabs, similar to the other
  addTab() method.  It is useful if you are using setTabBar() to set a
  QTabBar subclass with an overridden QTabBar::paint() routine for a
  subclass of QTab.
*/
void QTabWidget::addTab( QWidget *child, QTab* tab)
{
    tab->enabled = TRUE;
    int id = d->tabs->addTab( tab );
    d->stack->addWidget( child, id );
    d->tabs->setMinimumSize( d->tabs->sizeHint() );
    setUpLayout();
}

/*!
  Returns TRUE if the page with object name \a name is enabled, and
  false if it is disabled.

  If \a name is 0 or not the name of any of the pages, isTabEnabled()
  returns FALSE.

  \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabWidget::isTabEnabled( const QString & name) const
{
    if ( name.isEmpty() )
	return FALSE;

    QObjectList * l
	= ((QTabWidget *)this)->queryList( "QWidget", name, FALSE, TRUE );
    bool r = FALSE;
    if ( l && l->first() ) {
	QWidget * w;
	while( l->current() ) {
	    while( l->current() && !l->current()->isWidgetType() )
		l->next();
	    w = (QWidget *)(l->current());
	    if ( w && d->stack->id(w) ) {
		r = w->isEnabled();
		delete l;
		return r;
	    }
	    l->next();
	}
    }
    delete l;
    return r;
}

/*!
  Finds the page with object name \a name, enables/disables it
  according to the value of \a enable, and redraws the page's tab
  appropriately.

  QTabWidget uses QWidget::setEnabled() internally, rather than keep a
  separate flag.

  Note that even a disabled tab/page may be visible.  If the page is
  visible already, QTabWidget will not hide it, and if all the pages
  are disabled, QTabWidget will show one of them.

  The object name is used (rather than the tab label) because the tab
  text may not be invariant in multi-language applications.

  \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabWidget::setTabEnabled( const QString & name, bool enable)
{
    if ( name.isEmpty() )
	return;
    QObjectList * l
	= ((QTabWidget *)this)->queryList( "QWidget", name, FALSE, TRUE );
    if ( l && l->first() ) {
	QWidget * w;
	while( l->current() ) {
	    while( l->current() && !l->current()->isWidgetType() )
		l->next();
	    w = (QWidget *)(l->current());
	    if ( w ) {
		int id = d->stack->id( w );
		if ( id ) {
		    w->setEnabled( enable );
		    d->tabs->setTabEnabled( id, enable );
		}
		delete l;
		return;
	    }
	    l->next();
	}
    }
    delete l;
}

/*!  Ensures that \a w is shown.  This is useful mainly for accelerators.

  \warning Used carelessly, this function can easily surprise or
  confuse the user.

  \sa QTabBar::setCurrentTab()
*/
void QTabWidget::showPage( QWidget * w)
{
    int i = d->stack->id( w );
    if ( i >= 0 ) {
	d->stack->raiseWidget( w );
	d->tabs->setCurrentTab( i );
    }
}

/*!  Returns the text in the tab for page \a w.
*/

QString QTabWidget::tabLabel( QWidget * w)
{
    QTab * t = d->tabs->tab( d->stack->id( w ) );
    return t ? t->label : QString::null;
}

/*!  Returns a pointer to the page currently being displayed by the
tab dialog.  The tab dialog does its best to make sure that this value
is never 0, but if you try hard enough it can be.
*/

QWidget * QTabWidget::currentPage() const
{
    return d->stack->visibleWidget();
}

/*\reimp
 */
void QTabWidget::resizeEvent( QResizeEvent * )
{
    setUpLayout();
}

/*!
  Replaces the QTabBar heading the dialog by the given tab bar.
  Note that this must be called \e before any tabs have been added,
  or the behavior is undefined.
  \sa tabBar()
*/
void QTabWidget::setTabBar( QTabBar* tb)
{
    delete d->tabs;
    d->tabs = tb;
    connect( d->tabs, SIGNAL(selected(int)),
	     this,    SLOT(showTab(int)) );
    d->tabs->setMinimumSize( d->tabs->sizeHint() );
    setUpLayout();
}


/*!
  Returns the currently set QTabBar.
  \sa setTabBar()
*/
QTabBar* QTabWidget::tabBar() const
{
    return d->tabs;
}

/*!
  Ensures that the selected tab's page is visible and appropriately sized.
*/

void QTabWidget::showTab( int i )
{
    if ( d->stack->widget( i ) ) {
	d->stack->raiseWidget( i );
	emit selected( d->tabs->tab( i )->label );
    }
}

/*!
  Set up the layout.
 */
void QTabWidget::setUpLayout( bool onlyCheck )
{
    if ( onlyCheck && !d->dirty )
	return; // nothing to do

    if ( !isVisible() ) {
	d->dirty = TRUE;
	return; // we'll do it later
    }
    QSize t( d->tabs->sizeHint() );
    int lw = d->stack->lineWidth();
    if ( d->pos == Bottom ) {
	d->tabs->setGeometry( QMAX(0, lw-2), height() - t.height(), t.width(), t.height() );
	d->stack->setGeometry( 0, 0, width(), height()-t.height()+lw );
    }
    else { // Top
	d->tabs->setGeometry( QMAX(0, lw-2), 0, t.width(), t.height() );
	d->stack->setGeometry( 0, t.height()-lw, width(), height()-t.height()+lw );
    }
	
    d->dirty = FALSE;
}

/*!
  Returns a suitable size for the tab widget.
*/

QSize QTabWidget::sizeHint() const
{
    QSize s( d->stack->sizeHint() );
    QSize t( d->tabs->sizeHint() );
    return QSize( QMAX( s.width(), t.width() ),
		  s.height() + t.height() );
}

/*! \reimp
 */
void QTabWidget::showEvent( QShowEvent * )
{
    setUpLayout( TRUE );
}


/*!
  Returns the position of the tabs.

  Possible values are QTabWidget::Top and QTabWidget::Bottom.
  \sa setTabPosition()
 */
QTabWidget::TabPosition QTabWidget::tabPosition() const
{
    return d->pos;
}

/*!
  Sets the position of the tabs to \e pos

  Possible values are QTabWidget::Top and QTabWidget::Bottom.
  \sa tabPosition()
 */
void QTabWidget::setTabPosition( QTabWidget::TabPosition pos)
{
    if (d->pos == pos)
	return;
    d->pos = pos;
    if (d->tabs->shape() == QTabBar::TriangularAbove || d->tabs->shape() == QTabBar::TriangularBelow ) {
	if ( pos == Bottom )
	    d->tabs->setShape( QTabBar::TriangularBelow );
	else
	    d->tabs->setShape( QTabBar::TriangularAbove );
    }
    else {
	if ( pos == Bottom )
	    d->tabs->setShape( QTabBar::RoundedBelow );
	else
	    d->tabs->setShape( QTabBar::RoundedAbove );
    }
    setUpLayout();
}



/*!
  Returns the width of the margin. The margin is the distance between
  the innermost pixel of the frame and the outermost pixel of the
  pages.

  \sa setMargin()
*/
int QTabWidget::margin() const
{
    return d->stack->margin();
}

/*!
  Sets the width of the margin to \e w.
  \sa margin()
*/
 void QTabWidget::setMargin( int w )
{
    d->stack->setMargin( w );
    setUpLayout();
}


/*! \reimp
 */
void QTabWidget::styleChange( GUIStyle )
{
    d->stack->setLineWidth( style().defaultFrameWidth() );
    setUpLayout();
}
