/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcombobox.cpp#61 $
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
#include <limits.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qcombobox.cpp#61 $");


/*!
  \class QComboBox qcombo.h
  \brief The QComboBox widget is a combined button and popup list.

  \ingroup realwidgets

  A combo box may be defined as a selection widget which displays the
  current selection, and which can pop up a list of possible
  selections.  Some combo boxes also allow the user to select
  arbitrary strings, using a line editor.

  Since combo boxes occupy little screen space and always display the
  current selection, they are very well suited to displaying and
  selecting modes (such as font family and size): The user can always
  see what mode he/she is in, and the majority of the screen space is
  available for real work.

  QComboBox supports three different appearances: Motif 1.x, Motif 2.0
  and Windows 95.  In Motif 1.x, a combo box was called XmOptionMenu,
  presumably because OSF's naming policy suffers from the NIH
  syndrome.  In Motif 2.0, OSF introduced an improved combo box and
  named that XmComboBox.  QComboBox provides both.



  A combo box emits two signals, activated() and highlighted(), when a
  new item has been activated (selected) or highlighted (set to
  current).

  

*/


/*! \fn void QComboBox::activated( int index )
  This signal is emitted when a new item has been activated (selected).
  The \e index is the position of the item in the popup list.
*/

/* \fn void QComboBox::activated( const char * )

  This signal is emitted when a new item has been activated
  (selected). The argument is the activated string.

  You can also use activated(int) signal, but be aware that its
  argument meaningful only for selected strings, not for typed
  strings.
*/

/*! \fn void QComboBox::highlighted( int index )
  This signal is emitted when a new item has been set to current.
  The \e index is the position of the item in the popup list.
*/

/* \fn void QComboBox::highlighted( const char * )

  This signal is emitted when a new item has been highlighted. The
  argument is the highlighted string.

  You can also use highlighted(int) signal.
*/

struct QComboData
{
    int		current;
    int		maxCount;
    QLineEdit * ed;  // /bin/ed rules!
    QComboBox::Policy p;
    bool	edEmpty;
    bool	usingListBox;
    bool	autoresize;
    bool	poppedUp;
    bool	mouseWasInsidePopup;
    bool	arrowPressed;
    bool	arrowDown;
    bool	discardNextMousePress;
    bool	shortClick;
    union {
	QPopupMenu *popup;
	QListBox   *listBox;
    };
};


