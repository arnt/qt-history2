/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.cpp#46 $
**
** Implementation of QComboBox widget class
**
** Created : 940426
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qcombo.h"
#include "qpopmenu.h"
#include "qlistbox.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qscrbar.h"   // we need the qDrawArrow function
#include "qkeycode.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qcombobox.cpp#46 $");


/*!
  \class QComboBox qcombo.h
  \brief The QComboBox widget is a combined button and popup list.

  \ingroup realwidgets

  A combo box is a kind of popup menu that is opened by pressing a button.
  The popup list contains a number of text or pixmap items.
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
    uint	autoresize            : 1;
    uint	poppedUp              : 1;
    uint	usingListBox          : 1;
    uint	mouseWasInsidePopup   : 1;
    uint	arrowPressed          : 1;
    uint	arrowDown             : 1;
    uint	discardNextMousePress : 1;
    union {
	QPopupMenu *popup;
	QListBox   *listBox;
    };
};

bool QComboBox::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
    if ( d->usingListBox ) {
	QRect r  = arrowRect();
	*buttonW = r.width();
	*buttonH = r.height();
	*dist    = 4;
	return TRUE;
    } else {
	int drawH = height() - 4;
	*dist     = ( drawH + 1 ) / 3;
	*buttonH  = drawH - 2*(*dist);
	*buttonW  = (*buttonH)*162/100;		// use the golden section
	if ( width() - 4 < *buttonW )
	    *buttonW = width() - 6;
	if ( drawH < 5 || *buttonW < 5 )
	    return FALSE;
	else
	    return TRUE;
    }
}

static inline bool checkInsertIndex( const char *method, int count, int *index)
{
    bool range_err = (*index > count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QComboBox::%s Index %d out of range", method, *index );
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}

static inline bool checkIndex( const char *method, int count, int index )
{
    bool range_err = (index >= count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QComboBox::%s Index %i out of range", method, index );
#endif
    return !range_err;
}


/*!
  Constructs a combo box widget with a parent and a name.
*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    d		  = new QComboData;
    if ( style() == WindowsStyle ) {
#if 1
	d->listBox      = new QListBox( 0, 0, WType_Popup );
#else
	d->listBox      = new QListBox;
#endif
	d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
	d->listBox->setLineWidth( 1 );
	
	d->usingListBox = TRUE;
	connect( d->listBox, SIGNAL(selected(int)),
		             SLOT(internalActivate(int)) );
	connect( d->listBox, SIGNAL(highlighted(int)),
		             SLOT(internalHighlight(int)));
    } else {
	d->popup        = new QPopupMenu;
	d->usingListBox = FALSE;
	connect( d->popup, SIGNAL(activated(int)),
		           SLOT(internalActivate(int)) );
	connect( d->popup, SIGNAL(highlighted(int)),
		           SLOT(internalHighlight(int)) );
    }
    d->current	             = 0;
    d->autoresize            = FALSE;
    d->poppedUp              = FALSE;
    d->arrowDown             = FALSE;
    d->discardNextMousePress = FALSE;
    
    //setAcceptFocus( TRUE );
}

/*!
  Destroys the combo box.
*/

QComboBox::~QComboBox()
{
    if ( !QApplication::closingDown() ) {
	if ( d->usingListBox )
	    delete d->listBox;
	else
	    delete d->popup;
    } else {
	if ( d->usingListBox )
	    d->listBox = 0;
	else
	    d->popup   = 0;
    }
    delete d;
}


/*!
  Returns the number of items in the combo box.
*/

int QComboBox::count() const
{
    if ( d->usingListBox )
	return d->listBox->count();
    else
	return d->popup->count();
}


/*!
  Inserts the list of strings at the index \e index in the combo box.
*/

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
	index = count();
    bool updcur = d->current == 0 && index == 0;
    while ( (tmp=it.current()) ) {
	++it;
	if ( d->usingListBox )
	    d->listBox->insertItem( tmp, index++ );
	else
	    d->popup->insertItem( tmp, index++ );
    }
    if ( updcur )
	currentChanged();
}

/*!
  Inserts the array of strings at the index \e index in the combo box.

  The \e numStrings argument is the number of strings.
  If \e numStrings is -1 (default), the \e strs array must be
  terminated with 0.

  Example:
  \code
    static const char *items[] = { "red", "green", "blue", 0 };
    combo->insertStrList( items );
  \endcode
*/

