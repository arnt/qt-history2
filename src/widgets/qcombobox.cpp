/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.cpp#2 $
**
** Implementation of QComboBox widget class
**
** Author  : Eirik Eng
** Created : 941121
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qcombo.h"
#include "qpainter.h"
#include "qkeycode.h"
#include "qstrlist.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qcombobox.cpp#2 $";
#endif


#include <qpopmenu.h>

struct QComboData 
{
    int         current;
    QPopupMenu *popup;
    uint        autoResize : 1;
};


static bool getMetrics( int width, int height, 
                        int *dist, int *buttonW, int *buttonH )
{
    int drawH = height - 4;
    *dist     = ( drawH + 1 ) / 3;
    *buttonH  = drawH - 2*(*dist);
    *buttonW  = (*buttonH)*162/100;        // use the golden section
    if ( width - 4 < *buttonW )
        *buttonW = width - 6;
    if ( drawH < 5 || *buttonW < 5 )
        return FALSE;
    else
        return TRUE;
}

static inline bool checkInsertIndex( const char *method, int count, int *index)
{
    if ( *index > count ) {
#if defined(CHECK_RANGE)
	warning( "QComboBox::%s Index %i out of range", method, index );
#endif
	return FALSE;
    }
    if ( *index < 0 )				// append
	*index = count;
    return TRUE;
}

static inline bool checkIndex( const char *method, int count, int index )
{
    if ( index >= count ) {
#if defined(CHECK_RANGE)
	warning( "QComboBox::%s Index %i out of range", method, index );
#endif
	return FALSE;
    }
    return TRUE;
}

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    init();
}

void QComboBox::init()
{
    d             = new QComboData;
    d->popup      = new QPopupMenu;
    d->current    = 0;
    d->autoResize = FALSE;
    connect( d->popup, SIGNAL(activated(int))  ,SLOT(internalActivate(int)) );
    connect( d->popup, SIGNAL(highlighted(int)),SLOT(internalHighlight(int)) );
}

QComboBox::~QComboBox()
{
    delete d->popup;
    delete d;
}


void QComboBox::setStrList( const QStrList *l )
{
    d->popup->clear();
    d->current = 0;
    if ( !l ) {
#if defined(CHECK_NULL)
	CHECK_PTR( l );
#endif
	return;
    }
    QStrListIterator iter( *l );
    const char *tmp;
    while ( (tmp = iter.current()) ) {
	d->popup->insertItem( tmp );
	++iter;
    }
    currentChanged();
}

void QComboBox::setStrList( const char **strs,int numStrings )
{
    d->popup->clear();
    d->current = 0;
    if ( !strs ) {
#if defined ( CHECK_NULL )
	CHECK_PTR( strs );
#endif
	return;
    }
    for( int i = 0 ; i < numStrings ; i++ )
	d->popup->insertItem( strs[i] );
    currentChanged();
}

  /*!
  Inserts a string item. If index is negative the string will be appended. 
  Note that this operation will change the indexes of all items after
  the one being inserted. If \e index is larger then count() - 1
  the function returns immediately.
  */

void QComboBox::insertItem( const char *string, int index )
{
    bool append = index < 0;
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
#if defined(CHECK_NULL)
    CHECK_PTR( string );
#endif
    d->popup->insertItem( string, index );
    if ( !append )
        reIndex();
    if ( index == d->current )
        currentChanged();
}

  /*!
  Inserts a pixmap item. If index is negative the pixmap will be appended. 
  Note that this operation will change the indexes of all items after
  the one being inserted. If \e index is larger then count() - 1
  the function returns immediately.
  */

void QComboBox::insertItem( const QPixmap &pixmap, int index )
{
    bool append = index < 0;
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    d->popup->insertItem( pixmap, index );
    if ( !append )
        reIndex();
    if ( index == d->current )
        currentChanged();
}

  /*!
  Removes an item. This will change the indexes of all items after
  the one being removed. If \e index is negative or larger then count() - 1
  the function returns immediately.
  */

void QComboBox::removeItem( int index )
{
    if ( !checkIndex( "removeItem", count(), index ) )
	return;

    d->popup->removeItemAt( index );
    reIndex();
    if ( index == d->current )
        currentChanged();
}


/*
long QComboBox::accel( int index ) const		// get accelerator key
{
    return d->popup->accel( index );
}

void QComboBox::setAccel( long key, int index )		// set accelerator key
{
    d->popup->setAccel( key, index );
}
*/
  /*!
  Returns the string item at a given index. If index is out of range or
  the item indexed is not a string 0 is returned.
  */

