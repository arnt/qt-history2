/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.cpp#35 $
**
** Implementation of QComboBox widget class
**
** Author  : Eirik Eng
** Created : 940426
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qcombo.h"
#include "qpopmenu.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qkeycode.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qcombobox.cpp#35 $")


/*----------------------------------------------------------------------------
  \class QComboBox qcombo.h
  \brief The QComboBox widget is a combined button and popup list.

  \ingroup realwidgets

  A combo box is a kind of popup menu that is opened by pressing a button.
  The popup list contains a number of text or pixmap items.
  The button displays the current item when the popup list is closed.

  A combo box emits two signals, activated() and highlighted(), when
  a new item has been activated (selected) or highlighted (set to current).
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn void QComboBox::activated( int index )
  This signal is emitted when a new item has been activated (selected).
  The \e index is the position of the item in the popup list.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QComboBox::highlighted( int index )
  This signal is emitted when a new item has been set to current.
  The \e index is the position of the item in the popup list.
 ----------------------------------------------------------------------------*/


struct QComboData
{
    int		current;
    QPopupMenu *popup;
    uint	autoresize : 1;
};

static bool getMetrics( int width, int height,
			int *dist, int *buttonW, int *buttonH )
{
    int drawH = height - 4;
    *dist     = ( drawH + 1 ) / 3;
    *buttonH  = drawH - 2*(*dist);
    *buttonW  = (*buttonH)*162/100;		// use the golden section
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
	warning( "QComboBox::%s Index %d out of range", method, *index );
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


/*----------------------------------------------------------------------------
  Constructs a combo box widget with a parent and a name.
 ----------------------------------------------------------------------------*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    d		  = new QComboData;
    d->popup	  = new QPopupMenu;
    d->current	  = 0;
    d->autoresize = FALSE;
    connect( d->popup, SIGNAL(activated(int))  ,SLOT(internalActivate(int)) );
    connect( d->popup, SIGNAL(highlighted(int)),SLOT(internalHighlight(int)) );
    //setAcceptFocus( TRUE );
}

/*----------------------------------------------------------------------------
  Destroys the combo box.
 ----------------------------------------------------------------------------*/

QComboBox::~QComboBox()
{
    if ( !QApplication::closingDown() )
	delete d->popup;
    else
	d->popup = 0;
    delete d;
}


/*----------------------------------------------------------------------------
  Returns the number of items in the combo box.
 ----------------------------------------------------------------------------*/

int QComboBox::count() const
{
    return d->popup->count();
}


/*----------------------------------------------------------------------------
  Inserts the list of strings at the index \e index in the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::insertStrList( const QStrList *list, int index )
{
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char *tmp;
    if ( index < 0 )
	index = d->popup->count();
    bool updcur = d->current == 0 && index == 0;
    while ( (tmp=it.current()) ) {
	++it;
	d->popup->insertItem( tmp, index++ );
    }
    if ( updcur )
	currentChanged();
}

/*----------------------------------------------------------------------------
  Inserts the array of strings at the index \e index in the combo box.

  The \e numStrings argument is the number of strings.
  If \e numStrings is -1 (default), the \e strs array must be
  terminated with 0.

  Example:
  \code
    static const char *items[] = { "red", "green", "blue", 0 };
    combo->insertStrList( items );
  \endcode
 ----------------------------------------------------------------------------*/

void QComboBox::insertStrList( const char **strings, int numStrings, int index)
{
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = d->popup->count();
    bool updcur = d->current == 0 && index == 0;
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	d->popup->insertItem( strings[i], index++ );
	i++;
    }
    if ( updcur )
	currentChanged();
}


/*----------------------------------------------------------------------------
  Inserts a text item at position \e index. The item will be appended if
  \e index is negative.
 ----------------------------------------------------------------------------*/

void QComboBox::insertItem( const char *text, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", cnt, &index ) )
	return;
    d->popup->insertItem( text, index, index );
    if ( index != cnt )
	reIndex();
    if ( index == d->current )
	currentChanged();
}