void QComboBox::insertStrList( const char **strings, int numStrings, int index)
{
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = count();
    bool updcur = d->current == 0 && index == 0;
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	if ( d->usingListBox )
	    d->listBox->insertItem( strings[i], index++ );
	else
	    d->popup->insertItem( strings[i], index++ );
	i++;
    }
    if ( updcur )
	currentChanged();
}


/*!
  Inserts a text item at position \e index. The item will be appended if
  \e index is negative.
*/

void QComboBox::insertItem( const char *txt, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", cnt, &index ) )
	return;
    if ( d->usingListBox )
        d->listBox->insertItem( txt, index );
    else
        d->popup->insertItem( txt, index );
    if ( index != cnt )
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
    if ( d->usingListBox )
        d->listBox->insertItem( pixmap, index );
    else
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
    if ( d->usingListBox )
	d->listBox->removeItem( index );
    else
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
    if ( d->usingListBox )
	d->listBox->clear();
    else
	d->popup->clear();
    d->current = 0;
    currentChanged();
}


/*!
  Returns the text item at a given index, or 0 if the item is not a string.
*/

const char *QComboBox::text( int index ) const
{
    if ( !checkIndex( "text", count(), index ) )
	return 0;
    if ( d->usingListBox )
	return d->listBox->text( index );
    else
	return d->popup->text( index );
}

/*!
  Returns the pixmap item at a given index, or 0 if the item is not a pixmap.
*/

const QPixmap *QComboBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    if ( d->usingListBox )
	return d->listBox->pixmap( index );
    else
	return d->popup->pixmap( index );
}

/*!
  Replaces the item at position \e index with a text.
*/

void QComboBox::changeItem( const char *text, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    if ( d->usingListBox )
	d->listBox->changeItem( text, index );
    else
	d->popup->changeItem( text, index );
}

/*!
  Replaces the item at position \e index with a pixmap.
*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    if ( d->usingListBox )
	d->listBox->changeItem( im, index );
    else
	d->popup->changeItem( im, index );
}


/*!
  Returns the current combo box item.
  \sa setCurrentItem()
*/

int QComboBox::currentItem() const
{
    return d->current;
}

/*!
  Sets the current combo box item.
  This is the item to be displayed on the combo box button.
  \sa currentItem()
*/

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

/*!
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/

bool QComboBox::autoResize() const
{
    return d->autoresize;
}

/*!
  Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
  FALSE.

  When auto-resizing is enabled, the combo box button will resize itself
  whenever the current combo box item change.

  \sa autoResize(), adjustSize()
*/

void QComboBox::setAutoResize( bool enable )
{
    if ( (bool)d->autoresize != enable ) {
	d->autoresize = enable;
	if ( enable )
	    adjustSize();
    }
}

