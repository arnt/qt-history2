/****************************************************************************
** $Id$
**
** Implementation of QToolBox widget class
**
** Created : 961105
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtoolbox.h"
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qscrollview.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qobjectlist.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qlayout.h>
#include <qptrdict.h>
#include <qtooltip.h>
#include <qeventloop.h>
#include <qdatetime.h>
#include <qcursor.h>
#ifdef Q_OS_WIN32
#include <windows.h>
#endif

class QToolBoxButton : public QToolButton
{
public:
    QToolBoxButton( QWidget *parent, const char *name ) :
	QToolButton( parent, name ), selected( FALSE )
	{ setBackgroundMode( PaletteLight ); }

    void setSelected( bool b ) { selected = b; update(); }
    virtual void setIcon( const QIconSet &is ) { ic = is; }
    QIconSet icon() const { return ic; }

    QSize sizeHint() const;

protected:
    void drawButton( QPainter * );

private:
    bool selected;
    QIconSet ic;

};

class QToolBoxPrivate
{
public:
    struct Page
    {
	QToolButton *button;
	QString label;
	QIconSet iconSet;
	QString toolTip;
    };

    QToolBoxPrivate()
	{
	    currentPage = 0;
	    lastButton = 0;
	    pageList = new QPtrList<Page>;
	    pageList->setAutoDelete( TRUE );
	}

    ~QToolBoxPrivate()
	{
	    delete pageList;
	}

    QToolBoxButton *button( QWidget *page )
	{
	    QPtrDictIterator<QWidget> it( pages );
	    while ( it.current() ) {
		if ( it.current() == page )
		    return (QToolBoxButton*)it.currentKey();
		++it;
	    }
	    return 0;
	}

    Page *page( QWidget *page )
	{
	    QToolBoxButton *b = button( page );
	    for ( QToolBoxPrivate::Page *c = pageList->first(); c;
		  c = pageList->next() ) {
		if ( c->button == b )
		    return c;
	    }
	    return 0;
	}

    // #### improve that algorithm // at the moment that one finds the
    // #### first available page, but it really should look in both
    // #### directions and find the closest available page
    QWidget *findClosestPage( QWidget *page )
	{
	    QWidget *p;
	    QPtrDictIterator<QWidget> it( pages );
	    while ( it.current() ) {
		if ( it.current() != page ) {
		    p = it.current();
		    break;
		}
		++it;
	    }
	    return p;

	}

    QPtrDict<QWidget> pages;
    QPtrList<Page> *pageList;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QToolBoxButton *lastButton;
};


QSize QToolBoxButton::sizeHint() const
{
    int ih = 0;
    if ( !ic.isNull() )
	ih = icon().pixmap( QIconSet::Small, QIconSet::Normal ).height() + 6;
    return QToolButton::sizeHint().expandedTo( QSize( 0, ih ) );
}

void QToolBoxButton::drawButton( QPainter *p )
{
    QStyle::SFlags flags = QStyle::Style_Default;

    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( selected )
	flags |= QStyle::Style_Selected;
    if ( hasFocus() )
	flags |= QStyle::Style_HasFocus;
    style().drawControl( QStyle::CE_ToolBoxTab, p, this, rect(),
			 colorGroup(), flags );

    int d = 20 + height() - 3;
    QRect tr, ir;
    int ih = 0;
    if ( icon().isNull() ) {
	tr = QRect( 2, 2, width() - d - 5, height() );
    } else {
	int iw = icon().pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	if ( width() < 4 * iw )
	    ih = iw = 0;
	else
	    ih = icon().pixmap( QIconSet::Small, QIconSet::Normal ).height();
	ir = QRect( 0, 0, iw + 2, ih );
	tr = QRect( ir.width() + 4, 0, width() - (ir.width() + 4) - d - 5, height() );
    }

    QString txt;
    if ( p->fontMetrics().width( text() ) < tr.width() ) {
	txt = text();
    } else {
	txt = text().left( 1 );
	int ew = p->fontMetrics().width( "..." );
	int i = 1;
	while ( p->fontMetrics().width( txt ) + ew +
		p->fontMetrics().width( text()[i] )  < tr.width() )
	    txt += text()[i++];
	txt += "...";
    }

    if ( selected ) {
	QFont f( p->font() );
	f.setBold( TRUE );
	p->setFont( f );
    }

    if ( ih )
	p->drawPixmap( ir.left(), (height() - ih) / 2,
		       icon().pixmap( QIconSet::Small, QIconSet::Normal ) );
    style().drawItem( p, tr, AlignLeft | AlignVCenter | ShowPrefix, colorGroup(),
		      isEnabled(), 0, txt );

    if ( !txt.isEmpty() && hasFocus() )
	style().drawPrimitive( QStyle::PE_FocusRect, p, tr, colorGroup() );
}

// ### is this needed, seems hacky
static void set_background_mode( QWidget *top, Qt::BackgroundMode bm )
{
    QObjectList *l = top->queryList( "QWidget" );
    l->append( top );
    for ( QObject *o = l->first(); o; o = l->next() )
	( (QWidget*)o )->setBackgroundMode( bm );
    delete l;
}


/*! \class QToolBox qtoolbox.h

  \brief The QToolBox class provides a stack of tabbed widgets

  A toolbox is a widget that display the tabs below each other and the
  current page is displayed below the current tab.

  To add pages to QToolBox, call addPage() or insertPage(). To remove
  a page again, use removePage().

  A page can be enabled or disabled using setPageEnabled(). The label,
  iconset and tooltip of a page can be modified using setPageLabel(),
  setPageIconSet() and setPageToolTip().

  The number of pages can be retrieved using count(); to set the
  current page, use setCurrentPage() and to retrieve it, call
  currentPage(). If the current page changes, currentChanged() is
  emitted.

  To map a page to its index or vica versa, use page() or pageIndex().

  \sa QTabWidget
*/