/*----------------------------------------------------------------------------
  Inserts a pixmap item at position \e index. The item will be appended if
  \e index is negative.
 ----------------------------------------------------------------------------*/

void QComboBox::insertItem( const QPixmap &pixmap, int index )
{
    int cnt = count();
    bool append = index < 0 || index == cnt;
    if ( !checkInsertIndex( "insertItem", cnt, &index ) )
	return;
    d->popup->insertItem( pixmap, index );
    if ( !append )
	reIndex();
    if ( index == d->current )
	currentChanged();
}


/*----------------------------------------------------------------------------
  Removes the item at position \e index.
 ----------------------------------------------------------------------------*/

void QComboBox::removeItem( int index )
{
    int cnt = count();
    if ( !checkIndex( "removeItem", cnt, index ) )
	return;
    d->popup->removeItemAt( index );
    if ( index != cnt-1 )
	reIndex();
    if ( index == d->current )
	currentChanged();
}


/*----------------------------------------------------------------------------
  Removes all combo box items.
 ----------------------------------------------------------------------------*/

void QComboBox::clear()
{
    d->popup->clear();
    d->current = 0;
    currentChanged();
}


/*----------------------------------------------------------------------------
  Returns the text item at a given index, or 0 if the item is not a string.
 ----------------------------------------------------------------------------*/

const char *QComboBox::text( int index ) const
{
    if ( !checkIndex( "text", count(), index ) )
	return 0;
    return d->popup->text( index );
}

/*----------------------------------------------------------------------------
  Returns the pixmap item at a given index, or 0 if the item is not a pixmap.
 ----------------------------------------------------------------------------*/

QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    return d->popup->pixmap( index );
}

/*----------------------------------------------------------------------------
  Replaces the item at position \e index with a text.
 ----------------------------------------------------------------------------*/

void QComboBox::changeItem( const char *text, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( text, index );
}

/*----------------------------------------------------------------------------
  Replaces the item at position \e index with a pixmap.
 ----------------------------------------------------------------------------*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( im, index );
}


/*----------------------------------------------------------------------------
  Returns the current combo box item.
  \sa setCurrentItem()
 ----------------------------------------------------------------------------*/

int QComboBox::currentItem() const
{
    return d->current;
}

/*----------------------------------------------------------------------------
  Sets the current combo box item.
  This is the item to be displayed on the combo box button.
  \sa currentItem()
 ----------------------------------------------------------------------------*/

void QComboBox::setCurrentItem( int index )
{
    if ( index == d->current )
	return;
    if ( !checkIndex( "setCurrentItem", count(), index ) ) {
	return;
    }
    d->current = index;
    currentChanged();
}


/*----------------------------------------------------------------------------
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

bool QComboBox::autoResize() const
{
    return d->autoresize;
}

/*----------------------------------------------------------------------------
  Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  When auto-resizing is enabled, the combo box button will resize itself
  whenever the current combo box item change.

  \sa autoResize(), adjustSize()
 ----------------------------------------------------------------------------*/

void QComboBox::setAutoResize( bool enable )
{
    if ( (bool)d->autoresize != enable ) {
	d->autoresize = enable;
	if ( enable )
	    adjustSize();
    }
}

/*----------------------------------------------------------------------------
  Adjusts the size of the combo box button to fit the contents.

  This function is called automatically whenever the current item changes and
  auto-resizing is enabled.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QComboBox::adjustSize()
{
    int dist, buttonH, buttonW;
    if ( !getMetrics( width(), height(), &dist, &buttonW, &buttonH ) ) {
	dist	= 0;
	buttonH = 0;
	buttonW = 0;
    }
    const char *tmp = d->popup->text( d->current );
    int w, h;
    if ( tmp ) {
	QFontMetrics fm = fontMetrics();
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
    resize( 4 + 4 + w + 2*dist + buttonW, h + 4 + 4 );
}


/*----------------------------------------------------------------------------
  \internal
  Receives activated signals from an internal popup list and emits
  the activated() signal.
 ----------------------------------------------------------------------------*/

