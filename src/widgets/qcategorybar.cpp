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

class QCategoryBarPrivate
{
public:
    QCategoryBarPrivate()
	{
	    currentPage = 0;
	    lastTab = 0;
	    buttons = new QWidgetList;
	}

    ~QCategoryBarPrivate()
	{
	    delete buttons;
	}

    QPtrDict<QWidget> pages;
    QWidgetList *buttons;
    QVBoxLayout *layout;
    QWidget *currentPage;
    QCategoryButton *lastTab;
};

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

void QCategoryButton::drawButton( QPainter *p )
{
    if ( selected ) {
	QFont f( p->font() );
	f.setBold( TRUE );
	p->setFont( f );
    }

    int d = 20 + height() - 3;

    QPointArray a( 7 );
    a.setPoint( 0, -1, height() + 1 );
    a.setPoint( 1, -1, 1 );
    a.setPoint( 2, width() - d, 1 );
    a.setPoint( 3, width() - 20, height() - 2 );
    a.setPoint( 4, width() - 1, height() - 2 );
    a.setPoint( 5, width() - 1, height() + 1 );
    a.setPoint( 6, -1, height() + 1 );


    if ( selected )
	p->setBrush( colorGroup().light() );
    else
	p->setBrush( colorGroup().brush( QColorGroup::Background ) );

    p->setPen( colorGroup().mid().dark( 150 ) );
    p->drawPolygon( a );
    p->setPen( colorGroup().light() );
    p->drawLine( 0, 2, width() - d, 2 );
    p->drawLine( width() - d - 1, 2, width() - 21, height() - 1 );
    p->drawLine( width() - 20, height() - 1, width(), height() - 1 );
    p->setBrush( NoBrush );

    p->setPen( colorGroup().buttonText() );
    if ( p->fontMetrics().width( text() ) < width() - d - 5 ) {
	p->drawText( 2, 2, width(), height() - 2, AlignVCenter | AlignLeft, text() );
    } else {
	QString s = text().left( 1 );
	int ew = p->fontMetrics().width( "..." );
	int i = 1;
	while ( p->fontMetrics().width( s ) + ew + p->fontMetrics().width( text()[i] )  < width() - d - 5 )
	    s += text()[i++];
	s += "...";
	p->drawText( 2, 2, width(), height() - 2, AlignVCenter | AlignLeft, s );
    }
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

static void set_background_mode( QWidget *top, Qt::BackgroundMode bm )
{
    QObjectList *l = top->queryList( "QWidget" );
    l->append( top );
    for ( QObject *o = l->first(); o; o = l->next() )
	( (QWidget*)o )->setBackgroundMode( bm );
    delete l;
}

void QCategoryBar::addCategory( const QString &name, QWidget *page )
{
    page->setBackgroundMode( PaletteBackground );
    QCategoryButton *button = new QCategoryButton( this, name.latin1() );
    d->buttons->append( button );
    button->setText( name );
    button->setFixedHeight( button->sizeHint().height() );
    connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
    QScrollView *sv = new QScrollView( this );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    sv->addChild( page );
    sv->setFrameStyle( QFrame::NoFrame );
    page->show();
    d->pages.insert( button, sv );
    d->layout->addWidget( button );
    d->layout->addWidget( sv );
    button->show();
    if ( d->pages.count() == 1 ) {
	d->currentPage = sv;
	d->lastTab = button;
	d->lastTab->setSelected( TRUE );
	sv->show();
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
    if ( !page || d->currentPage == page )
	return;
    tb->setSelected( TRUE );
    if ( d->lastTab )
	d->lastTab->setSelected( FALSE );
    d->lastTab = tb;
    if ( d->currentPage )
	d->currentPage->hide();
    d->currentPage = page;
    d->currentPage->show();
    set_background_mode( d->currentPage, PaletteLight );
    updateTabs();
}

void QCategoryBar::updateTabs()
{
    bool after = FALSE;
    for ( QWidget *w = d->buttons->first(); w; w = d->buttons->next() ) {
	w->setBackgroundMode( !after ? PaletteBackground : PaletteLight );
	w->update();
	after = w == d->lastTab;
    }
}
