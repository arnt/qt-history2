/****************************************************************************
** $Id: //depot/qt/main/examples/qiconview/qiconview.cpp#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qiconview.h"

#include <qpixmap.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qevent.h>
#include <qpalette.h>
#include <qmime.h>
#include <qimage.h>
#include <qpen.h>
#include <qbrush.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qkeycode.h>

static const char *unknown[] = {
    "32 32 11 1",
    "c c #ffffff",
    "g c #c0c0c0",
    "a c #c0ffc0",
    "h c #a0a0a4",
    "d c #585858",
    "f c #303030",
    "i c #400000",
    "b c #00c000",
    "e c #000000",
    "# c #000000",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebaaaaaaa#.........",
    ".#cccccgcgghhebbbbaaaaa#........",
    ".#ccccccgcgggdebbbbbbaa#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "....##ddhhhghhhhhhhhhhd#b#......",
    "......##ddhhhhhhhhhhhhd#b#......",
    "........##ddhhhhhhhhhhd#b#......",
    "..........##ddhhhhhhhhd#b#......",
    "............##ddhhhhhhd#b###....",
    "..............##ddhhhhd#b#####..",
    "................##ddhhd#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};

/*****************************************************************************
 *
 * Struct QtIconViewPrivate
 *
 *****************************************************************************/

struct QtIconViewPrivate
{
    QtIconViewItem *firstItem, *lastItem;
    unsigned int count;
    QIconSet::Size mode;
    bool mousePressed, startDrag;
    QtIconView::SelectionMode selectionMode;
    QtIconViewItem *currentItem, *tmpCurrentItem;
    QRect *rubber;
    QTimer *scrollTimer;
    int rastX, rastY, spacing;
    bool cleared;
};

/*****************************************************************************
 *
 * Class QtIconViewItemLineEdit
 *
 *****************************************************************************/

QtIconViewItemLineEdit::QtIconViewItemLineEdit( const QString &text, QWidget *parent, QtIconViewItem *theItem, const char *name )
    : QMultiLineEdit( parent, name ), item( theItem ), startText( text )
{
    setText( text );
    clearTableFlags();
}

void QtIconViewItemLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key()  == Key_Escape ) {
        item->QtIconViewItem::setText( startText );
        item->iconView()->repaintContents( item->iconView()->contentsX(), item->iconView()->contentsY(),
                                           item->iconView()->contentsWidth(), item->iconView()->contentsHeight() );
        emit escapePressed();
    }
    else if ( e->key() == Key_Enter ||
              e->key() == Key_Return )
        emit returnPressed();
    else
        QMultiLineEdit::keyPressEvent( e ) ;
}

/*****************************************************************************
 *
 * Class QtIconViewItem
 *
 *****************************************************************************/

