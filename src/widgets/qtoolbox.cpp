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
    Q_OBJECT

public:
    QToolBoxButton( QWidget *parent, const char *name ) :
	QToolButton( parent, name ), selected( FALSE )
    {}

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

    inline QToolBoxPrivate()
	: currentPage( 0 ), lastButton( 0 ), pageBackgroundMode( Qt::NoBackground )
	{
	    pageList.setAutoDelete( TRUE );
	}


    QToolBoxButton *button( QWidget *page );
    Page *page( QWidget *page );
    void updatePageBackgroundMode();
    void updateTabs( QToolBox *tb );

    // #### improve that algorithm at the moment that one finds the
    // #### first available page, but it really should look in both
    // #### directions and find the closest available page
    inline QWidget *findClosestPage( QWidget *page )
	{
	    QWidget *p = 0;
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
    QPtrList<Page> pageList;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QToolBoxButton *lastButton;
    Qt::BackgroundMode pageBackgroundMode;
};

QToolBoxPrivate::Page *QToolBoxPrivate::page( QWidget *page )
{
    QToolBoxButton *b = button( page );
    for ( QToolBoxPrivate::Page *c = pageList.first(); c;
	  c = pageList.next() ) {
	if ( c->button == b )
	    return c;
    }
    return 0;
}

QToolBoxButton *QToolBoxPrivate::button( QWidget *page )
{
    QPtrDictIterator<QWidget> it( pages );
    while ( it.current() ) {
	if ( it.current() == page )
	    return (QToolBoxButton*)it.currentKey();
	++it;
    }
    return 0;
}

void QToolBoxPrivate::updatePageBackgroundMode()
{
    if ( pageBackgroundMode == Qt::NoBackground || !currentPage )
	return;
    QObjectList *l =
	((QScrollView*)currentPage)->viewport()->queryList( "QWidget" );
    for ( QObject *o = l->first(); o; o = l->next() )
	((QWidget*)o)->setBackgroundMode( pageBackgroundMode );
}

void QToolBoxPrivate::updateTabs( QToolBox *tb )
{
    bool after = FALSE;
    for ( QToolBoxPrivate::Page *c = pageList.first(); c;
	  c = pageList.next() ) {
	c->button->setBackgroundMode( (!after ||
				       pageBackgroundMode == Qt::NoBackground) ?
				      tb->backgroundMode() : pageBackgroundMode );
	c->button->update();
	after = c->button == lastButton;
    }
}





QSize QToolBoxButton::sizeHint() const
{
    int ih = 0;
    if ( !ic.isNull() )
	ih = ic.pixmap( QIconSet::Small, QIconSet::Normal ).height() + 6;
    return QToolButton::sizeHint().expandedTo( QSize( 0, ih ) );
}