/*! \fn void QToolBox::currentChanged( QWidget *page )
  This signal is emitted when the current page is changed to \a page.
*/

/*! Constructs a toolbox called \a name with parent \a parent.
*/


QToolBox::QToolBox( QWidget *parent, const char *name )
    :  QWidget( parent, name )
{
    d = new QToolBoxPrivate;
    d->layout = new QVBoxLayout( this );
}

/*! \reimp */

QToolBox::~QToolBox()
{
    delete d;
}

/*! \overload

  Adds the page \a page to bottom of the toolbox. The label of the
  page's tab will be set to \a label.
*/

void QToolBox::addPage( const QString &label, QWidget *page )
{
    addPage( label, QIconSet(), page );
}

/*!Adds the page \a page to bottom of the toolbox. The label of the
  page's tab will be set to \a label and \a iconSet will be displayed
  in front of it.
*/

void QToolBox::addPage( const QString &label, const QIconSet &iconSet,
				QWidget *page )
{
    insertPage( label, iconSet, page );
}

/*! \overload

  Insert the page \a page at index \a index to the toolbox. The
  label of the page's tab will be set to \a label.
*/

void QToolBox::insertPage( const QString &label, QWidget *page, int index )
{
    insertPage( label, QIconSet(), page, index );
}

/*! Insert the page \a page at index \a index to the toolbox. The
  label of the page's tab will be set to \a label and \a iconSet will
  be displayed in front of it.
*/