/*!
  Returns a size which fits the contents of the combo box button.
*/
QSize QComboBox::sizeHint() const
{
    int dist, buttonH, buttonW;
    if ( !getMetrics( &dist, &buttonW, &buttonH ) ) {
	dist	= 0;
	buttonH = 0;
	buttonW = 0;
    }
    int i, w, h;
    int maxW = 0;
    int maxH = 0;
    const char *tmp;
    for( i = 0 ; i < count() ; i++ ) {
	tmp = text( i );
	if ( tmp ) {
	    QFontMetrics fm = fontMetrics();
	    w = fm.width( tmp );
	    h = fm.lineSpacing() + 1;
	} else {
	    const QPixmap *pix = pixmap( i );
	    if ( pix ) {
		w = pix->width();
		h = pix->height();
	    } else {
		w = 0;
		h = height() - 4;
	    }
	}
	if ( w > maxW )
	    maxW = w;
	if ( h > maxH )
	    maxH = h;
    }
    return QSize( 4 + 4 + maxW + 2*dist + buttonW, maxH + 4 + 4 );
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
    if ( d->usingListBox )
	popDown();
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
  Reimplements QWidget::setBackgroundColor().

  Sets the background color for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setBackgroundColor( const QColor &color )
{
    QWidget::setBackgroundColor( color );
    if ( d->usingListBox )
	d->listBox->setBackgroundColor( color );
    else
	d->popup->setBackgroundColor( color );
}

/*!
  Reimplements QWidget::setPalette().

  Sets the palette for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setPalette( const QPalette &palette )
{
    QWidget::setPalette( palette );
    if ( d->usingListBox )
	d->listBox->setPalette( palette );
    else
	d->popup->setPalette( palette );
}

/*!
  Reimplements QWidget::setFont().

  Sets the font for both the combo box button and the
  combo box popup list.
*/

void QComboBox::setFont( const QFont &font )
{
    if ( d->autoresize )
	adjustSize();
    QWidget::setFont( font );
    if ( d->usingListBox )
	d->popup->setFont( font );
    else
	d->popup->setFont( font );
}


/*!
  Handles paint events for the combo box.
*/

void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColorGroup g  = colorGroup();

    p.begin( this );

    int dist, buttonH, buttonW;


    if ( getMetrics( &dist, &buttonW, &buttonH ) ) {
	if ( d->usingListBox ) {
	    QColor	  bg  = isEnabled() ? g.base() : g.background();
	    QFontMetrics  fm  = fontMetrics();
	    const char   *str = d->listBox->text( d->current );

	    QBrush fill( bg );
	    qDrawWinPanel( &p, 0, 0, width(), height(), g, TRUE, &fill );

	    QRect arrowR = arrowRect();
	    qDrawWinPanel(&p, arrowR, g, d->arrowDown );
	    qDrawArrow( &p, DownArrow, WindowsStyle, d->arrowDown, 
			arrowR.x() + 2, arrowR.y() + 2,
			arrowR.width() - 4, arrowR.height() - 4, g );
	
	    QRect clipR( 4, 4, width()  - 4 - 4 - arrowR.width(), 
			       height() - 4 - 4 );
	    p.setClipRect( clipR );
	    if ( str ) {
		QFontMetrics fm = fontMetrics();
		p.drawText( clipR, AlignLeft | AlignVCenter | SingleLine, str);
	    } else {
		const QPixmap *pix = d->listBox->pixmap( d->current );
		if ( pix ) {
		    p.drawPixmap( 4, (height()-pix->height())/2, *pix );
		}
	    }
	    p.setClipping( FALSE );
	} else {
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
	    qDrawShadePanel( &p, rect(), g, FALSE, 2 );
	}
    } else {
	qDrawShadePanel( &p, rect(), g, FALSE, 2 );
    }
    p.end();
}

QRect QComboBox::arrowRect() const
{
    return QRect( width() - 2 - 16, 2, 16, height() - 4 );
}

/*!
  Handles mouse press events for the combo box.
*/

void QComboBox::mousePressEvent( QMouseEvent *e )
{
    if ( d->discardNextMousePress ) {
	d->discardNextMousePress = FALSE;
	return;
    }
    popup();
    if ( style() == WindowsStyle ) {
	if ( arrowRect().contains( e->pos() ) ) {
	    d->arrowPressed = TRUE;
	    d->arrowDown    = TRUE;
	    repaint( FALSE );
	} else {
	    d->arrowPressed = FALSE;
	}
    } else {
	QMouseEvent me1( Event_MouseButtonPress,
			 d->popup->mapFromGlobal(mapToGlobal( QPoint(0,0) ) ),
			 e->button(), e->state() );
	QApplication::sendEvent( d->popup, &me1 );
	QMouseEvent me2( Event_MouseMove,
			 d->popup->mapFromGlobal(mapToGlobal( e->pos() ) ),
			 e->button(), e->state() );
	QApplication::sendEvent( d->popup, &me2 );
    }
}

/*!
  Handles mouse move events for the combo box.
*/

void QComboBox::mouseMoveEvent( QMouseEvent * )
{
}

/*!
  Handles mouse release events for the combo box.
*/

void QComboBox::mouseReleaseEvent( QMouseEvent * )
{
}

/*!
  Handles mouse double click events for the combo box.
*/

void QComboBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}


/*!
  Handles key press events for the combo box.  Only Enter and Return
  are accepted.
*/

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