void QComboBox::internalActivate( int index )
{
    if ( d->current != index ) {
	d->current = index;
	currentChanged();
    }
    emit activated( index );
}

/*----------------------------------------------------------------------------
  \internal
  Receives highlighted signals from an internal popup list and emits
  the highlighted() signal.
 ----------------------------------------------------------------------------*/

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
}


/*----------------------------------------------------------------------------
  Reimplements QWidget::setBackgroundColor().

  Sets the background color for both the combo box button and the
  combo box popup list.
 ----------------------------------------------------------------------------*/

void QComboBox::setBackgroundColor( const QColor &color )
{
    QWidget::setBackgroundColor( color );
    d->popup->setBackgroundColor( color );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::setPalette().

  Sets the palette for both the combo box button and the
  combo box popup list.
 ----------------------------------------------------------------------------*/

void QComboBox::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    d->popup->setPalette( palette );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::setFont().

  Sets the font for both the combo box button and the
  combo box popup list.
 ----------------------------------------------------------------------------*/

void QComboBox::setFont( const QFont &font )
{
    if ( d->autoresize )
	adjustSize();
    QWidget::setFont( font );
    d->popup->setFont( font );
}


/*----------------------------------------------------------------------------
  Handles paint events for the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColorGroup g  = colorGroup();

    p.begin( this );
    int dist, buttonH, buttonW;

    if ( getMetrics( width(), height(), &dist, &buttonW, &buttonH ) ) {
	int xPos = width() - dist - buttonW - 1;
	qDrawShadePanel( &p, xPos, height() - 2 - dist - buttonH,
			 buttonW, buttonH, g, FALSE, 2 );
	QFontMetrics fm = p.fontMetrics();
	QRect clip( 4, 2, xPos - 2 - 4, height() - 4 );
	const char *str = d->popup->text( d->current );
	if ( str ) {
	    p.drawText( clip, AlignCenter | SingleLine, str );
	} else {
	    QPixmap *pix = d->popup->pixmap( d->current );
	    if ( pix ) {
		p.setClipRect( clip );
		p.drawPixmap( 4, (height()-pix->height())/2, *pix );
		p.setClipping( FALSE );
	    }
	}
    }
    qDrawShadePanel( &p, rect(), g, FALSE, 2 );
    p.end();
}


/*----------------------------------------------------------------------------
  Handles mouse press events for the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::mousePressEvent( QMouseEvent * )
{
    popup();
}

/*----------------------------------------------------------------------------
  Handles mouse move events for the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::mouseMoveEvent( QMouseEvent * )
{
}

/*----------------------------------------------------------------------------
  Handles mouse release events for the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
}

/*----------------------------------------------------------------------------
  Handles mouse double click events for the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::mouseDoubleClickEvent( QMouseEvent * )
{
}


/*----------------------------------------------------------------------------
  Handles key press events for the combo box.  Only Enter and Return
  are accepted.
 ----------------------------------------------------------------------------*/

void QComboBox::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Return:
	case Key_Enter:
	    e->accept();
	    popup();
	    break;
	default:
	    e->ignore();
	    break;
    }
}


/*----------------------------------------------------------------------------
  Popups the combo box popup list.
 ----------------------------------------------------------------------------*/

void QComboBox::popup()
{
    d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
}


/*----------------------------------------------------------------------------
  \internal
  Re-indexes the identifiers in the popup list.
 ----------------------------------------------------------------------------*/

void QComboBox::reIndex()
{
    int cnt = count();
    while ( cnt-- )
	d->popup->setId( cnt, cnt );
}

/*----------------------------------------------------------------------------
  \internal
  Repaints the combo box.
 ----------------------------------------------------------------------------*/

void QComboBox::currentChanged()
{
    if ( d->autoresize )
	adjustSize();
    repaint();
}
