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

#ifndef QT_NO_TOOLBOX

#include <qbutton.h>
#include <qlayout.h>
#include <qscrollview.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qvaluelist.h>
#include <qtooltip.h>
#include <qeventloop.h>
#include <qdatetime.h>

class QToolBoxButton : public QButton
{
public:
    QToolBoxButton( QWidget *parent, const char *name )
	: QButton( parent, name ), selected( FALSE )
    {
	setBackgroundMode(PaletteBackground);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	setFocusPolicy(NoFocus);
    }

    inline void setSelected( bool b ) { selected = b; update(); }
    inline void setTextLabel( const QString &text ) { label = text; update(); }
    inline QString textLabel() const { return label; }
    inline void setIconSet( const QIconSet &is ) { icon = is; update(); }
    inline QIconSet iconSet() const { return icon; }

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void drawButton( QPainter * );
    void drawButtonLabel( QPainter * );

private:
    bool selected;
    QString label;
    QIconSet icon;
};

void QToolBoxButton::drawButtonLabel( QPainter * )
{
}

class QToolBoxPrivate
{
public:
    struct Page
    {
	QToolBoxButton *button;
	QScrollView *sv;
	QWidget *widget;
	QString toolTip;

	inline void setTextLabel( const QString &text ) { button->setTextLabel(text); }
	inline void setIconSet( const QIconSet &is ) { button->setIconSet(is); }
	inline void setToolTip( const QString &tip )
	{
	    toolTip = tip;
	    QToolTip::remove( button );
	    if ( !tip.isNull() )
		QToolTip::add( button, tip );
	}

	inline bool operator==(const Page& other) const
	{
	    return widget == other.widget;
	}
    };
    typedef QValueList<Page> PageList;

    inline QToolBoxPrivate()
	: currentPage( 0 ), pageBackgroundMode( Qt::NoBackground )
    {
    }

    Page *page( QWidget *widget );

    void updatePageBackgroundMode();
    void updateTabs( QToolBox *tb );

    PageList pageList;
    QVBoxLayout *layout;
    Page *currentPage;
    Qt::BackgroundMode pageBackgroundMode;
};

QToolBoxPrivate::Page *QToolBoxPrivate::page( QWidget *widget )
{
    if ( !widget )
	return 0;

    for ( PageList::ConstIterator i = pageList.constBegin(); i != pageList.constEnd(); ++i )
	if ( (*i).widget == widget )
	    return (Page*) &(*i);
    return 0;
}

void QToolBoxPrivate::updatePageBackgroundMode()
{
    if ( pageBackgroundMode == Qt::NoBackground || !currentPage )
	return;
    QObjectList l = currentPage->sv->viewport()->queryList( "QWidget" );
    for (int i = 0; i < l.size(); ++i) {
	QWidget *w = (QWidget*)l.at(i);
	if ( w->backgroundMode() == pageBackgroundMode )
	    continue;
	w->setBackgroundMode( pageBackgroundMode );
	w->update();
    }
}

void QToolBoxPrivate::updateTabs( QToolBox *tb )
{
    QToolBoxButton *lastButton = currentPage ? currentPage->button : 0;
    bool after = FALSE;
    for ( PageList::ConstIterator i = pageList.constBegin(); i != pageList.constEnd(); ++i ) {
	Qt::BackgroundMode bm = ( after ? tb->backgroundMode() : Qt::PaletteBackground  );
	if ( (*i).button->backgroundMode() != bm ) {
	    (*i).button->setBackgroundMode( bm );
	    (*i).button->update();
	}
	after = (*i).button == lastButton;
    }
}

QSize QToolBoxButton::sizeHint() const
{
    QSize iconSize(8, 8);
    if ( !icon.isNull() )
	iconSize += icon.pixmap( QIconSet::Small, QIconSet::Normal ).size() + QSize( 2, 0 );
    QSize textSize = fontMetrics().size( Qt::ShowPrefix, label ) + QSize(0, 8);

    QSize total(iconSize.width() + textSize.width(), QMAX(iconSize.height(), textSize.height()));
    return total.expandedTo(QApplication::globalStrut());
}