/* 
    ### Should be handled by QListBox 
*/
static int listHeight( QListBox *l )
{
    int i,h;
    const char *tmp;
    int sumH = 0;
    for( i = 0 ; i < (int) l->count() ; i++ ) {
	tmp = l->text( i );
	if ( tmp ) {
	    QFontMetrics fm = l->fontMetrics();
	    h = fm.lineSpacing() + 1;
	} else {
	    const QPixmap *pix = l->pixmap( i );
	    if ( pix ) {
		h = pix->height();
	    } else {
		h = 0;
	    }
	}
	sumH += h;
    }
    return sumH;
}


/*!
  Popups the combo box popup list.
*/

void QComboBox::popup()
{
    if ( d->usingListBox ) {
	                // Send all listbox events to eventFilter():
	d->mouseWasInsidePopup = FALSE;
	d->listBox->installEventFilter( this );
	d->listBox->resize( width(), listHeight(d->listBox) + 2 );
	QWidget *desktop = QApplication::desktop();
	int sw = desktop->width();			// screen width
	int sh = desktop->height();			// screen height
	QPoint pos = mapToGlobal( QPoint(0,height()) );
	int x  = pos.x();
	int y  = pos.y();
	int w  = width();
	int h  = height();
	if ( x+w > sw )				// the complete widget must
	    x = sw - w;				//   be visible
	if ( y+h > sh )
	    y = sh - h;
	if ( x < 0 )
	    x = 0;
	if ( y < 0 )
	    y = 0;
	d->listBox->move( x, y );
	d->listBox->raise();
	d->listBox->setCurrentItem( d->current );
	d->listBox->show();
    } else {
	d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
    }
    d->poppedUp = TRUE;
}
/*!
  Pops down (removes) the combo box popup list.
*/

void QComboBox::popDown()
{
    if ( d->usingListBox ) {
	d->listBox->removeEventFilter( this );
	d->listBox->hide();
	if ( d->arrowDown ) {
	    d->arrowDown = FALSE;
	    repaint( FALSE );
	}
	d->poppedUp = FALSE;
	
    }    
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
    if ( d->autoresize )
	adjustSize();
    repaint();
}

bool QComboBox::eventFilter( QObject *object, QEvent *event )
{
    ASSERT( d->usingListBox );
    ASSERT( object == d->listBox );

    QListBox *listB = d->listBox;
    switch ( event->type() ) {
        case Event_MouseMove: {
	    QMouseEvent *e = Q_MOUSE_EVENT(event);
	    if ( !d->mouseWasInsidePopup  ) {
		QPoint pos = e->pos();
		if ( listB->rect().contains( pos ) )
		    d->mouseWasInsidePopup = TRUE;
	                        // Check if arrow button should toggle:
		if ( d->arrowPressed ) {
		    QPoint comboPos = 
			mapFromGlobal( listB->mapToGlobal(pos) );
		    if ( arrowRect().contains( comboPos ) ) {
			if ( !d->arrowDown  ) {
			    d->arrowDown = TRUE;
			    repaint( FALSE );
			}
		    } else {
			if ( d->arrowDown  ) {
			    d->arrowDown = FALSE;
			    repaint( FALSE );
			}
		    }
		}
	    }
	    break;
	}
        case Event_MouseButtonRelease: {
	    QMouseEvent *e = Q_MOUSE_EVENT(event);
	    if ( listB->rect().contains( e->pos() ) ) {
		QMouseEvent tmp( Event_MouseButtonDblClick, 
				 e->pos(), e->button(), e->state() ) ;
		QApplication::sendEvent( object, &tmp );  // will hide popup
	    } else {
		if ( d->mouseWasInsidePopup ) {
		    popDown();
		} else {
		    d->arrowPressed = FALSE;
		    if ( d->arrowDown  ) {
			d->arrowDown = FALSE;
			repaint( FALSE );
		    }
		}
	    }
	    break;
	}
        case Event_MouseButtonDblClick:
        case Event_MouseButtonPress: {
	    QMouseEvent *e = Q_MOUSE_EVENT(event);
	    if ( !listB->rect().contains( e->pos() ) ) {
		QPoint comboPos = mapFromGlobal(listB->mapToGlobal(e->pos()));
		if ( rect().contains( comboPos ) )
		    d->discardNextMousePress = TRUE; // avoid popping up again
		popDown();
		return TRUE;			// Block the event;
	    }
	    break;
	}
	default:
	    break;
    }
    return FALSE;				// Don't block the event
}
