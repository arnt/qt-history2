/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombo.cpp#53 $
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
#include "qtimer.h"
#include "qapp.h"
#include "qlined.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qcombo.cpp#53 $");


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
    uint	usingListBox          : 1;
    uint	autoresize            : 1;
    uint	poppedUp              : 1;
    uint	mouseWasInsidePopup   : 1;
    uint	arrowPressed          : 1;
    uint	arrowDown             : 1;
    uint	discardNextMousePress : 1;
    uint	shortClick	      : 1;
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
	*dist     = 8;
	*buttonH  = 7;
	*buttonW  = 11;
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
	d->listBox      = new QListBox( 0, 0, WType_Popup );
	d->listBox->setAutoScrollBar( FALSE );
	d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
	d->listBox->setLineWidth( 1 );
	d->listBox->resize( 100, 10 );
	
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
    d->shortClick            = FALSE;
    
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

void QComboBox::insertItem( const char *text, int index )
{
    int cnt = count();
    if ( !checkInsertIndex( "insertItem", cnt, &index ) )
	return;
    if ( d->usingListBox )
        d->listBox->insertItem( text, index );
    else
        d->popup->insertItem( text, index );
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
    getMetrics( &dist, &buttonW, &buttonH );

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
	popDownListBox();
    else
	d->popup->removeEventFilter( this );
    d->poppedUp = FALSE;
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
  \internal
  Receives timeouts after a click. Used to decide if a Motif style
  popup should stay up or not after a click.
*/
void QComboBox::internalClickTimeout()
{
    d->shortClick = FALSE;
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
    QWidget::setFont( font );
    if ( d->usingListBox )
	d->listBox->setFont( font );
    else
	d->popup->setFont( font );
    if ( d->autoresize )
	adjustSize();
}


/*!
  Handles paint events for the combo box.
*/

void QComboBox::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColorGroup g  = colorGroup();

    p.begin( this );

    if ( width() < 5 || height() < 5 ) {
	qDrawShadePanel( &p, rect(), g, FALSE, 2 );
        p.end();
	return;
    }

    if ( d->usingListBox ) {		// windows style
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

	QRect clipR( 5, 4, width()  - 5 - 4 - arrowR.width(), 
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
    } else {				// Motif style
	int dist, buttonH, buttonW;

	getMetrics( &dist, &buttonW, &buttonH );
	int xPos = width() - dist - buttonW - 1;
	qDrawShadePanel( &p, xPos, (height() - buttonH)/2,
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
    p.end();
}

/*!
  \internal
  Returns the button arrow rectangle for windows style combo boxes.
*/
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
	QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	d->shortClick = TRUE;
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


/*!
  \internal
   Calculates the listbox height needed to contain all items.
*/
static int listHeight( QListBox *l )
{
    int i;
    int sumH = 0;
    for( i = 0 ; i < (int) l->count() ; i++ ) {
	sumH += l->itemHeight( i );
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
	d->listBox->installEventFilter( this );
	d->mouseWasInsidePopup = FALSE;
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
	d->popup->installEventFilter( this );
	d->popup->popup( mapToGlobal( QPoint(0,0) ), d->current );
    }
    d->poppedUp = TRUE;
}

/*!
  \internal
  Pops down (removes) the combo box popup list box.
*/
void QComboBox::popDownListBox()
{
    ASSERT( d->usingListBox );
    d->listBox->removeEventFilter( this );
    d->listBox->hide();
    if ( d->arrowDown ) {
	d->arrowDown = FALSE;
	repaint( FALSE );
    }
    d->poppedUp = FALSE;
}


/*!
  \internal
  Re-indexes the identifiers in the popup list.
*/

void QComboBox::reIndex()
{
    if ( !d->usingListBox ) {
	int cnt = count();
	while ( cnt-- )
	    d->popup->setId( cnt, cnt );
    }
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

/* nobang
  \internal 
   The event filter steals events from the popup or listbox
   when they are popped up. It makes the popup stay up after a short click
   in motif style. In windows style it toggles the arrow button of the
   combo box field, and activates an item and takes down the listbox when
   the mouse button is released.
*/

bool QComboBox::eventFilter( QObject *object, QEvent *event )
{
    if ( d->usingListBox )
	ASSERT( object == d->listBox );
    else
	ASSERT( object == d->popup );

    QListBox *listB = d->listBox;
    switch ( event->type() ) {
        case Event_MouseMove: {
	    if ( !d->usingListBox )
		break;
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
	    if ( !d->usingListBox ) {
		if ( d->shortClick ) {
		    QMouseEvent tmp( Event_MouseMove, 
				     e->pos(), e->button(), e->state() ) ;
		                  // highlight item, but don't pop down:
		    QApplication::sendEvent( object, &tmp );
		    return TRUE;			// Block the event
		}
		break;
	    }
	    if ( listB->rect().contains( e->pos() ) ) {
		QMouseEvent tmp( Event_MouseButtonDblClick, 
				 e->pos(), e->button(), e->state() ) ;
		QApplication::sendEvent( object, &tmp );  // will hide popup
	    } else {
		if ( d->mouseWasInsidePopup ) {
		    popDownListBox();
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
	    if ( !d->usingListBox ) {
		if ( !d->popup->rect().contains( e->pos() ) ) {
				 // remove filter, event will take down popup:
		    d->listBox->removeEventFilter( this );
		}
		break;
	    }
	    if ( !listB->rect().contains( e->pos() ) ) {
		QPoint globalPos = listB->mapToGlobal(e->pos());
		if ( QApplication::widgetAt( globalPos, TRUE ) == this )
		    d->discardNextMousePress = TRUE; // avoid popping up again
		popDownListBox();
		return TRUE;			// Block the event;
	    }
	    break;
	    }
	default:
	    break;
    }
    return FALSE;				// Don't block the event
}



QEditableComboBox::QEditableComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    initMetaObject();
    ed = new QLineEdit( this, "this is not /bin/ed" );
    ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
    ed->installEventFilter( this );
    edEmpty = TRUE;
    
    connect( ed, SIGNAL(returnPressed()), SLOT(returnPressed()) );
    connect( this, SIGNAL(activated(int)), SLOT(translateActivate(int)) );
    connect( this, SIGNAL(highlighted(int)), SLOT(translateHighlight(int)) );
}


void QEditableComboBox::resizeEvent( QResizeEvent * )
{
    ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
}


void QEditableComboBox::translateActivate( int index )
{
    const char * t = text( index );
    if ( !text )
	return; // I don't see why but so what?
    ed->setText( t );
    emit activated( t );
}


void QEditableComboBox::translateHighlight( int index )
{
    const char * t = text( index );
    if ( text )
	emit highlighted( t );
}


void QEditableComboBox::returnPressed() {
    const char * s = ed->text();
    if ( s && qstrcmp( s, text( currentItem() ) ) ) {
	changeItem( s, currentItem() );
	emit activated( s );
    }
}


/*!
  This event filter is used to manipulate the line editor in magic
  ways.  In Qt 2.0 it will all change, until then binary compatibility
  lays down the law.
*/

bool QEditableComboBox::eventFilter( QObject *object, QEvent *event )
{
    if ( object == (QObject *)ed ) {
	if ( event && event->type() == Event_Paint ) {
	    if ( edEmpty ) {
		edEmpty = FALSE;
		ed->setText( text( currentItem() ) );
		return TRUE; // we'll get another paint event right away
	    }

	    QLineEdit * le = (QLineEdit *) object;
	    QPainter p;
	    p.begin( le );
	    p.fillRect( 0, 0, le->width(), le->height(), isEnabled() 
			? le->colorGroup().base() 
			: le->colorGroup().background() );
	    p.translate( 0, -4 ); // MAGIC! this is *_MARGIN from qlined.cpp
	    le->paintText( &p, QSize( le->width()+8, le->height()+8 ), FALSE );
	    p.end();
	    return TRUE;
	} else {
	    return FALSE;
	}
    } else {
	return QComboBox::eventFilter( object, event );
    }
}