QtIconViewItem::QtIconViewItem( QtIconView *parent )
    : view( parent ), itemText(), itemIcon( QPixmap( unknown ) ),
      prev( 0L ), next( 0L ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::QtIconViewItem( QtIconView *parent, QtIconViewItem *after )
    : view( parent ), itemText(), itemIcon( QPixmap( unknown ) ),
      prev( 0L ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::QtIconViewItem( QtIconView *parent, const QString &text )
    : view( parent ), itemText( text ), itemIcon( QPixmap( unknown ) ),
      prev( 0L ), next( 0L ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::QtIconViewItem( QtIconView *parent, QtIconViewItem *after, const QString &text )
    : view( parent ), itemText( text ), itemIcon( QPixmap( unknown ) ),
      prev( 0L ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::QtIconViewItem( QtIconView *parent, const QString &text, const QIconSet &icon )
    : view( parent ), itemText( text ), itemIcon( icon ),
      prev( 0L ), next( 0L ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::QtIconViewItem( QtIconView *parent, QtIconViewItem *after, const QString &text, const QIconSet &icon )
    : view( parent ), itemText( text ), itemIcon( icon ),
      prev( 0L ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0L ), renameBox( 0L )
{
    if ( view ) {
        fm = new QFontMetrics( view->font() );
        viewMode = view->viewMode();

        makeActiveIcon();
        calcRect();

        view->insertItem( this );
    }
}

QtIconViewItem::~QtIconViewItem()
{
    view->removeItem( this );
    removeRenameBox();
}

void QtIconViewItem::setText( const QString &text )
{
    if ( text == itemText )
        return;

    itemText = text;
    calcRect();
    repaint();
}

void QtIconViewItem::setIcon( const QIconSet &icon )
{
    itemIcon = icon;
    calcRect();
    repaint();
}

void QtIconViewItem::setAllowRename( bool allow )
{
    allow_rename = allow;
}

void QtIconViewItem::setAllowDrag( bool allow )
{
    allow_drag = allow;
}

void QtIconViewItem::setAllowDrop( bool allow )
{
    allow_drop = allow;
}

QString QtIconViewItem::text()
{
    return itemText;
}

QIconSet QtIconViewItem::icon()
{
    return itemIcon;
}

bool QtIconViewItem::allowRename()
{
    return allow_rename;
}

bool QtIconViewItem::allowDrag()
{
    return allow_drag;
}

bool QtIconViewItem::allowDrop()
{
    return allow_drop;
}

QtIconView *QtIconViewItem::iconView() const
{
    return view;
}

QtIconViewItem *QtIconViewItem::prevItem() const
{
    return prev;
}

QtIconViewItem *QtIconViewItem::nextItem() const
{
    return next;
}

int QtIconViewItem::index()
{
    if ( view )
        return view->index( this );

    return -1;
}

void QtIconViewItem::setSelected( bool s, bool cb )
{
    if ( selectable && s != selected ) {
        if ( !s )
            selected = FALSE;
        else {
            if ( view->d->selectionMode == QtIconView::Single && view->d->currentItem )
                view->d->currentItem->setSelected( FALSE );
            else if ( view->d->selectionMode == QtIconView::StrictMulti && !cb )
                view->selectAll( FALSE );
        }
        selected = s;

        repaint();
    }
}

void QtIconViewItem::setSelectable( bool s )
{
    selectable = s;
    if ( selected )
        selected = FALSE;
}

bool QtIconViewItem::isSelected()
{
    return selected;
}

bool QtIconViewItem::isSelectable()
{
    return selectable;
}

void QtIconViewItem::repaint()
{
    if ( view )
        view->repaintItem( this );
}

void QtIconViewItem::move( int x, int y )
{
    itemRect.setRect( x, y, itemRect.width(), itemRect.height() );
}

void QtIconViewItem::moveBy( int dx, int dy )
{
    itemRect.moveBy( dx, dy );
}

void QtIconViewItem::move( const QPoint &pnt )
{
    move( pnt.x(), pnt.y() );
}

void QtIconViewItem::moveBy( const QPoint &pnt )
{
    moveBy( pnt.x(), pnt.y() );
}

QRect QtIconViewItem::rect()
{
    return itemRect;
}

int QtIconViewItem::x()
{
    return itemRect.x();
}

int QtIconViewItem::y()
{
    return itemRect.y();
}

int QtIconViewItem::width()
{
    return itemRect.width();
}

int QtIconViewItem::height()
{
    return itemRect.height();
}

QSize QtIconViewItem::size()
{
    return QSize( itemRect.width(), itemRect.height() );
}

QPoint QtIconViewItem::pos()
{
    return QPoint( itemRect.x(), itemRect.y() );
}

/*!
 * If relative is TRUE, the returned rectangle is relative to the origin of the viewport's
 * coordinate system, else the rectangle is relative to the origin of the item's rectangle
 */

QRect QtIconViewItem::textRect( bool relative )
{
    if ( relative )
        return itemTextRect;
    else
        return QRect( x() + itemTextRect.x(), y() + itemTextRect.y(), itemTextRect.width(), itemTextRect.height() );
}

/*!
 * If relative is TRUE, the returned rectangle is relative to the origin of the viewport's
 * coordinate system, else the rectangle is relative to the origin of the item's rectangle
 */

QRect QtIconViewItem::iconRect( bool relative )
{
    if ( relative )
        return itemIconRect;
    else
        return QRect( x() + itemIconRect.x(), y() + itemIconRect.y(), itemIconRect.width(), itemIconRect.height() );
}

bool QtIconViewItem::contains( QPoint pnt )
{
    return ( textRect( FALSE ).contains( pnt ) ||
             iconRect( FALSE ).contains( pnt ) );
}

bool QtIconViewItem::intersects( QRect r )
{
    return ( textRect( FALSE ).intersects( r ) ||
             iconRect( FALSE ).intersects( r ) );
}

void QtIconViewItem::setFont( const QFont &font )
{
    f = font;

    if ( fm )
        delete fm;

    fm = new QFontMetrics( f );
    calcRect();
    repaint();
}

QFont QtIconViewItem::font()
{
    return f;
}

void QtIconViewItem::setViewMode( QIconSet::Size mode )
{
    viewMode = mode;
    calcRect();
}

bool QtIconViewItem::acceptDrop( QMimeSource * )
{
    return FALSE;
}

QDragObject *QtIconViewItem::dragObject()
{
    return 0L;
}

void QtIconViewItem::rename()
{
    renameBox = new QtIconViewItemLineEdit( itemText, view->viewport(), this );
    renameBox->resize( textRect().width() - 2, textRect().height() - 2 );
    view->addChild( renameBox, textRect( FALSE ).x() + 1, textRect( FALSE ).y() + 1 );
    renameBox->setFrameStyle( QFrame::NoFrame );//Box | QFrame::Plain );
    renameBox->selectAll();
    renameBox->setFocus();
    renameBox->show();
    view->viewport()->setFocusProxy( renameBox );
    connect( renameBox, SIGNAL( returnPressed() ), this, SLOT( renameItem() ) );
    connect( renameBox, SIGNAL( escapePressed() ), this, SLOT( cancelRenameItem() ) );
}

void QtIconViewItem::renameItem()
{
    if ( !renameBox )
        return;

    QRect r = itemRect;
    setText( renameBox->text() );
    view->repaintContents( r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2 );
    removeRenameBox();
}

void QtIconViewItem::cancelRenameItem()
{
    if ( !renameBox )
        return;

    removeRenameBox();
}

void QtIconViewItem::removeRenameBox()
{
    if ( !renameBox )
        return;

    delete renameBox;
    renameBox = 0L;
    view->viewport()->setFocusProxy( 0L );
}

void QtIconViewItem::calcRect()
{
    if ( !fm )
        return;

    int w = 0, h = 0;

    if ( view->rastX() != -1 && fm->width( itemText ) > view->rastX() - 4 ) {
        QStringList lst;
        breakLines( itemText, lst, view->rastX() - 4 );

        QValueListIterator<QString> it = lst.begin();
        for ( ; it != lst.end(); ++it ) {
            w = QMAX( w, fm->width( *it ) );
            h += fm->height();
        }
        w += 6;
        h += 2;
    } else {
        w = fm->width( itemText ) + 6;
        h = fm->height() + 2;
    }

    itemTextRect.setWidth( w );
    itemTextRect.setHeight( h );

    w = itemIcon.pixmap( viewMode, QIconSet::Normal ).width() + 2;
    h = itemIcon.pixmap( viewMode, QIconSet::Normal ).height() + 2;

    itemIconRect.setWidth( w );
    itemIconRect.setHeight( h );

    w = QMAX( itemTextRect.width(), itemIconRect.width() );
    h = itemTextRect.height() + itemIconRect.height() + 1;

    itemRect.setWidth( w );
    itemRect.setHeight( h );

    itemTextRect = QRect( ( width() - itemTextRect.width() ) / 2, height() - itemTextRect.height(),
                          itemTextRect.width(), itemTextRect.height() );
    itemIconRect = QRect( ( width() - itemIconRect.width() ) / 2, 0,
                          itemIconRect.width(), itemIconRect.height() );
}

void QtIconViewItem::paintItem( QPainter *p )
{
    QIconSet::Mode m = QIconSet::Normal;
    if ( isSelected() )
        m = QIconSet::Active;
    else if ( !isSelectable() )
        m = QIconSet::Disabled;

    int w = itemIcon.pixmap( viewMode, QIconSet::Normal ).width();

    if ( isSelected() )
        p->fillRect( iconRect( FALSE ), view->colorGroup().highlight() );
    p->drawPixmap( x() + ( width() - w ) / 2, y(), itemIcon.pixmap( viewMode, m ) );


    p->save();
    if ( isSelected() ) {
        p->fillRect( textRect( FALSE ), view->colorGroup().highlight() );
        p->setPen( QPen( view->colorGroup().highlightedText() ) );
    }
    else
        p->setPen( view->colorGroup().text() );

    if ( view->rastX() != -1 && fm->width( itemText ) > view->rastX() - 4 ) {
        QStringList lst;
        breakLines( itemText, lst, view->rastX() - 4 );

        QValueListIterator<QString> it = lst.begin();
        int h = 0;
        for ( ; it != lst.end(); ++it ) {
            p->drawText( QRect( textRect( FALSE ).x(), h + textRect( FALSE ).y(),
                                textRect( FALSE ).width(), fm->height() ),
                         Qt::AlignCenter, *it);
            h += fm->height();
        }
    } else {
        p->drawText( textRect( FALSE ), Qt::AlignCenter, itemText );
    }

    p->restore();
}

void QtIconViewItem::paintFocus( QPainter *p )
{
    view->style().drawFocusRect( p, QRect( textRect( FALSE ).x(), textRect( FALSE ).y(), textRect( FALSE ).width(), textRect( FALSE ).height() ),
                                 view->colorGroup(), isSelected() ? &view->colorGroup().highlight() : &view->colorGroup().base(), isSelected() );
    view->style().drawFocusRect( p, QRect( iconRect( FALSE ).x(), iconRect( FALSE ).y(), iconRect( FALSE ).width(), iconRect( FALSE ).height() ),
                                 view->colorGroup(), isSelected() ? &view->colorGroup().highlight() : &view->colorGroup().base(), isSelected() );
}

void QtIconViewItem::makeActiveIcon()
{
}

void QtIconViewItem::dropped( QMimeSource * )
{
}

void QtIconViewItem::breakLines( const QString text, QStringList &lst, int width )
{
    lst.clear();

    if ( !fm )
        return;

    if ( text.isEmpty() ) {
        lst.append( text );
        return;
    }

    QStringList words;
    QString line = text;

    if ( line.simplifyWhiteSpace().find( QChar( ' ') ) == -1 ) {
        // we have only one word - do some ugly line breaking

        QString txt;
        for ( unsigned int i = 0; i < line.length(); i++ ) {
            if ( fm->width( txt ) + fm->width( line.at( i ) ) <= width ) {
                txt += line.at( i );
            } else {
                lst.append( txt );
                txt = line.at( i );
            }
        }
        if ( !txt.isEmpty() )
            lst.append( txt );

        return;
    }


    int i = text.find( QChar( ' ' ), 0 );

    while ( i != -1 ) {
        words.append( line.left( i ) );
        line.remove( 0, i + 1 );
        i = line.find( QChar( ' ' ), 0 );
    }

    if ( !line.simplifyWhiteSpace().isEmpty() )
        words.append( line.simplifyWhiteSpace() );

    QValueListIterator<QString> it = words.begin();

    int w = 0;
    line = QString::null;

    for ( ; it != words.end(); ++it ) {
        if ( w == 0 ) {
            line = *it;
            line += QChar( ' ' );
            w = fm->width( line );
        } else {
            if ( fm->width( line + *it ) <= width ) {
                line += *it;
                line += QChar( ' ' );
                w = fm->width( line );
            } else {
                lst.append( line );
                w = 0;
                --it;
            }
        }
    }

    if ( !line.isEmpty() )
        lst.append( line );
}

void QtIconViewItem::dragEntered()
{
}

void QtIconViewItem::dragLeft()
{
}

/*****************************************************************************
 *
 * Class QtIconView
 *
 *****************************************************************************/

QtIconView::QtIconView( QWidget *parent, const char *name )
    : QScrollView( parent, name )
{
    d = new QtIconViewPrivate;
    d->firstItem = 0L;
    d->lastItem = 0L;
    d->count = 0;
    d->mode = QIconSet::Automatic;
    d->mousePressed = FALSE;
    d->selectionMode = Single;
    d->currentItem = 0L;
    d->rubber = 0L;
    d->scrollTimer = 0L;
    d->startDrag = FALSE;
    d->tmpCurrentItem = 0L;
    d->rastX = d->rastY = -1;
    d->spacing = 5;
    d->cleared = FALSE;

    viewport()->setBackgroundColor( colorGroup().base() );

    setAcceptDrops( TRUE );
    viewport()->setAcceptDrops( TRUE );

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    setFocusPolicy( QWidget::StrongFocus );
    viewport()->setFocusPolicy( QWidget::StrongFocus );
}

QtIconView::~QtIconView()
{
    delete d;
}

void QtIconView::insertItem( QtIconViewItem *item, QtIconViewItem *after )
{
    if ( !item )
        return;

    if ( !d->firstItem ) {
        d->firstItem = d->lastItem = item;
        item->prev = 0L;
        item->next = 0L;
    } else {
        if ( !after || after == d->lastItem ) {
            d->lastItem->next = item;
            item->prev = d->lastItem;
            d->lastItem = item;
        } else {
            QtIconViewItem *i = d->firstItem;
            while ( i != after )
                i = i->next;

            if ( i ) {
                QtIconViewItem *next = i->next;
                item->next = next;
                item->prev = i;
                i->next = item;
                next->prev = item;
            }
        }
    }

    if ( isVisible() ) {
        insertInGrid( item );

        resizeContents( QMAX( contentsWidth(), item->x() + item->width() + 5 ),
                        QMAX( contentsHeight(), item->y() + item->height() + 5 ) );
    }

    d->count++;
}

void QtIconView::removeItem( QtIconViewItem *item )
{
    if ( !item )
        return;

    if ( item == d->firstItem )
        d->firstItem = d->firstItem->next;
    else if ( item == d->lastItem )
        d->lastItem = d->lastItem->prev;
    else {
        QtIconViewItem *i = d->firstItem;
        while ( i != item )
            i = i->next;

        if ( i ) {
            i->prev->next = i->next;
            i->next->prev = i->prev;
        }
    }

    d->count--;
}

int QtIconView::index( QtIconViewItem *item )
{
    if ( !item )
        return -1;

    if ( item == d->firstItem )
        return 0;
    else if ( item == d->lastItem )
        return d->count - 1;
    else {
        QtIconViewItem *i = d->firstItem;
        unsigned int j = 0;
        while ( i != item ) {
            i = i->next;
            ++j;
        }

        return j;
    }
}

QtIconViewItem *QtIconView::firstItem() const
{
    return d->firstItem;
}

QtIconViewItem *QtIconView::lastItem() const
{
    return d->lastItem;
}

QtIconViewItem *QtIconView::currentItem() const
{
    return d->currentItem;
}

void QtIconView::setCurrentItem( QtIconViewItem *item )
{
    d->currentItem = item;
}

unsigned int QtIconView::count()
{
    return d->count;
}

void QtIconView::setViewMode( QIconSet::Size mode )
{
    d->mode = mode;

    if ( !d->firstItem )
        return;

    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
        item->setViewMode( mode );

    repaintContents( contentsX(), contentsY(), contentsWidth(), contentsHeight() );
}

QIconSet::Size QtIconView::viewMode()
{
    return d->mode;
}

void QtIconView::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( Qt::black, 1, Qt::DashDotLine ) );
    p.setBrush( Qt::NoBrush );
    drawRubber( &p );
    p.end();

    ensureVisible( pos.x(), pos.y() );

    pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    QRect oldRubber = QRect( *d->rubber );

    d->rubber->setRight( pos.x() );
    d->rubber->setBottom( pos.y() );

    selectByRubber( oldRubber );

    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( Qt::black, 1, Qt::DashDotLine ) );
    p.setBrush( Qt::NoBrush );
    drawRubber( &p );

    p.end();

    pos = QCursor::pos();
    pos = mapFromGlobal( pos );

    if ( !QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
         !d->scrollTimer ) {
        d->scrollTimer = new QTimer( this );

        connect( d->scrollTimer, SIGNAL( timeout() ),
                 this, SLOT( doAutoScroll() ) );
        d->scrollTimer->start( 100, FALSE );
    } else if ( QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
                d->scrollTimer ) {
        disconnect( d->scrollTimer, SIGNAL( timeout() ),
                    this, SLOT( doAutoScroll() ) );
        d->scrollTimer->stop();
        delete d->scrollTimer;
        d->scrollTimer = 0L;
    }

}

void QtIconView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( !d->firstItem )
        return;

    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
        if ( item->rect().intersects( QRect( cx, cy, cw, ch ) ) )
            item->paintItem( p );

    if ( d->currentItem && d->currentItem->rect().intersects( QRect( cx, cy, cw, ch ) ) )
        d->currentItem->paintFocus( p );
}

void QtIconView::orderItemsInGrid()
{
    int w = 0, h = 0;

    resizeContents( viewport()->width(), viewport()->height() );
    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
        insertInGrid( item );
        w = QMAX( w, item->x() + item->width() );
        h = QMAX( h, item->y() + item->height() );
    }

    resizeContents( w /*+ 5*/, h /*+ 5*/ );
}