const char *QComboBox::string( int index ) const
{
    if ( !checkIndex( "string", count(), index ) )
	return 0;
    return d->popup->string( index );
}

  /*!
  Returns the pixmap item at a given index. If index is out of range or
  the item indexed is not a pixmap 0 is returned.
  */

QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    return d->popup->pixmap( index );
}

  /*!
  Replaces the item at a given index with a string. If index is out of range 
  the operation is ignored.
  */

void QComboBox::changeItem( const char *string, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( string, index );
}

  /*!
  Replaces the item at a given index with a pixmap. If index is out of range 
  the operation is ignored.
  */

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( im, index );
}

  /*!
  Removes all items, leaving an empty combo box button.
  */
void QComboBox::clear()
{
    d->popup->clear();
    d->current = 0;
}

int QComboBox::count() const
{
    return d->popup->count();
}

  /*!
  Sets current item, i.e. the item displayed on the combo box button.
  */
void QComboBox::setCurrentItem( int index )
{
    if ( index == d->current )
	return;

    if ( !checkIndex( "setCurrentItem", count(), index ) )
	return;

    d->current = index;
    currentChanged();
}

void QComboBox::setAutoResizing( bool b )
{
    if ( d->autoResize == b )
        return;
    d->autoResize = b;
    if ( b )
        adjustSize();
}

bool QComboBox::autoResizing() const
{
    return d->autoResize;
}

void QComboBox::internalActivate( int index )
{
    if ( d->current != index ) {
        d->current = index;
        currentChanged();
    }
    emit activated( index );
}

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
}

void QComboBox::setBackgroundColor( const QColor &c )
{
    QWidget::setBackgroundColor( c );
    d->popup->setBackgroundColor( c );
}

void QComboBox::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    d->popup->setPalette( p );
}

void QComboBox::setFont( const QFont &f )
{
    QWidget::setFont( f );
    d->popup->setFont( f );
}

void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p;


    QColorGroup g  = colorGroup();
    QColor	fillcol = g.background();
	QColor tColor, bColor;

    p.begin( this );

    int dist, buttonH, buttonW;

    if ( getMetrics( width(), height(), &dist, &buttonW, &buttonH ) ) {
        int xPos = width() - 2 - dist - buttonW - 1;
        p.drawShadePanel( xPos, height() - 2 - dist - buttonH,
                         buttonW, buttonH,
                         g.light(), g.dark(), 2 );
        QFontMetrics fm( font() );
        QRect clip( 2, 2, xPos - 2 - 3, height() - 4 );
        const char *tmp = d->popup->string( d->current );
        if ( tmp ) {
            p.drawText( clip, AlignCenter | AlignVCenter, tmp );
        } else {
            QPixmap *pix = d->popup->pixmap( d->current );
            if ( pix ) {
                p.setClipRect( clip );
                p.drawPixmap( 2, 2, *pix );
                p.setClipping( FALSE );
            }
        }
    }
    p.drawShadePanel( rect(), g.light(), g.dark(), 2 );
    p.end();
}

void QComboBox::mousePressEvent( QMouseEvent *e )
{
    popup();
}

void QComboBox::mouseMoveEvent( QMouseEvent * )
{
    debug("Mouse move event");
}

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
    debug("Mouse release event");
}

void QComboBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    debug("Mouse double event");
}

void QComboBox::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Return:
	case Key_Enter:
            popup();
	    break;
	default:
	    break;
    }
}

void QComboBox::popup()
{
    d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
}

void QComboBox::reIndex()
{
    int cnt = count();
    while ( cnt-- )
        d->popup->setId( cnt, cnt );
}

void QComboBox::adjustSize()
{
    int dist, buttonH, buttonW;
    if ( !getMetrics( width(), height(), &dist, &buttonW, &buttonH ) ) {
        dist    = 0;
        buttonH = 0;
        buttonW = 0;
    }
    const char *tmp = d->popup->string( d->current );
    int w, h;
    if ( tmp ) {
        QFontMetrics fm( font() );
        w = fm.width( tmp );
        h = fm.height();
    } else {
        QPixmap *pix = d->popup->pixmap( d->current );
        if ( pix ) {
            w = pix->width();
            h = pix->height();
        } else {
            w = 0;
            h = height() - 4;
        }
    }
    debug( "w = %i", w);
    resize( 4 + 4 + w + 2*dist + buttonW, h + 4 + 4 );
}

void QComboBox::currentChanged()
{
    if ( d->autoResize )
        adjustSize();
    repaint();
}