bool QComboBox::getMetrics( int *dist, int *buttonW, int *buttonH ) const
{
    if ( d->usingListBox && style() == WindowsStyle ) {
	QRect r  = arrowRect();
	*buttonW = r.width();
	*buttonH = r.height();
	*dist    = 4;
    } else if ( d->usingListBox ) {
	*dist = 6;
	*buttonW = 16;
	*buttonH = 18;
    } else {
	*dist     = 8;
	*buttonH  = 7;
	*buttonW  = 11;
    }
    return TRUE;
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

  This constructor creates a popup menu if the program uses Motif look
  and feel; this is compatible with Motif 1.x.
*/

QComboBox::QComboBox( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    d		             = new QComboData;
    if ( style() == WindowsStyle ) {
	d->listBox           = new QListBox( 0, 0, WType_Popup );
	d->listBox->setAutoScrollBar( FALSE );
	d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
	d->listBox->setLineWidth( 1 );
	d->listBox->resize( 100, 10 );
	
	d->usingListBox      = TRUE;
	connect( d->listBox, SIGNAL(selected(int)),
		             SLOT(internalActivate(int)) );
	connect( d->listBox, SIGNAL(highlighted(int)),
		             SLOT(internalHighlight(int)));
    } else {
	d->popup             = new QPopupMenu;
	d->usingListBox      = FALSE;
	connect( d->popup, SIGNAL(activated(int)),
		           SLOT(internalActivate(int)) );
	connect( d->popup, SIGNAL(highlighted(int)),
		           SLOT(internalHighlight(int)) );
    }
    d->edEmpty               = TRUE;
    d->ed                    = 0;
    d->current               = 0;
    d->maxCount              = INT_MAX;
    d->p = AtEnd;
    d->autoresize            = FALSE;
    d->poppedUp              = FALSE;
    d->arrowDown             = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick            = FALSE;

    setFocusPolicy( TabFocus );
}


/*!

  Constructs a combo box with a maximum size and either Motif 2.0 or
  Windows look and feel.

  \a size is the maximum number of lines the list box will show at
  once.  If the contents are any larger, it will scroll.
*/


QComboBox::QComboBox( bool rw, QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    d = new QComboData;
    d->listBox = new QListBox( 0, 0, WType_Popup );
    d->listBox->setAutoScrollBar( FALSE );
    d->listBox->setFrameStyle( QFrame::Box | QFrame::Plain );
    d->listBox->setLineWidth( 1 );
    d->listBox->resize( 100, 10 );
	
    d->usingListBox = TRUE;
    connect( d->listBox, SIGNAL(selected(int)),
	     SLOT(internalActivate(int)) );
    connect( d->listBox, SIGNAL(highlighted(int)),
	     SLOT(internalHighlight(int)));

    d->edEmpty = TRUE;
    d->current = 0;
    d->maxCount = INT_MAX;
    d->p = AtEnd;
    d->autoresize = FALSE;
    d->poppedUp = FALSE;
    d->arrowDown = FALSE;
    d->discardNextMousePress = FALSE;
    d->shortClick = FALSE;

    if ( style() == MotifStyle )
	setFocusPolicy( TabFocus );

    if ( rw ) {
	d->ed = new QLineEdit( this, "this is not /bin/ed" );
	if ( style() == WindowsStyle )
	    d->ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
	else
	    d->ed->setGeometry( 3, 3, width() - 3 - 3 - 21, height() - 3 - 3 );
	d->ed->installEventFilter( this );
    
	connect( d->ed, SIGNAL(returnPressed()), SLOT(returnPressed()) );
    } else {
	d->ed = 0;
    }

    
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

  If the combo box is writable, the pixmap is not inserted.
*/

void QComboBox::insertItem( const QPixmap &pixmap, int index )
{
    if ( d->ed )
	return;

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
  Replaces the item at position \e index with a pixmap, unless the
  combo box is writable.

  \sa insertItem()
*/

void QComboBox::changeItem( const QPixmap &im, int index )
{
    if ( d->ed != 0 || !checkIndex( "changeItem", count(), index ) )
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
    return QSize( 4 + 4 + maxW + 2*dist + buttonW, maxH + 5 + 5 );
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

    const char *t = text( index );
    if ( !t )
	return;					// shouldn't happen
    if ( d->ed )
	d->ed->setText( t );
    emit activated( t );
}

/*!
  \internal
  Receives highlighted signals from an internal popup list and emits
  the highlighted() signal.
*/

void QComboBox::internalHighlight( int index )
{
    emit highlighted( index );
    const char *t = text( index );
    if ( t )
	emit highlighted( t );
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
  Handles resize events for the combo box.
*/

void QComboBox::resizeEvent( QResizeEvent * )
{
    if ( !d->ed )
	return;
    else if ( style() == WindowsStyle )
	d->ed->setGeometry( 2, 2, width() - 2 - 2 - 16, height() - 2 - 2 );
    else
	d->ed->setGeometry( 3, 3, width() - 3 - 3 - 21, height() - 3 - 3 );
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

    if ( !d->usingListBox ) {		// motif 1.x style
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

	if ( hasFocus() )
	    p.drawRect( xPos - 5, 4, width() - xPos + 1 , height() - 8 );

    } else if ( style() == MotifStyle ) { // motif 2.0 style
	int x1, y1;
	QPointArray l;

	x1 = width() - 2 - 5 - 15;

	y1 = height()/2 - 9;

	// light matter
	l.setPoints( 20,
		     x1,y1+17, x1,y1+17-2, 
		     x1,y1+17-2, x1+15, y1+17-2,
		     x1+1,y1, x1+14,y1,
		     x1+1,y1+1, x1+12,y1+1,
		     x1+2,y1+2, x1+2,y1+3,
		     x1+3,y1+2, x1+3,y1+5,
		     x1+4,y1+4, x1+4,y1+7,
		     x1+5,y1+6, x1+5,y1+9,
		     x1+6,y1+8, x1+6,y1+11,
		     x1+7,y1+10, x1+7,y1+11 );
	p.setPen( colorGroup().light() );
	p.drawLineSegments( l );

	// dark matter
	l.setPoints( 18,
		     x1+8,y1+10, x1+8,y1+11,
		     x1+9,y1+8, x1+9,y1+11,
		     x1+10,y1+6, x1+10,y1+9,
		     x1+11,y1+4, x1+11,y1+7,
		     x1+12,y1+2, x1+12,y1+5,
		     x1+13,y1+1, x1+13,y1+3,
		     x1+14,y1+1, x1+14,y1+1,
		     x1+1,y1+17, x1+15,y1+17,
		     x1+15,y1+16, x1+15,y1+16 );
	p.setPen( colorGroup().dark() );
	p.drawLineSegments( l );

	QFontMetrics fm = p.fontMetrics();
	QRect clip( 4, 2, x1 - 2 - 4, height() - 4 );
	const char *str = d->listBox->text( d->current );
	p.setPen( colorGroup().foreground() );
	p.drawText( clip, AlignCenter | SingleLine, str );
	qDrawShadePanel( &p, rect(), g, FALSE, 2 );

	if ( hasFocus() )
	    p.drawRect( x1 - 2, y1 - 2, 20, 22 );

    } else {				// windows 95 style
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
    if ( style() == WindowsStyle ) {
	popup();
	if ( arrowRect().contains( e->pos() ) ) {
	    d->arrowPressed = TRUE;
	    d->arrowDown    = TRUE;
	    repaint( FALSE );
	} else {
	    d->arrowPressed = FALSE;
	}
    } else if ( d->usingListBox ) {
	if ( e->pos().x() >= width() - 23 ) {
	    popup();
	    QTimer::singleShot( 200, this, SLOT(internalClickTimeout()));
	    d->shortClick = TRUE;
	}
    } else {
	popup();
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
  Handles key press events for the combo box.

  In Motif style, up and down change the selected item and both enter
  and return pops up the list.  In Windows style, all four arrow keys
  change the selected item, and Space pops up the list.
*/

void QComboBox::keyPressEvent( QKeyEvent *e )
{
    int c;
    switch ( e->key() ) {
    case Key_Return:
    case Key_Enter:
	if ( style() == MotifStyle ) {
	    e->ignore();
	    break;
	}
	// fall through for motif
	e->accept();
	popup();
	break;
    case Key_Space:
	e->accept();
	popup();
	break;
    case Key_Left:
	if ( style() != WindowsStyle ) {
	    e->ignore();
	    break;
	}
	// fall through for windows
    case Key_Up:
	c = currentItem();
	if ( c > 0 )
	    setCurrentItem( c-1 );
	else
	    setCurrentItem( count()-1 );
	e->accept();
	break;
    case Key_Right:
	if ( style() == MotifStyle ) {
	    e->ignore();
	    break;
	}
	// fall through for windows
    case Key_Down:
c	 = currentItem();
	if ( ++c < count() )
	    setCurrentItem( c );
	else
	    setCurrentItem( 0 );
	e->accept();
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

/*!
  This event filter is used to manipulate the line editor in magic
  ways.  In Qt 2.0 it will all change, until then binary compatibility
  lays down the law.

   The event filter steals events from the popup or listbox
   when they are popped up. It makes the popup stay up after a short click
   in motif style. In windows style it toggles the arrow button of the
   combo box field, and activates an item and takes down the listbox when
   the mouse button is released.
*/

bool QComboBox::eventFilter( QObject *object, QEvent *event )
{
    if ( !event )
	return TRUE;
    else if ( object == (QObject *)(d->ed) ) {
	if ( event->type() == Event_Paint ) {
	    if ( d->edEmpty ) {
		d->edEmpty = FALSE;
		d->ed->setText( text( currentItem() ) );
		return TRUE; // we'll get another paint event right away
	    }

	    if ( style() == WindowsStyle ) {
		QLineEdit * le = (QLineEdit *) object;
		QPainter p;
		p.begin( le );
		p.fillRect( 0, 0, le->width(), le->height(), isEnabled() 
			    ? le->colorGroup().base() 
			    : le->colorGroup().background() );
		p.translate( 0, -4 ); // MAGIC! *_MARGIN from qlined.cpp
		le->paintText( &p, QSize( le->width()+8, le->height()+8 ),
			       FALSE );
		p.end();
		return TRUE;
	    }
	} else if ( event->type() == Event_KeyPress &&
		    style() == MotifStyle ) {
	    int c;
	    switch( ((QKeyEvent *)event)->key() ) {
	    case Key_Up:
		c = currentItem();
		if ( c > 0 )
		    setCurrentItem( c-1 );
		else
		    setCurrentItem( count()-1 );
		return TRUE;
		break;
	    case Key_Down:
		c = currentItem();
		if ( ++c < count() )
		    setCurrentItem( c );
		else
		    setCurrentItem( 0 );
		return TRUE;
		break;
	    default:
		break;
	    }
	}
    } else if ( d->usingListBox && object == d->listBox ) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch( event->type() ) {
        case Event_MouseMove:
	    if ( !d->mouseWasInsidePopup  ) {
		QPoint pos = e->pos();
		if ( d->listBox->rect().contains( pos ) )
		    d->mouseWasInsidePopup = TRUE;
		// Check if arrow button should toggle
		// this applies only to windows style
		if ( d->arrowPressed ) {
		    QPoint comboPos;
		    comboPos = mapFromGlobal( d->listBox->mapToGlobal(pos) );
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
	case Event_MouseButtonRelease:
	    if ( d->listBox->rect().contains( e->pos() ) ) {
		QMouseEvent tmp( Event_MouseButtonDblClick, 
				 e->pos(), e->button(), e->state() ) ;
		// will hide popup
		QApplication::sendEvent( object, &tmp );
		return TRUE;
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
        case Event_MouseButtonDblClick:
        case Event_MouseButtonPress:
	    if ( !d->listBox->rect().contains( e->pos() ) ) {
		QPoint globalPos = d->listBox->mapToGlobal(e->pos());
		if ( QApplication::widgetAt( globalPos, TRUE ) == this ) {
		    d->discardNextMousePress = TRUE;
		    // avoid popping up again
		}
		popDownListBox();
		return TRUE;
	    }
	    break;
	default:
	    break;
	}
    } else if ( !d->usingListBox && object == d->popup ) {
	QMouseEvent *e = (QMouseEvent*)event;
	switch ( event->type() ) {
        case Event_MouseButtonRelease:
	    if ( d->shortClick ) {
		QMouseEvent tmp( Event_MouseMove, 
				 e->pos(), e->button(), e->state() ) ;
		// highlight item, but don't pop down:
		QApplication::sendEvent( object, &tmp );
		return TRUE;
	    }
	    break;
        case Event_MouseButtonDblClick:
        case Event_MouseButtonPress:
	    if ( !d->popup->rect().contains( e->pos() ) ) {
		// remove filter, event will take down popup:
		d->listBox->removeEventFilter( this );
	    }
	    break;
	    if ( !d->listBox->rect().contains( e->pos() ) ) {
		QPoint globalPos = d->listBox->mapToGlobal(e->pos());
		if ( QApplication::widgetAt( globalPos, TRUE ) == this )
		    d->discardNextMousePress = TRUE; // avoid popping up again
		popDownListBox();
		return TRUE;
	    }
	    break;
	default:
	    break;
	}
    }
    return FALSE;
}


/*!
  Returns the current maximum size of the combo box.  By default,
  there is no limit, so this function returns UINT_MAX.

  \sa setSizeLimit() count()
*/

int QComboBox::sizeLimit() const
{
    return d ? d->maxCount : INT_MAX;
}


/*!
  Sets the maximum number of items the combo box can hold to \a count.

  If \a count is smaller than the current number of items, the list is
  truncated at the end.  There is no limit by default.

  \sa sizeLimit() count()
*/

void QComboBox::setSizeLimit( int count )
{
    int l = this->count();
    while( --l > count )
	removeItem( l );
    d->maxCount = count;
}

/*!
  Returns the current insertion policy of the combo box.

  \sa setPolicy()
*/

QComboBox::Policy QComboBox::policy() const
{
    return d->p;
}


/*! 
  Sets the insertion policy of the combo box to \a policy.

  The insertion policy governs where items typed in by the user are
  inserted in the list.  The possible values are <ul> <li> \c
  NoInsertion: Strings typed by the user aren't inserted anywhere <li>
  \c AtBeginning: Strings typed by the user are inserted before the
  first item in the list <li> AtCurrent: Strings typed by the user
  replace the last selected item <li> AtEnd: Strings typed by the user
  are inserted at the end of the list. </ul>

  The default insertion policy is \c AtEnd.

  \sa policy()
*/

void QComboBox::setPolicy( QComboBox::Policy policy )
{
    d->p = policy;
}



/*!
  Internal slot to keep the line editor up to date.
*/

void QComboBox::returnPressed()
{
    const char * s = d->ed->text();
    if ( s ) {
	switch ( policy() ) {
	case AtCurrent:
	    if ( qstrcmp( s, text( currentItem() ) ) ) {
		changeItem( s, currentItem() );
		emit activated( s );
	    }
	    break;
	case AtBeginning:
	    if ( count() < d->maxCount )
		insertItem( s, 0 );
	    emit activated( s );
	    break;
	case AtEnd:
	    if ( count() < d->maxCount )
		insertItem( s );
	    emit activated( s );
	    break;
	case NoInsertion:
	    emit activated( s );
	    break;
	}
    }
}