void QtIconView::show()
{
    QScrollView::show();
    orderItemsInGrid();
}

void QtIconView::setSelectionMode( SelectionMode m )
{
    d->selectionMode = m;
}

QtIconView::SelectionMode QtIconView::selectionMode()
{
    return d->selectionMode;
}

QtIconViewItem *QtIconView::findItem( const QPoint &pos )
{
    if ( !d->firstItem )
        return 0L;

    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
        if ( item->contains( pos ) )
            return item;

    return 0L;
}

void QtIconView::selectAll( bool select )
{
    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
        item->setSelected( select );
}

void QtIconView::repaintItem( QtIconViewItem *item )
{
    if ( !item )
        return;

    if ( QRect( contentsX(), contentsY(), contentsWidth(), contentsHeight() ).
         intersects( QRect( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 ) ) )
        repaintContents( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 );
}

void QtIconView::ensureItemVisible( QtIconViewItem *item )
{
    if ( !item )
        return;

    ensureVisible( item->x(), item->y(),
                   item->width(), item->height() );
}

void QtIconView::clear()
{
    if ( !d->firstItem )
        return;

    QtIconViewItem *item = d->firstItem, *tmp;
    while ( item ) {
        tmp = item->next;
        delete item;
        item = tmp;
    }

    d->count = 0;
    d->firstItem = 0L;
    d->lastItem = 0L;
    d->currentItem = 0L;
    d->tmpCurrentItem = 0L;

    setContentsPos( 0, 0 );
    resizeContents( viewport()->width(), viewport()->height() );
    viewport()->repaint( TRUE );

    d->cleared = TRUE;
}

