/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.cpp#2 $
**
** Implementation of QListBox class
**
** Author  : Eirik Eng
** Created : 941121
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlistbox.h"
#include "qfontmet.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlistbox.cpp#2 $";
#endif


QListBox::QListBox( QWidget *parent, const char *name )
    : QTableWidget( parent, name )
{
    initMetaObject();

    QFontMetrics fm( font() );

    doDrag       = TRUE;
    doAutoScroll = TRUE;
    strList      = 0;
    selected     = -1;
    isTiming     = FALSE;
    setCellWidth( 100 );
    setCellHeight( fm.height() + 4 );
    setNumCols( 1 );
    setAutoVerScrollBar( TRUE );
    setClipCellPainting( FALSE );
    setCutCellsHor( TRUE );
}

QListBox::~QListBox()
{
    delete strList;
}


void QListBox::setStrList( QStrList *l, bool copy )
{
    copy = copy;

    strList  = l;
    bool tmp = activeUpdate();
    if ( tmp ) {
        setActiveUpdate( FALSE );
    }
    if ( !l )
        setNumRows( 0 );
    else
        setNumRows( l->count() );
    setTopCell( 0 );
    if ( tmp ) {
        setActiveUpdate( TRUE );
        update(); //repaint(); ###
    }
}

void QListBox::setStrList( const char **strs,int numStrings, bool copy )
{
    copy = copy;

    QStrList *tmp = new QStrList;  // Not good!!!

    for( int i = 0 ; i < numStrings ; i++ )
        tmp->append( strs[i] );
    setStrList( tmp, copy );
}

void QListBox::setSelectedItem( long index )
{
    if( !strList )
        return;

    if ( index == selected )
        return;

    if ( index < 0 || index >= strList->count() ) {
#if defined(CHECK_RANGE)
        warning( "QListBox::setSelectedItem: Index %i out of range.", index );
#endif
        return;
    }

    debug( "Selected = %i", index );

    long oldSelected = selected;
    selected = index;
    updateItem( oldSelected );
    updateItem( selected, FALSE ); // Do not clear, selected marker covers item

}

bool QListBox::itemVisible( long index )
{
    return( itemYPos( index ) != -1 );
}

void QListBox::paintCell( QPainter *p, long row, long column )
{
   debug("Paintint %i, selected = %i", row, selected );
    QFontMetrics fm( font() );

    if ( !strList )
        return;

    if ( column )
        debug( "QListBox::paintCell: Column = %i!", column );

    char *tmp = strList->at( row );
    if ( !tmp )
        return;

    if ( selected == row ) {
        p->fillRect( 0, 0, cellWidth( column ), cellHeight( row ), black );
        p->pen().setColor( backgroundColor() );
    } else {
        p->pen().setColor( black );
    }
    p->drawText( 2, cellHeight( row ) - 2 - fm.descent(), tmp );
}


void QListBox::mousePressEvent( QMouseEvent *e )
{
    long itemClicked = findItem( e->pos().y() );
    if ( itemClicked != -1 ) {
        setSelectedItem( itemClicked );
    }
}

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    if ( doDrag ) {
        int itemClicked = findItem( e->pos().y() );
        if ( itemClicked != -1 ) {
            if ( isTiming ) {
                killTimers();
                isTiming = FALSE;
            }
            setSelectedItem( itemClicked );
            return;
        } else {
            if ( !doAutoScroll )
                return;
            if ( e->pos().y() < topMargin() )
                scrollDown = FALSE;
            else
                scrollDown = TRUE;
            if ( !isTiming ) {
                isTiming = TRUE;
                startTimer( 100 );
            }
        }
    }
}

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( doDrag )
        mouseMoveEvent( e );
    if ( isTiming ) {
        killTimers();
        isTiming = FALSE;
    }
}

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mouseReleaseEvent( e );
}

void QListBox::resizeEvent( QResizeEvent *e )
{
    setCellWidth( clientWidth() );
    QTableWidget::resizeEvent( e );
}

void QListBox::timerEvent( QTimerEvent *e )
{
    if ( scrollDown ) {
        if ( topItem() != maxRowOffset() ) {
            setTopItem( topItem() + 1 );
            setSelectedItem( maxRowVisible() );
        }
    } else {
        if ( topItem() != 0 ) {
            setTopItem( topItem() - 1 );
            setSelectedItem( topItem() );
        }
    }
}