void QToolBox::insertPage( const QString &label, const QIconSet &iconSet,
				   QWidget *page, int index )
{
    page->setBackgroundMode( PaletteBackground );

    QToolBoxButton *button = new QToolBoxButton( this, label.latin1() );
    QToolBoxPrivate::Page *c = new QToolBoxPrivate::Page;
    c->button = button;
    c->label = label;
    c->iconSet = iconSet;
    bool needRelayout = FALSE;
    if ( index < 0 || index >= count() ) {
	d->pageList->append( c );
    } else {
	d->pageList->insert( index, c );
	needRelayout = TRUE;
    }

    button->setText( label );
    if ( !iconSet.isNull() )
	button->setIcon( iconSet );
    button->setFixedHeight( button->sizeHint().height() );
    connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );

    QScrollView *sv = new QScrollView( this );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    sv->addChild( page );
    sv->setFrameStyle( QFrame::NoFrame );

    d->pages.insert( button, sv );

    if ( !needRelayout ) {
	d->layout->addWidget( button );
	d->layout->addWidget( sv );
    } else {
	relayout();
    }

    page->show();
    button->show();

    if ( count() == 1 ) {
	d->currentPage = sv;
	d->lastButton = button;
	d->lastButton->setSelected( TRUE );
	sv->show();
	// #### is this needed, seems hacky
	set_background_mode( d->currentPage, PaletteLight );
    } else {
	sv->hide();
    }

    updateTabs();
}

#ifndef Q_OS_WIN32

#define INIT_TIMER \
QTime t; \
t.start()

#define CHECK_TIMER \
if ( t.elapsed() > 1 )

#define REINIT_TIMER \
t.restart()

#else

#define INIT_TIMER \
LARGE_INTEGER count, next, oneMill; \
QueryPerformanceFrequency( &count ); \
QueryPerformanceCounter( &next ); \
oneMill.QuadPart = count.QuadPart/1000

#define CHECK_TIMER \
QueryPerformanceCounter( &count ); \
if ( count.QuadPart > next.QuadPart )

#define REINIT_TIMER \
next.QuadPart = next.QuadPart + oneMill.QuadPart;

#endif

void QToolBox::buttonClicked()
{
    QToolBoxButton *tb = (QToolBoxButton*)sender();
    QWidget *page = d->pages.find( tb );

    if ( page == d->currentPage )
	return;


    if ( qApp->isEffectEnabled( UI_AnimateToolBox ) ) {
	// ### This implementation can be improved by resizing the old
	// ### and new current page accordingly. This will improve the
	// ### visual effect
	int direction = 0;

	QWidgetList buttons;
	for ( QToolBoxPrivate::Page *c = d->pageList->first(); c;
	      c = d->pageList->next() ) {
	    if ( c->button == tb ) {
		if ( direction < 0 ) {
		    buttons.append( c->button );
		    break;
		}
		direction = 8;
	    } else if ( c->button == d->lastButton ) {
		if ( direction > 0 ) {
		    buttons.append( c->button );
		    break;
		}
		direction = -8;
	    } else if ( direction != 0 ) {
		buttons.append( c->button );
	    }
	}

	int dist = 0;
	int h = d->currentPage->parentWidget()->height() - d->lastButton->height();

	INIT_TIMER;
	while ( dist < h ) {
	    CHECK_TIMER {
		QWidgetListIt it( buttons );
		while ( it.current() ) {
		    it.current()->raise();
		    it.current()->move( it.current()->x(),
					it.current()->y() + direction );
		    ++it;
		}
		dist += QABS( direction );
		REINIT_TIMER;
	    }
	    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
	}
    }

    setCurrentPage( page );
}

void QToolBox::updateTabs()
{
    bool after = FALSE;
    for ( QToolBoxPrivate::Page *c = d->pageList->first(); c;
	  c = d->pageList->next() ) {
	c->button->setBackgroundMode( !after ? PaletteBackground : PaletteLight );
	c->button->update();
	after = c->button == d->lastButton;
    }
}

/*! \property QToolBox::count
  \brief returns the number of pages contained by the toolbox
*/

int QToolBox::count() const
{
    return d->pageList->count();
}

void QToolBox::setCurrentPage( int index )
{
    setCurrentPage( page( index ) );
}

/*! Sets the page \a page to be the current one. */

void QToolBox::setCurrentPage( QWidget *page )
{
    if ( !page || d->currentPage == page )
	return;

    QToolBoxButton *tb = d->button( page );
    if( !tb )
	return;

    tb->setSelected( TRUE );
    if ( d->lastButton )
	d->lastButton->setSelected( FALSE );
    d->lastButton = tb;
    if ( d->currentPage )
	d->currentPage->hide();
    d->currentPage = page;
    d->currentPage->show();
    // #### is this needed, seems hacky
    set_background_mode( d->currentPage, PaletteLight );
    updateTabs();
    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
    emit currentChanged( page );
}

