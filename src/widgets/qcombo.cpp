/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombo.cpp#7 $
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
#include "qpopmenu.h"
#include "qpainter.h"
#include "qkeycode.h"
#include "qstrlist.h"
#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qcombo.cpp#7 $";
#endif

/*!
\class QComboBox qcombo.h
\brief The QComboBox widget is a combined button and popup list.

A combo box is a kind of popup menu that is opened by pressing a button.
The popup list contains a number of text/pixmap items.
The button displays the current item when the popup list is closed.

A combo box emits two signals, activated() and highlighted(), when
a new item has been activated (selected) or highlighted (set to current).
*/


/*!
\fn void QComboBox::activated( int index )
This signal is emitted when a new item has been activated (selected).
The \e index is the position of the item in the popup list.
*/

/*!
\fn void QComboBox::highlighted( int index )
This signal is emitted when a new item has been set to current.
The \e index is the position of the item in the popup list.
*/


struct QComboData 
{
    int		current;
    QPopupMenu *popup;
    uint	autoResize : 1;
};


static bool getMetrics( int width, int height, 
			int *dist, int *buttonW, int *buttonH )
{
    int drawH = height - 4;
    *dist     = ( drawH + 1 ) / 3;
    *buttonH  = drawH - 2*(*dist);
    *buttonW  = (*buttonH)*162/100;	   // use the golden section
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


/*!
Constructs a combo box widget with a parent and a name.
*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    init();
}

void QComboBox::init()
{
    d		  = new QComboData;
    d->popup	  = new QPopupMenu;
    d->current	  = 0;
    d->autoResize = FALSE;
    connect( d->popup, SIGNAL(activated(int))  ,SLOT(internalActivate(int)) );
    connect( d->popup, SIGNAL(highlighted(int)),SLOT(internalHighlight(int)) );
}

/*!
Destroys the combo box.
*/

QComboBox::~QComboBox()
{
    delete d->popup;
    delete d;
}


/*!
Returns the number of items in the combo box.
*/

int QComboBox::count() const
{
    return d->popup->count();
}


/*!
Sets the contents of the combo box to the list of strings \e list.
*/

void QComboBox::setStrList( const QStrList *list )
{
    d->popup->clear();
    d->current = 0;
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator iter( *list );
    const char *tmp;
    while ( (tmp = iter.current()) ) {
	d->popup->insertItem( tmp );
	++iter;
    }
    currentChanged();
}

/*!
Sets the contents of the combo box to the array of strings \e strs.

The \e numStrings argument is the number of strings.
If \e numStrings is -1 (default), the \e strs array must be
terminated with 0.

Example of use:
\code
  static const char *items[] = { "red", "green", "blue", 0 };
  combo->setStrList( items );
\endcode
*/

void QComboBox::setStrList( const char **strs, int numStrings )
{
    d->popup->clear();
    d->current = 0;
    if ( !strs ) {
#if defined(CHECK_NULL)
	ASSERT( strs != 0 );
#endif
	return;
    }
    int i = 0;
    while ( (numStrings<0 && strs[i]!=0) || i<numStrings )
	d->popup->insertItem( strs[i++] );
    currentChanged();
}


/*!
Inserts a text item at position \e index. The item will be appended if
\e index is negative.
*/

void QComboBox::insertItem( const char *text, int index )
{
    int cnt = count();
    bool append = index < 0 || index == cnt;
    if ( !checkInsertIndex( "insertItem", cnt, &index ) )
	return;
    d->popup->insertItem( text, index );
    if ( !append )
	reIndex();
    if ( index == d->current )
	currentChanged();
}

/*!
Inserts a pixmap item at position \e index. The item will be appended if
\e index is negative.
*/

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


/*!
Removes the item at position \e index.
*/

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


/*!
Removes all combo box items.
*/

void QComboBox::clear()
{
    d->popup->clear();
    d->current = 0;
}


/*!
Returns the text item at a given index, or 0 if the item is not a text.
*/

const char *QComboBox::text( int index ) const
{
    if ( !checkIndex( "text", count(), index ) )
	return 0;
    return d->popup->text( index );
}

/*!
Returns the pixmap item at a given index, or 0 if the item is not a pixmap.
*/

QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    return d->popup->pixmap( index );
}

/*!
Replaces the item at position \e index with a text.
*/

void QComboBox::changeItem( const char *text, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( text, index );
}

/*!
Replaces the item at position \e index with a pixmap.
*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    d->popup->changeItem( im, index );
}


/*!
Sets current item, i.e. the item to be displayed on the combo box button.
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


/*!
Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
FALSE.

When auto-resizing is enabled, the combo box button will resize itself whenever
the current combo box item change.

\sa autoResizing() and adjustSize().
*/

void QComboBox::setAutoResizing( bool enable )
{
    if ( d->autoResize == enable )
	return;
    d->autoResize = enable;
    if ( enable )
	adjustSize();
}

/*!
Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
disabled.

Auto-resizing is disabled by default.

\sa setAutoResizing().
*/

bool QComboBox::autoResizing() const
{
    return d->autoResize;
}

/*!
Adjusts the size of the combo box button to fit the contents.

This function is called automatically whenever the current item change and
auto-resizing is enabled.

\sa setAutoResizing()
*/

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
    resize( 4 + 4 + w + 2*dist + buttonW, h + 4 + 4 );
}


/*!
\internal
Receives activated signals from an internal popup list and emits
the activated() signal.
*/

void QComboBox::internalActivate( int index )
{
    if ( d->current != index ) {
	d->current = index;
	currentChanged();
    }
    emit activated( index );
}

/*!
\internal
Receives highlighted signals from an internal popup list and emits
the highlighted() signal.
*/

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
}


/*!
Reimplementes the virtual function QWidget::setBackgroundColor().

Sets the background color for both the combo box button and the
combo box popup list.
*/

void QComboBox::setBackgroundColor( const QColor &color )
{
    QWidget::setBackgroundColor( color );
    d->popup->setBackgroundColor( color );
}

/*!
Reimplementes the virtual function QWidget::setPalette().

Sets the palette for both the combo box button and the
combo box popup list.
*/

void QComboBox::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    d->popup->setPalette( palette );
}

/*!
Reimplementes the virtual function QWidget::setFont().

Sets the font for both the combo box button and the
combo box popup list.
*/

void QComboBox::setFont( const QFont &font )
{
    QWidget::setFont( font );
    d->popup->setFont( font );
}


void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColorGroup g  = colorGroup();

    p.begin( this );
    int dist, buttonH, buttonW;

    if ( getMetrics( width(), height(), &dist, &buttonW, &buttonH ) ) {
	int xPos = width() - 2 - dist - buttonW - 1;
	p.drawShadePanel( xPos, height() - 2 - dist - buttonH,
			 buttonW, buttonH,
			 g.light(), g.dark(), 2 );
	QFontMetrics fm( font() );
	QRect clip( 2, 2, xPos - 2 - 3, height() - 4 );
	const char *tmp = d->popup->text( d->current );
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
}

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
}

void QComboBox::mouseDoubleClickEvent( QMouseEvent *e )
{
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


/*!
Popups the combo box popup list.
*/

void QComboBox::popup()
{
    d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
}


/*!
\internal
Re-indexes the identifiers in the popup list.
*/

void QComboBox::reIndex()
{
    int cnt = count();
    while ( cnt-- )
	d->popup->setId( cnt, cnt );
}

/*!
\internal
Repaints the combo box.
*/

void QComboBox::currentChanged()
{
    if ( d->autoResize )
	adjustSize();
    repaint();
}