void QToolBoxButton::drawButton( QPainter *p )
{
    QStyle::SFlags flags = QStyle::Style_Default;
    const QColorGroup &cg = colorGroup();

    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( selected )
	flags |= QStyle::Style_Selected;
    if ( hasFocus() )
	flags |= QStyle::Style_HasFocus;
    style().drawControl( QStyle::CE_ToolBoxTab, p, parentWidget(), rect(),
			 cg, flags );

    int d = 20 + height() - 3;
    QRect tr, ir;
    int ih = 0;
    if ( icon().isNull() ) {
	tr = QRect( 2, 0, width() - d - 5, height() );
    } else {
	int iw = icon().pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	if ( width() < 4 * iw )
	    ih = iw = 0;
	else
	    ih = icon().pixmap( QIconSet::Small, QIconSet::Normal ).height();
	ir = QRect( 2, 2, iw + 2, ih );
	tr = QRect( ir.width() + 4, 0, width() - (ir.width() + 4) - d - 5, height() );
    }

    if ( selected
	 && style().styleHint( QStyle::SH_ToolBox_SelectedPageTitleBold ) ) {
	QFont f( p->font() );
	f.setBold( TRUE );
	p->setFont( f );
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

    if ( ih )
	p->drawPixmap( ir.left(), (height() - ih) / 2,
		       icon().pixmap( QIconSet::Small, QIconSet::Normal ) );
    QToolBox *tb = (QToolBox*)parentWidget();

    QColor c = tb->paletteForegroundColor();
    if ( selected &&
	 style().styleHint( QStyle::SH_ToolBox_SelectedPageTitleBold ) &&
	 tb->pageBackgroundMode() != NoBackground )
	c = cg.color( QPalette::foregroundRoleFromMode( tb->pageBackgroundMode() ) );

    style().drawItem( p, tr, AlignLeft | AlignVCenter | ShowPrefix, cg,
		      isEnabled(), 0, txt, -1, &c );

    if ( !txt.isEmpty() && hasFocus() )
	style().drawPrimitive( QStyle::PE_FocusRect, p, tr, cg );
}

static QWidget *real_page( QWidget *pg )
{
    QScrollView *sv = (QScrollView*)pg;
    if ( !sv )
	return 0;
    if ( !sv->viewport()->children() )
	return 0;
    return (QWidget*)sv->viewport()->children()->getFirst();
}

static QWidget *internal_page( QWidget *pg, const QWidget *tb )
{
    QWidget *pp;
    while ( pg && ( pp = pg->parentWidget() ) && pp != tb )
	pg = pp;
    return pg;
}

/*!
    \class QToolBox

    \brief The QToolBox class provides a column of tabbed widgets.

    A toolbox is a widget that displays a column of tabs one above the
    other, with the current page displayed below the current tab.
    Every tab has an index position within the column of tabs. A tab's
    page is a QWidget.

    Each tab has a pageLabel(), an optional icon, pageIconSet(), an
    optional pageToolTip(), and a \link page() widget\endlink. The
    tab's attributes can be changed with setPageLabel(),
    setPageIconSet() and setPageToolTip().

    Pages are added using addPage(), or inserted at particular
    positions using insertPage(). Pages are deleted with removePage().
    The total number of pages is given by count().

    The current page is returned by currentPage() and set with
    setCurrentPage(). If you prefer you can work in terms of indexes
    using currentIndex(), setCurrentPage(), pageIndex() and page().

    The currentChanged() signal is emitted when the current page
    is changed.

    \sa QTabWidget
*/

/*!
    \fn void QToolBox::currentChanged( QWidget *page )

    This signal is emitted when the current page is changed. The new
    current page is passed in \a page.
*/

/*!
    Constructs a toolbox called \a name with parent \a parent.
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

/*!
  \fn void QToolBox::addPage( QWidget *page, const QString &label )
    \overload

    Adds the widget \a page in a new tab at bottom of the toolbox. The
    new tab's label is set to \a label.
*/
/*!
  \fn void QToolBox::addPage( QWidget *page, const QIconSet &iconSet,const QString &label )
    Adds the widget \a page in a new tab at bottom of the toolbox. The
    new tab's label is set to \a label, and the \a iconSet is
    displayed to the left of the \a label.
*/
/*!
    \fn void QToolBox::insertPage( QWidget *page, const QString &label, int index )
    \overload

    Inserts the widget \a page in a new tab at position \a index. The
    new tab's label is set to \a label.
*/

/*!
    Inserts the widget \a page in a new tab at position \a index. The
    new tab's label is set to \a label, and the \a iconSet is
    displayed to the left of the \a label.
*/

void QToolBox::insertPage( QWidget *page, const QIconSet &iconSet,
			   const QString &label, int index )
{
    QToolBoxButton *button = new QToolBoxButton( this, label.latin1() );
    QToolBoxPrivate::Page *c = new QToolBoxPrivate::Page;
    c->button = button;
    c->label = label;
    c->iconSet = iconSet;
    bool needRelayout = FALSE;
    if ( index < 0 || index >= count() ) {
	d->pageList.append( c );
    } else {
	d->pageList.insert( index, c );
	needRelayout = TRUE;
    }

    button->setText( label );
    if ( !iconSet.isNull() )
	button->setIcon( iconSet );
    button->setFixedHeight( button->sizeHint().height() );
    connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );

    QScrollView *sv = new QScrollView( this );
    sv->hide();
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

    button->show();

    if ( count() == 1 )
	setCurrentPage( page );

    d->updateTabs( this );
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
next.QuadPart = next.QuadPart + oneMill.QuadPart

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
	for ( QToolBoxPrivate::Page *c = d->pageList.first(); c;
	      c = d->pageList.next() ) {
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

/*!
    \property QToolBox::count
    \brief The number of pages contained in the toolbox.
*/

int QToolBox::count() const
{
    return d->pageList.count();
}

void QToolBox::setCurrentIndex( int index )
{
    if ( index < 0 || index >= count() )
	return;
    setCurrentPage( page( index ) );
}

/*!
    \overload

    Sets the current page to be \a page.
*/

void QToolBox::setCurrentPage( QWidget *page )
{
    page = internal_page( page, this );
    if ( !page || d->currentPage == page )
	return;

    QToolBoxButton *tb = d->button( page );
    if( !tb )
	return;

    if ( d->lastButton )
	d->lastButton->setSelected( FALSE );
    tb->setSelected( TRUE );
    d->lastButton = tb;
    if ( d->currentPage )
	d->currentPage->hide();
    d->currentPage = page;
    d->updatePageBackgroundMode();
    d->currentPage->show();
    d->updateTabs( this );
    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
    emit currentChanged( page );
}

void QToolBox::relayout()
{
    delete d->layout;
    d->layout = new QVBoxLayout( this );
    for ( QToolBoxPrivate::Page *c = d->pageList.first(); c;
	  c = d->pageList.next() ) {
	d->layout->addWidget( c->button );
	d->layout->addWidget( d->pages.find( c->button ) );
    }
    QWidget *currPage = d->currentPage;
    d->currentPage = 0;
    setCurrentPage( currPage );
}

/*!
    Removes the widget \a page from the toolbox. Note that the \a page
    is \e not deleted.
*/

void QToolBox::removePage( QWidget *page )
{
    page = internal_page( page, this );
    if ( !page )
	return;

    QToolBoxButton *tb = d->button( page );
    if ( !tb )
	return;

    page->hide();
    tb->hide();
    d->layout->remove( page );
    d->layout->remove( tb );
    d->pages.remove( page );
    d->pageList.remove( d->page( page ) );

    if ( d->pageList.isEmpty() ) {
	d->lastButton = 0;
	d->currentPage = 0;
    } else {
	setCurrentPage( d->pages.find( d->pageList.first()->button ) );
    }
}

/*!
    Returns the toolbox's current page.
*/

QWidget *QToolBox::currentPage() const
{
    return real_page( d->currentPage );
}

/*!
    \property QToolBox::currentPage
    \brief the current page's index position
*/

int QToolBox::currentIndex() const
{
    return indexOf( d->currentPage );
}

/*!
    Returns the page at position \a index.
*/

QWidget *QToolBox::page( int index ) const
{
    QToolBoxPrivate::Page *p = d->pageList.at( index );
    if ( !p )
	return 0;
    QWidget *pg = d->pages.find( p->button );
    if ( !pg )
	return pg;
    return real_page( pg );
}

/*!
    Returns the index position of page \a page.
*/

int QToolBox::indexOf( QWidget *page ) const
{
    page = internal_page( page, this );
    if ( !page )
	return -1;
    QToolBoxButton *tb = d->button( page );
    int i = 0;
    for ( QToolBoxPrivate::Page *c = d->pageList.first(); c;
	  c = d->pageList.next(), ++i ) {
	if ( c->button == tb )
	    return i;
    }
    return -1;
}

/*!
    If \a enabled is TRUE then page \a page is enabled; otherwise page
    \a page is disabled.
*/

void QToolBox::setPageEnabled( QWidget *page, bool enabled )
{
    page = internal_page( page, this );
    if ( !page )
	return;
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

/*!
    Sets the tab label of page \a page to \a label.
*/

void QToolBox::setPageLabel( QWidget *page, const QString &label )
{
    page = internal_page( page, this );
    if ( !page )
	return;
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->label = label;
    c->button->setText( label );
}

/*!
    Sets the tab icon of page \a page to \a iconSet.
*/

void QToolBox::setPageIconSet( QWidget *page, const QIconSet &iconSet )
{
    page = internal_page( page, this );
    if ( !page )
	return;
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->iconSet = iconSet;
    ( (QToolBoxButton*)c->button )->setIcon( iconSet );
}

/*!
    Sets the tab tooltip of page \a page to \a toolTip.
*/

void QToolBox::setPageToolTip( QWidget *page, const QString &toolTip )
{
    page = internal_page( page, this );
    if ( !page )
	return;
    QToolBoxButton *tb = d->button( page );
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !tb || !c )
	return;

    c->toolTip = toolTip;
    QToolTip::remove( tb );
    QToolTip::add( tb, toolTip );
}

/*!
    Returns TRUE if page \a page is enabled; otherwise returns FALSE.
*/

bool QToolBox::isPageEnabled( QWidget *page ) const
{
    page = internal_page( page, this );
    if ( !page )
	return FALSE;
    QToolBoxButton *tb = d->button( page );
    return tb && tb->isEnabled();
}

/*!
    Returns page \a page's tab label.
*/

QString QToolBox::pageLabel( QWidget *page ) const
{
    page = internal_page( page, this );
    if ( !page )
	return QString::null;
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->label : QString::null;
}

/*!
    Returns page \a page's tab icon.
*/

QIconSet QToolBox::pageIconSet( QWidget *page ) const
{
    page = internal_page( page, this );
    if ( !page )
	return QIconSet();
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->iconSet : QIconSet();
}

/*!
    Returns page \a page's tooltip.
*/

QString QToolBox::pageToolTip( QWidget *page ) const
{
    page = internal_page( page, this );
    if ( !page )
	return QString::null;
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? c->toolTip : QString::null;
}

/*!
  \property QToolBox::pageBackgroundMode
  \brief The background mode in which the current page should be displayed.
*/

void QToolBox::setPageBackgroundMode( BackgroundMode bm )
{
    if ( d->pageBackgroundMode == bm )
	return;
    d->pageBackgroundMode = bm;
    d->updatePageBackgroundMode();
    d->updateTabs( this );
}

Qt::BackgroundMode QToolBox::pageBackgroundMode() const
{
    return d->pageBackgroundMode;
}

void QToolBox::showEvent( QShowEvent *e )
{
    d->updatePageBackgroundMode();
    QWidget::showEvent( e );
}

#include "qtoolbox.moc"