void QtIconView::setRastX( int rx )
{
    d->rastX = rx;
}

void QtIconView::setRastY( int ry )
{
    d->rastY = ry;
}

int QtIconView::rastX()
{
    return d->rastX;
}

int QtIconView::rastY()
{
    return d->rastY;
}

void QtIconView::setSpacing( int sp )
{
    d->spacing = sp;
}

int QtIconView::spacing()
{
    return d->spacing;
}

void QtIconView::contentsMousePressEvent( QMouseEvent *e )
{
    if ( d->currentItem )
        d->currentItem->renameItem();

    if ( e->button() == LeftButton ) {
        d->startDrag = FALSE;

        QtIconViewItem *item = findItem( e->pos() );
        QtIconViewItem *oldCurrent = d->currentItem;

        if ( item  && item->isSelected() &&
             item->textRect( FALSE ).contains( e->pos() ) ) {

            if ( !item->allowRename() )
                return;

			ensureItemVisible( item );
            d->currentItem = item;
            repaintItem( item );
            item->rename();
            return;
        }

        if ( item && item->isSelectable() )
            if ( d->selectionMode == Single )
                item->setSelected( TRUE, e->state() & ControlButton );
            else
                item->setSelected( !item->isSelected(), e->state() & ControlButton );
        else
            selectAll( FALSE );

        d->currentItem = item;

        repaintItem( oldCurrent );
        repaintItem( d->currentItem );

        if ( !d->currentItem && d->selectionMode != Single ) {
            if ( d->rubber )
                delete d->rubber;
            d->rubber = 0L;
            d->rubber = new QRect( e->x(), e->y(), 0, 0 );

            if ( d->selectionMode == StrictMulti && !( e->state() & ControlButton ) )
                selectAll( FALSE );
        }

        d->mousePressed = TRUE;
    } else if ( e->button() == RightButton ) {
        QtIconViewItem *item = findItem( e->pos() );
        if ( item )
            emit itemRightClicked( item );
        else
            emit viewportRightClicked();
    }
}