QSize QToolBoxButton::minimumSizeHint() const
{
    if ( icon.isNull() )
	return QSize();
    return QSize(8, 8) + icon.pixmap( QIconSet::Small, QIconSet::Normal ).size();
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
    if (isDown())
	flags |= QStyle::Style_Down;
    style().drawControl( QStyle::CE_ToolBoxTab, p, parentWidget(), rect(), cg, flags );

    QPixmap pm = icon.pixmap( QIconSet::Small, isEnabled() ? QIconSet::Normal : QIconSet::Disabled );

    QRect cr = style().subRect( QStyle::SR_ToolBoxTabContents, this );
    QRect tr, ir;
    int ih = 0;
    if ( pm.isNull() ) {
	tr = cr;
	tr.addCoords( 4, 0, -8, 0 );
    } else {
	int iw = pm.width() + 4;
	ih = pm.height();
	ir = QRect( cr.left() + 4, cr.top(), iw + 2, ih );
	tr = QRect( ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height() );
    }

    if ( selected && style().styleHint( QStyle::SH_ToolBox_SelectedPageTitleBold ) ) {
	QFont f( p->font() );
	f.setBold( TRUE );
	p->setFont( f );
    }

    QString txt;
    if ( p->fontMetrics().width(label) < tr.width() ) {
	txt = label;
    } else {
	txt = label.left( 1 );
	int ew = p->fontMetrics().width( "..." );
	int i = 1;
	while ( p->fontMetrics().width( txt ) + ew +
		p->fontMetrics().width( label[i] )  < tr.width() )
	    txt += label[i++];
	txt += "...";
    }

    if ( ih )
	p->drawPixmap( ir.left(), (height() - ih) / 2, pm );

    QToolBox *tb = (QToolBox*)parentWidget();

    const QColor* fill = 0;
    if ( selected &&
	 style().styleHint( QStyle::SH_ToolBox_SelectedPageTitleBold ) &&
	 tb->backgroundMode() != NoBackground )
	fill = &cg.color( QPalette::foregroundRoleFromMode( tb->backgroundMode() ) );

    style().drawItem( p, tr, AlignLeft | AlignVCenter | ShowPrefix, cg,
		      isEnabled(), 0, txt, -1, fill );

    if ( !txt.isEmpty() && hasFocus() )
	style().drawPrimitive( QStyle::PE_FocusRect, p, tr, cg );
}

/*!
    \class QToolBox
    \brief The QToolBox class provides a column of tabbed widgets.
    \mainclass
    \ingroup advanced

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
    using currentIndex(), setCurrentIndex(), indexOf() and page().

    The currentChanged() signal is emitted when the current page
    is changed.

    \sa QTabWidget
*/

/*!
    \fn void QToolBox::currentChanged( int index )

    This signal is emitted when the current page is changed. The index
    position of the new current page is passed in \a index.
*/

/*!
    Constructs a toolbox called \a name with parent \a parent and flags \a f.
*/

QToolBox::QToolBox( QWidget *parent, const char *name, WFlags f )
    :  QFrame( parent, name, f )
{
    d = new QToolBoxPrivate;
    d->layout = new QVBoxLayout( this );
    QWidget::setBackgroundMode( PaletteButton );
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
    if ( !page )
	return;

    page->setBackgroundMode( PaletteButton );
    QToolBoxPrivate::Page c;
    c.widget = page;
    c.button = new QToolBoxButton( this, label.latin1() );
    connect( c.button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );

    c.sv = new QScrollView( this );
    c.sv->hide();
    c.sv->setResizePolicy( QScrollView::AutoOneFit );
    c.sv->addChild( page );
    c.sv->setFrameStyle( QFrame::NoFrame );

    c.setTextLabel( label );
    c.setIconSet( iconSet );

    if ( index < 0 || index >= count() ) {
	d->pageList.append( c );
	d->layout->addWidget( c.button );
	d->layout->addWidget( c.sv );
    } else {
	d->pageList.insert( d->pageList.at(index), c );
	relayout();
    }

    c.button->show();

    if ( count() == 1 )
	setCurrentPage( page );

    d->updateTabs( this );
}

void QToolBox::buttonClicked()
{
    QToolBoxButton *tb = ::qt_cast<QToolBoxButton*>(sender());
    QWidget *widget = 0;
    for ( QToolBoxPrivate::PageList::ConstIterator i = d->pageList.constBegin(); i != d->pageList.constEnd(); ++i )
	if ( (*i).button == tb ) {
	    widget = (*i).widget;
	    break;
	}

    if ( widget == d->currentPage->widget )
	return;

    setCurrentPage( widget );
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
    Sets the current page to be \a page.
*/

void QToolBox::setCurrentPage( QWidget *page )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !c || d->currentPage == c )
	return;

    c->button->setSelected( TRUE );
    if ( d->currentPage ) {
	d->currentPage->sv->hide();
	d->currentPage->button->setSelected(FALSE);
    }
    d->currentPage = c;
    d->updatePageBackgroundMode();
    d->currentPage->sv->show();
    d->updateTabs( this );
    emit currentChanged( indexOf(page) );
}

void QToolBox::relayout()
{
    delete d->layout;
    d->layout = new QVBoxLayout( this );
    for ( QToolBoxPrivate::PageList::ConstIterator i = d->pageList.constBegin(); i != d->pageList.constEnd(); ++i ) {
	d->layout->addWidget( (*i).button );
	d->layout->addWidget( (*i).sv );
    }
    QWidget *currPage = d->currentPage ? d->currentPage->widget : 0;
    d->currentPage = 0;
    if ( currPage )
	setCurrentPage( currPage );
}

/*!
    Removes the widget \a page from the toolbox. Note that the \a page
    is \e not deleted.
*/