void QToolBox::relayout()
{
    delete d->layout;
    d->layout = new QVBoxLayout( this );
    for ( QToolBoxPrivate::Page *c = d->pageList->first(); c;
	  c = d->pageList->next() ) {
	d->layout->addWidget( c->button );
	d->layout->addWidget( d->pages.find( c->button ) );
    }
    QWidget *currPage = d->currentPage;
    d->currentPage = 0;
    setCurrentPage( currPage );
}

/*! Removes the page \a page from the toolbox. Note that this function
  does not delete the widget \a page! */

void QToolBox::removePage( QWidget *page )
{
    if ( !page )
	return;

    QToolBoxButton *tb = d->button( page );
    if ( !tb )
	return;

    activateClosestPage( page );

    page->hide();
    tb->hide();
    d->layout->remove( page );
    d->layout->remove( tb );
}

/*! Returns the current page of the toolbox.
*/

QWidget *QToolBox::currentPage() const
{
    return d->currentPage;
}

/*! \property QToolBox::currentPage
  \brief the index of the current page of the toolbox
*/

int QToolBox::currentIndex() const
{
    return pageIndex( d->currentPage );
}

/*! Returns the page which is located at index \a index in the
  toolbox */

QWidget *QToolBox::page( int index ) const
{
    return d->pages.find( d->pageList->at( index )->button );
}

/*! Returns the index at which the page \a page is located */

int QToolBox::pageIndex( QWidget *page ) const
{
    QToolBoxButton *tb = d->button( page );
    int i = 0;
    for ( QToolBoxPrivate::Page *c = d->pageList->first(); c;
	  c = d->pageList->next(), ++i ) {
	if ( c->button == tb )
	    return i;
    }
    return -1;
}

/*! Enabled the page \a page, if \a enabled is TRUE. Disables the page
  otherwise. */

void QToolBox::setPageEnabled( QWidget *page, bool enabled )
{
    QToolBoxButton *tb = d->button( page );
    if ( !tb )
	return;

    if ( !enabled )
	activateClosestPage( page );

    tb->setEnabled( enabled );
}

void QToolBox::activateClosestPage( QWidget *page )
{
    if ( page != d->currentPage )
	return;

    QWidget *p = 0;
    if ( page == d->currentPage ) {
	p = d->findClosestPage( page );
	if ( !p ) {
	    d->currentPage = 0;
	    d->lastButton = 0;
	} else {
	    setCurrentPage( p );
	}
    }
}

/*! Sets the page's tab label of page \a page to \a label. */

void QToolBox::setPageLabel( QWidget *page, const QString &label )
{
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->label = label;
    c->button->setText( label );
}

/*! Sets the page's tab icon set of page \a page to \a iconSet. */

void QToolBox::setPageIconSet( QWidget *page, const QIconSet &iconSet )
{
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->iconSet = iconSet;
    ( (QToolBoxButton*)c->button )->setIcon( iconSet );
}

/*! Sets the page's tab tooltip set of page \a page to \a tooltip. */

void QToolBox::setPageToolTip( QWidget *page, const QString &toolTip )
{
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->toolTip = toolTip;
    QToolTip::remove( tb );
    QToolTip::add( tb, toolTip );
}

/*! Returns TRUE if the page \a page is enabled, FALSE otherwise. */

bool QToolBox::isPageEnabled( QWidget *page ) const
{
    QToolBoxButton *tb = d->button( page );
    return tb && tb->isEnabled();
}

/*! Returns the label of the \a page's tab. */

QString QToolBox::pageLabel( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->label : QString::null;
}

/*! Returns the icon set of the \a page's tab. */

QIconSet QToolBox::pageIconSet( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->iconSet : QIconSet();
}

/*! Returns the tooltip set of the \a page's tab. */

QString QToolBox::pageToolTip( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->toolTip : QString::null;
}