void QtIconView::contentsMouseReleaseEvent( QMouseEvent * )
{
    d->mousePressed = FALSE;
    d->startDrag = FALSE;

    if ( d->rubber ) {
        QPainter p;
        p.begin( viewport() );
        p.setRasterOp( NotROP );
        p.setPen( QPen( Qt::black, 1, Qt::DashDotLine ) );
        p.setBrush( Qt::NoBrush );

        drawRubber( &p );

        p.end();

        delete d->rubber;
        d->rubber = 0L;
    }

    if ( d->scrollTimer ) {
        disconnect( d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );
        d->scrollTimer->stop();
        delete d->scrollTimer;
        d->scrollTimer = 0L;
    }
}

void QtIconView::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( d->mousePressed && d->currentItem && d->currentItem->allowDrag() ) {
        if ( !d->startDrag ) {
            d->currentItem->setSelected( TRUE, TRUE );
            d->startDrag = TRUE;
        }
        else {
            d->mousePressed = FALSE;
            d->cleared = FALSE;
            startDrag( e->state() & ControlButton );
            if ( d->tmpCurrentItem )
                repaintItem( d->tmpCurrentItem );
        }
    } else if ( d->mousePressed && !d->currentItem && d->rubber )
        doAutoScroll();
}

void QtIconView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    QtIconViewItem *item = findItem( e->pos() );
    if ( item ) {
        selectAll( FALSE );
        item->setSelected( TRUE, TRUE );
        emit doubleClicked( item );
    }
}

