/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtmessageview.cpp#3 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtmessageview.h"

#include <qfontmetrics.h>
#include <qheader.h>

/****************************************************************************
 *
 * Class: QTMessageViewEdit
 *
 ****************************************************************************/

QTMessageViewEdit::QTMessageViewEdit( QTMessageView *parent )
    : QMultiLineEdit( parent->viewport() )
{
    clearTableFlags();
    setTableFlags( Tbl_clipCellPainting | Tbl_smoothScrolling );
}

void QTMessageViewEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Return || e->key() == Key_Enter ) {
        emit ctrlReturnPressed();
        return;
    }
    if ( e->key() == Key_Escape ) {
        emit escapePressed();
        return;
    }

    QMultiLineEdit::keyPressEvent( e );
}

void QTMessageViewEdit::focusOutEvent( QFocusEvent * )
{
    emit escapePressed();
}

/****************************************************************************
 *
 * Class: QTMessageViewItem
 *
 ****************************************************************************/

QTMessageViewItem::QTMessageViewItem( QListView *parent )
    : QListViewItem( parent )
{
}

int QTMessageViewItem::height() const
{
    QFontMetrics fm( listView()->font() );
    QString txt = text( 0 );
    int lines = txt.contains( '\n' );
    return fm.height() * ( lines + 1 );
}

/****************************************************************************
 *
 * Class: QTMessageView
 *
 ****************************************************************************/

QTMessageView::QTMessageView( QWidget *parent, const char *name )
    : QListView( parent, name ), editCol( -1 )
{
    editBox = new QTMessageViewEdit( this );
    editBox->hide();
    editBox->setFrameStyle( QFrame::Plain | QFrame::Box );
    editBox->setLineWidth( 1 );

    connect( editBox, SIGNAL( escapePressed() ),
             this, SLOT( stopEdit() ) );
    connect( editBox, SIGNAL( ctrlReturnPressed() ),
             this, SLOT( editNext() ) );
}

void QTMessageView::viewportMousePressEvent( QMouseEvent *e )
{

    if ( editBox->isVisible() )
        stopEdit();

    if ( !hasFocus() && !viewport()->hasFocus() )
        setFocus();

    if ( e->button() != LeftButton ) {
        QListView::viewportMousePressEvent( e );
        return;
    }

    QListViewItem *i = currentItem();
    QListView::viewportMousePressEvent( e );

    if ( itemAt( e->pos() ) != i )
        return;

    if ( i == currentItem() && currentItem() )
        startEdit( e->x() + contentsX() );
}

void QTMessageView::startEdit( int x )
{
    if ( !currentItem() )
        return;

    QTMessageViewItem *item = ( QTMessageViewItem* )currentItem();

    int col = -1;

    if ( editCol == -1 ) {
        for ( unsigned int i = 1; ( int )i < header()->count(); ++i ) {
            if ( x >= header()->cellPos( i ) && x <= header()->cellPos( i ) + header()->cellSize( i ) ) {
                col = i;
                break;
            }
        }
    } else
        col = editCol;

    if ( col == -1 )
        return;

    editCol = col;

    QRect ir = itemRect( item );
    QRect r( header()->cellPos( col ) - contentsX(), ir.y(),
             header()->cellSize( col ), ir.height() );
    editBox->setGeometry( r );
    editBox->setText( item->text( col ) );
    editBox->setFocus();
    editBox->show();
    viewport()->setFocusProxy( editBox );
}

void QTMessageView::stopEdit()
{
    editBox->hide();
    viewport()->setFocusProxy( 0 );
    editCol = -1;
}

void QTMessageView::editNext()
{
    if ( !currentItem() || editCol == -1 )
        return;

    QTMessageViewItem *item = ( QTMessageViewItem* )currentItem();
    item->setText( editCol, editBox->text() );

    int col = editCol;
    stopEdit();

    QListViewItemIterator it( item );
    ++it;

    if ( it.current() ) {
        setCurrentItem( it.current() );
        setSelected( it.current(), TRUE );
        editCol = col;
        startEdit( 0 );
    } else if ( col + 1 < header()->count() ) {
        setCurrentItem( firstChild() );
        setSelected( firstChild(), TRUE );
        editCol = col + 1;
        startEdit( 0 );
    } else {
        setCurrentItem( firstChild() );
        setSelected( firstChild(), TRUE );
        editCol = 1;
        startEdit( 0 );
    }
}