void QToolBox::removePage( QWidget *page )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !c )
	return;

    d->layout->remove( c->sv );
    d->layout->remove( c->button );
    page->reparent( this, QPoint(0,0) );
    page->hide();
    delete c->sv;
    delete c->button;

    d->pageList.remove( *c );

    if ( d->pageList.isEmpty() ) {
	d->currentPage = 0;
    } else if ( c == d->currentPage ) {
	d->currentPage = 0;
	setCurrentIndex( 0 );
    }
}

/*!
    Returns the toolbox's current page, or 0 if the toolbox is empty.
*/

QWidget *QToolBox::currentPage() const
{
    return d->currentPage ? d->currentPage->widget : 0;
}

/*!
    \property QToolBox::currentIndex
    \brief the current page's index position
*/

int QToolBox::currentIndex() const
{
    return d->currentPage ? indexOf( d->currentPage->widget ) : -1;
}

/*!
    Returns the page at position \a index, or 0 if there is no such page.
*/

QWidget *QToolBox::page( int index ) const
{
    if ( index < 0 || index >= (int) d->pageList.size() )
	return 0;
    return (*d->pageList.at( index )).widget;
}

/*!
    Returns the index position of page \a page, or -1 if \a page is not
    in this toolbox.
*/

int QToolBox::indexOf( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return c ? d->pageList.findIndex( *c ) : -1;
}

/*!
    If \a enabled is TRUE then page \a page is enabled; otherwise page
    \a page is disabled.
*/

void QToolBox::setPageEnabled( QWidget *page, bool enabled )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( !c )
	return;

    c->button->setEnabled( enabled );
    if ( !enabled )
	activateClosestPage( page );
}

void QToolBox::activateClosestPage( QWidget *widget )
{
    QToolBoxPrivate::Page *c = d->page( widget );
    if ( !c || c != d->currentPage )
	return;

    QWidget *p = 0;
    int curIndexUp = d->pageList.findIndex( *c );
    int curIndexDown = curIndexUp;
    while ( !p ) {
	if ( curIndexDown < (int)d->pageList.count()-1 ) {
	    c = d->page(page(++curIndexDown));
	    if ( c->button->isEnabled() ) {
		p = c->widget;
		break;
	    }
	} else if ( curIndexUp > 1 ) {
	    c = d->page(page(--curIndexUp));
	    if ( c->button->isEnabled() ) {
		p = c->widget;
		break;
	    }
	} else {
	    break;
	}
    }
    if ( p )
	setCurrentPage( p );
}

/*!
    Sets the tab label of page \a page to \a label.
*/

void QToolBox::setPageLabel( QWidget *page, const QString &label )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( c )
	c->setTextLabel( label );
}

/*!
    Sets the tab icon of page \a page to \a iconSet.
*/

void QToolBox::setPageIconSet( QWidget *page, const QIconSet &iconSet )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( c )
	c->setIconSet( iconSet );
}

/*!
    Sets the tab tooltip of page \a page to \a toolTip.
*/

void QToolBox::setPageToolTip( QWidget *page, const QString &toolTip )
{
    QToolBoxPrivate::Page *c = d->page( page );
    if ( c )
	c->setToolTip( toolTip );
}

/*!
    Returns TRUE if page \a page is enabled; otherwise returns FALSE.
*/

bool QToolBox::isPageEnabled( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return c && c->button->isEnabled();
}

/*!
    Returns page \a page's tab label, or the null string if
    \a page is not in this toolbox.
*/

QString QToolBox::pageLabel( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return ( c ? c->button->textLabel() : QString::null );
}

/*!
    Returns page \a page's tab icon, or the null icon if
    \a page is not in this toolbox.
*/

QIconSet QToolBox::pageIconSet( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return ( c ? c->button->iconSet() : QIconSet() );
}

/*!
    Returns page \a page's tooltip, or the null string if
    \a page is not in this toolbox.
*/

QString QToolBox::pageToolTip( QWidget *page ) const
{
    QToolBoxPrivate::Page *c = d->page( page );
    return ( c ? c->toolTip : QString::null );
}

/*!
  \property QToolBox::pageBackgroundMode
  \brief The background mode in which the current page should be displayed.
  If this pageBackgroundMode is set to something else than
  NoBackground, the background mode is also set to all children of the
  pages.
*/

void QToolBox::setPageBackgroundMode( BackgroundMode bm )
{
    if ( d->pageBackgroundMode == bm )
	return;
    d->pageBackgroundMode = (bm == NoBackground ? backgroundMode() : bm);
    setBackgroundMode( d->pageBackgroundMode );
    d->updatePageBackgroundMode();
    d->updateTabs( this );
    d->pageBackgroundMode = bm;
}

Qt::BackgroundMode QToolBox::pageBackgroundMode() const
{
    return d->pageBackgroundMode;
}

/*! \reimp */
void QToolBox::showEvent( QShowEvent *e )
{
    d->updatePageBackgroundMode();
    QWidget::showEvent( e );
}

/*! \reimp */
void QToolBox::frameChanged()
{
    d->layout->setMargin( frameWidth() );
    QFrame::frameChanged();
}

#endif //QT_NO_TOOLBOX