void QtIconView::contentsDragEnterEvent( QDragEnterEvent * )
{
    d->tmpCurrentItem = 0L;
}

void QtIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( d->tmpCurrentItem )
        repaintItem( d->tmpCurrentItem );

    QtIconViewItem *old = d->tmpCurrentItem;
    d->tmpCurrentItem = 0L;

    QtIconViewItem *item = findItem( e->pos() );

    if ( item ) {
        if ( item != old ) {
            if ( old )
                old->dragLeft();
            item->dragEntered();
        }

        if ( item->acceptDrop( e ) )
            e->accept();
        else
            e->ignore();

        d->tmpCurrentItem = item;
        QPainter p;
        p.begin( viewport() );
        p.translate( -contentsX(), -contentsY() );
        item->paintFocus( &p );
        p.end();
    } else {
        e->accept();
        if ( old )
            old->dragLeft();
    }
}

void QtIconView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    if ( d->tmpCurrentItem ) {
        repaintItem( d->tmpCurrentItem );
        d->tmpCurrentItem->dragLeft();
    }

    d->tmpCurrentItem = 0L;
}

void QtIconView::contentsDropEvent( QDropEvent *e )
{
    if ( d->tmpCurrentItem )
        repaintItem( d->tmpCurrentItem );

    QtIconViewItem *i = findItem( e->pos() );

    if ( !i && e->source() == viewport() && d->currentItem && !d->cleared ) {
        QRect r = d->currentItem->rect();

        d->currentItem->move( QPoint( e->pos().x() - d->currentItem->iconRect().width() / 2,
                                      e->pos().y() - d->currentItem->iconRect().height() / 2 ) );

        int w = d->currentItem->x() + d->currentItem->width() + 1;
        int h = d->currentItem->y() + d->currentItem->height() + 1;

        repaintItem( d->currentItem );
        repaintContents( r.x(), r.y(), r.width(), r.height() );

        if ( d->selectionMode != Single ) {
            int dx = d->currentItem->x() - r.x();
            int dy = d->currentItem->y() - r.y();

            QtIconViewItem *item = d->firstItem;
            for ( ; item; item = item->next )
                if ( item->isSelected() && item != d->currentItem ) {
                    QRect or = item->rect();
                    item->moveBy( dx, dy );
                    repaintItem( item );
                    repaintContents( or.x(), or.y(), or.width(), or.height() );
                    w = QMAX( w, item->x() + item->width() + 1 );
                    h = QMAX( h, item->y() + item->height() + 1 );
                }
        }
        bool fullRepaint = FALSE;
        if ( w > contentsWidth() ||
             h > contentsHeight() )
            fullRepaint = TRUE;

        int oldw = contentsWidth();
        int oldh = contentsHeight();

        resizeContents( QMAX( contentsWidth(), w ), QMAX( contentsHeight(), h ) );

        if ( fullRepaint ) {
            repaintContents( oldw, 0, contentsWidth() - oldw, contentsHeight() );
            repaintContents( 0, oldh, contentsWidth(), contentsHeight() - oldh );
        }
    } else if ( !i && e->source() != viewport() || d->cleared )
        emit dropped( e );
    else if ( i )
        i->dropped( e );
}

