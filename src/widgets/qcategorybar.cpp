/****************************************************************************
** $Id$
**
** Implementation of QCategoryBar widget class
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

#include "qcategorybar.h"
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

class QCategoryButton : public QToolButton
{
public:
    QCategoryButton( QWidget *parent, const char *name ) :
	QToolButton( parent, name ), selected( FALSE )
	{ setBackgroundMode( PaletteLight ); }

    void setSelected( bool b ) { selected = b; update(); }

protected:
    void drawButton( QPainter * );

private:
    bool selected;

};

class QCategoryBarPrivate
{
public:
    struct Category
    {
	QToolButton *button;
	QString label;
	QIconSet iconSet;
	QString toolTip;
    };

    QCategoryBarPrivate()
	{
	    currentPage = 0;
	    lastButton = 0;
	    categories = new QPtrList<Category>;
	    categories->setAutoDelete( TRUE );
	    scrollEffect = TRUE;
	}

    ~QCategoryBarPrivate()
	{
	    delete categories;
	}

    QCategoryButton *button( QWidget *page )
	{
	    QPtrDictIterator<QWidget> it( pages );
	    while ( it.current() ) {
		if ( it.current() == page )
		    return (QCategoryButton*)it.currentKey();
		++it;
	    }
	    return 0;
	}

    Category *category( QWidget *page )
	{
	    QCategoryButton *b = button( page );
	    for ( QCategoryBarPrivate::Category *c = categories->first(); c;
		  c = categories->next() ) {
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
    QPtrList<Category> *categories;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QCategoryButton *lastButton;
    bool scrollEffect;
};



void QCategoryButton::drawButton( QPainter *p )
{
    QStyle::SFlags flags = QStyle::Style_Default;

    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( selected )
	flags |= QStyle::Style_Selected;
    if ( hasFocus() )
	flags |= QStyle::Style_HasFocus;
    style().drawControl( QStyle::CE_CategoryBarTab, p, this, rect(),
			 colorGroup(), flags );

    int d = 20 + height() - 3;
    QRect tr, ir;
    if ( iconSet().isNull() ) {
	tr = QRect( 2, 2, width() - d - 5, height() );
    } else {
	int iw = iconSet().pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	ir = QRect( 0, 0, iw + 2, height() );
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

    style().drawItem( p, tr, AlignLeft | AlignVCenter | ShowPrefix, colorGroup(),
		      isEnabled(), 0, txt );

    if ( !txt.isEmpty() && hasFocus() )
	style().drawPrimitive( QStyle::PE_FocusRect, p, tr, colorGroup() );
}


QCategoryBar::QCategoryBar( QWidget *parent, const char *name )
    :  QWidget( parent, name )
{
    d = new QCategoryBarPrivate;
    d->layout = new QVBoxLayout( this );
}

QCategoryBar::~QCategoryBar()
{
    delete d;
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

void QCategoryBar::addCategory( const QString &label, QWidget *page )
{
    addCategory( label, QIconSet(), page );
}

void QCategoryBar::addCategory( const QString &label, const QIconSet &iconSet,
				QWidget *page )
{
    insertCategory( label, iconSet, page );
}

void QCategoryBar::insertCategory( const QString &label, QWidget *page, int index )
{
    insertCategory( label, QIconSet(), page, index );
}

void QCategoryBar::insertCategory( const QString &label, const QIconSet &iconSet,
				   QWidget *page, int index )
{
    page->setBackgroundMode( PaletteBackground );

    QCategoryButton *button = new QCategoryButton( this, label.latin1() );
    QCategoryBarPrivate::Category *c = new QCategoryBarPrivate::Category;
    c->button = button;
    c->label = label;
    c->iconSet = iconSet;
    bool needRelayout = FALSE;
    if ( index < 0 || index >= count() ) {
	d->categories->append( c );
    } else {
	d->categories->insert( index, c );
	needRelayout = TRUE;
    }

    button->setText( label );
    if ( !iconSet.isNull() )
	button->setIconSet( iconSet );
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

void QCategoryBar::buttonClicked()
{
    QCategoryButton *tb = (QCategoryButton*)sender();
    QWidget *page = d->pages.find( tb );

    if ( page == d->currentPage )
	return;


    if ( d->scrollEffect ) {
	// ### This implementation can be improved by resizing the old
	// ### and new current page accordingly. This will improve the
	// ### visual effect
	int direction = 0;

	QWidgetList buttons;
	for ( QCategoryBarPrivate::Category *c = d->categories->first(); c;
	      c = d->categories->next() ) {
	    if ( c->button == tb ) {
		buttons.append( c->button );
		if ( direction < 0 )
		    break;
		direction = 8;
	    } else if ( c->button == d->lastButton ) {
		if ( direction > 0 )
		    break;
		direction = -8;
	    } else if ( direction != 0 ) {
		buttons.append( c->button );
	    }
	}

	int dist = 0;
	int h = d->currentPage->parentWidget()->height() - d->lastButton->height();

	QTime t;
	t.start();

	while ( dist < h ) {
	    if ( t.elapsed() > 1 ) {
		QWidgetListIt it( buttons );
		while ( it.current() ) {
		    it.current()->raise();
		    it.current()->move( it.current()->x(),
					it.current()->y() + direction );
		    ++it;
		}
		dist += QABS( direction );
		t.restart();
	    }
	    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
	}
    }

    setCurrentPage( page );
}

void QCategoryBar::updateTabs()
{
    bool after = FALSE;
    for ( QCategoryBarPrivate::Category *c = d->categories->first(); c;
	  c = d->categories->next() ) {
	c->button->setBackgroundMode( !after ? PaletteBackground : PaletteLight );
	c->button->update();
	after = c->button == d->lastButton;
    }
}

int QCategoryBar::count() const
{
    return d->categories->count();
}

void QCategoryBar::setCurrentPage( int index )
{
    setCurrentPage( page( index ) );
}

void QCategoryBar::setCurrentPage( QWidget *page )
{
    if ( !page || d->currentPage == page )
	return;

    QCategoryButton *tb = d->button( page );
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
    emit currentChanged( page );
}

void QCategoryBar::relayout()
{
    delete d->layout;
    d->layout = new QVBoxLayout( this );
    for ( QCategoryBarPrivate::Category *c = d->categories->first(); c;
	  c = d->categories->next() ) {
	d->layout->addWidget( c->button );
	d->layout->addWidget( d->pages.find( c->button ) );
    }
    QWidget *currPage = d->currentPage;
    d->currentPage = 0;
    setCurrentPage( currPage );
}

void QCategoryBar::removeCategory( QWidget *page )
{
    if ( !page )
	return;

    QCategoryButton *tb = d->button( page );
    if ( !tb )
	return;

    activateClosestPage( page );

    page->hide();
    tb->hide();
    d->layout->remove( page );
    d->layout->remove( tb );
}

QWidget *QCategoryBar::currentPage() const
{
    return d->currentPage;
}

int QCategoryBar::currentIndex() const
{
    return pageIndex( d->currentPage );
}

QWidget *QCategoryBar::page( int index ) const
{
    return d->pages.find( d->categories->at( index )->button );
}

int QCategoryBar::pageIndex( QWidget *page ) const
{
    QCategoryButton *tb = d->button( page );
    int i = 0;
    for ( QCategoryBarPrivate::Category *c = d->categories->first(); c;
	  c = d->categories->next(), ++i ) {
	if ( c->button == tb )
	    return i;
    }
    return -1;
}

void QCategoryBar::setCategoryEnabled( QWidget *page, bool enabled )
{
    QCategoryButton *tb = d->button( page );
    if ( !tb )
	return;

    if ( !enabled )
	activateClosestPage( page );

    tb->setEnabled( enabled );
}

void QCategoryBar::activateClosestPage( QWidget *page )
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

void QCategoryBar::setCategoryLabel( QWidget *page, const QString &label )
{
    QCategoryButton *tb = d->button( page );
    QCategoryBarPrivate::Category *c = d->category( page );
    if ( !tb || !c )
	return;

    c->label = label;
    c->button->setText( label );
}

void QCategoryBar::setCategoryIconSet( QWidget *page, const QIconSet &iconSet )
{
    QCategoryButton *tb = d->button( page );
    QCategoryBarPrivate::Category *c = d->category( page );
    if ( !tb || !c )
	return;

    c->iconSet = iconSet;
    c->button->setIconSet( iconSet );
}

void QCategoryBar::setCategoryToolTip( QWidget *page, const QString &toolTip )
{
    QCategoryButton *tb = d->button( page );
    QCategoryBarPrivate::Category *c = d->category( page );
    if ( !tb || !c )
	return;

    c->toolTip = toolTip;
    QToolTip::remove( tb );
    QToolTip::add( tb, toolTip );
}

bool QCategoryBar::isCategoryEnabled( QWidget *page ) const
{
    QCategoryButton *tb = d->button( page );
    return tb && tb->isEnabled();
}

QString QCategoryBar::categoryLabel( QWidget *page ) const
{
    QCategoryBarPrivate::Category *c = d->category( page );
    return c ? c->label : QString::null;
}

QIconSet QCategoryBar::categoryIconSet( QWidget *page ) const
{
    QCategoryBarPrivate::Category *c = d->category( page );
    return c ? c->iconSet : QIconSet();
}

QString QCategoryBar::categoryToolTip( QWidget *page ) const
{
    QCategoryBarPrivate::Category *c = d->category( page );
    return c ? c->toolTip : QString::null;
}

bool QCategoryBar::isScrollEffectEnabled() const
{
    return d->scrollEffect;
}

void QCategoryBar::setScrollEffectEnabled( bool enable )
{
    d->scrollEffect = enable;
}