void QtIconView::keyPressEvent( QKeyEvent *e )
{
    if ( !d->firstItem )
        return;

    if ( !d->currentItem ) {
        if ( e->key() == Key_Control )
            return;

        d->currentItem = d->firstItem;

        if ( d->selectionMode == Single )
            d->currentItem->setSelected( TRUE, TRUE );

        repaintItem( d->currentItem );
        return;
    }

    switch ( e->key() )
    {
    case Key_Home:
    {
        if ( !d->firstItem )
            return;

        QtIconViewItem *item = d->currentItem;
        d->currentItem = d->firstItem;

        if ( d->selectionMode == Single ) {
            item->setSelected( FALSE );
            d->currentItem->setSelected( TRUE, TRUE );
        } else {
            if ( e->state() & ShiftButton )
                d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
        }

        repaintItem( item );
        repaintItem( d->currentItem );
    } break;
    case Key_End:
    {
        if ( !d->lastItem )
            return;

        QtIconViewItem *item = d->currentItem;
        d->currentItem = d->lastItem;

        if ( d->selectionMode == Single ) {
            item->setSelected( FALSE );
            d->currentItem->setSelected( TRUE, TRUE );
        } else {
            if ( e->state() & ShiftButton )
                d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
        }

        repaintItem( item );
        repaintItem( d->currentItem );
    } break;
    case Key_Right:
    {
        if ( !d->currentItem->next )
            return;

        QtIconViewItem *item = d->currentItem;
        d->currentItem = d->currentItem->next;

        if ( d->selectionMode == Single ) {
            item->setSelected( FALSE );
            d->currentItem->setSelected( TRUE, TRUE );
        } else {
            if ( e->state() & ShiftButton )
                d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
        }

        repaintItem( item );
        repaintItem( d->currentItem );
    } break;
    case Key_Left:
    {
        if ( !d->currentItem->prev )
            return;

        QtIconViewItem *item = d->currentItem;
        d->currentItem = d->currentItem->prev;

        if ( d->selectionMode == Single ) {
            item->setSelected( FALSE );
            d->currentItem->setSelected( TRUE, TRUE );
        } else {
            if ( e->state() & ShiftButton )
                d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
        }

        repaintItem( item );
        repaintItem( d->currentItem );
    } break;
    case Key_Space:
    {
        if ( d->selectionMode == Single )
            return;

        d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    } break;
    case Key_Enter: case Key_Return:
        emit doubleClicked( d->currentItem );
        break;
    }

    ensureItemVisible( d->currentItem );
}

void QtIconView::selectByRubber( QRect oldRubber )
{
    if ( !d->rubber )
        return;

    oldRubber = oldRubber.normalize();
    int minx = contentsWidth(), miny = contentsHeight();
    int maxx = 0, maxy = 0;
    bool ensureV = FALSE;

    QtIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
        if ( item->intersects( oldRubber ) &&
             !item->intersects( d->rubber->normalize() ) )
            item->setSelected( FALSE );
        else if ( item->intersects( d->rubber->normalize() ) ) {
            item->setSelected( TRUE, TRUE );

            minx = QMIN( minx, item->x() - 1 );
            miny = QMIN( miny, item->y() - 1 );
            maxx = QMAX( maxx, item->x() + item->width() + 1 );
            maxy = QMAX( maxy, item->y() + item->height() + 1 );

            ensureV = TRUE;
        }
    }
}

void QtIconView::drawRubber( QPainter *p )
{
    if ( !p || !d->rubber )
        return;

    QPoint pnt( d->rubber->x(), d->rubber->y() );
    pnt = contentsToViewport( pnt );
    p->drawRect( QRect( pnt.x(), pnt.y(), d->rubber->width(), d->rubber->height() ) );
}

QDragObject *QtIconView::dragObject()
{
    if ( !d->currentItem )
        return 0L;

    QtIconViewItemDrag *drag = new QtIconViewItemDrag( viewport() );
    drag->setPixmap( QPixmap( d->currentItem->icon().pixmap( d->mode, QIconSet::Normal ) ),
                     QPoint( d->currentItem->iconRect().width() / 2, d->currentItem->iconRect().height() / 2 ) );

    return drag;
}

void QtIconView::startDrag( bool move )
{
    QDragObject *drag = dragObject();
    if ( !drag )
        return;

    if ( move ) {
        if ( drag->dragMove() )
            if ( drag->target() != viewport() )
                emit moved();
    }
    else
        drag->dragCopy();
}

void QtIconView::insertInGrid( QtIconViewItem *item )
{
    if ( !item )
        return;

    int px = d->spacing;
    int py = d->spacing;
    int pw = d->rastX == -1 ? 1 : d->rastX / 2;
    int ph = d->rastY == -1 ? 1 : d->rastY / 2;
    bool isFirst = item == d->firstItem;

    if ( item->prev ) {
        px = item->prev->x();
        py = item->prev->y();
        pw = item->prev->width();
        ph = item->prev->height();
    }

    int xpos = 0;
    int ypos = 0;
    bool nextRow = FALSE;

    if ( d->rastX == -1 ) {
        xpos = px + pw + d->spacing;
        if ( xpos + item->width() >= contentsWidth() ) {
            xpos = d->spacing;
            nextRow = TRUE;
        }
    } else {
        int fact = px / d->rastX;

        if ( !isFirst )
            xpos = ( fact + 1 ) * d->rastX;
        else
            xpos = fact * d->rastX;
        xpos += ( d->rastX - item->width() ) / 2 + d->spacing;
        if ( xpos + item->width() >= contentsWidth() ) {
            xpos = d->spacing;
            nextRow = TRUE;
            xpos += ( d->rastX - item->width() ) / 2 + d->spacing;
        }
    }

    if ( d->rastY == -1 ) {
        if ( !nextRow )
            ypos = py;
        else
            ypos = py + ph + d->spacing;
    } else {
        int fact = py / d->rastY;

        if ( !nextRow )
            ypos = fact * d->rastY;
        else
            ypos = ( fact + 1 ) * d->rastY;
        ypos += ( d->rastY - item->height() ) / 2;
    }

    item->move( xpos, ypos );
}
