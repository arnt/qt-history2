/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#112 $
**
** Implementation of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlabel.h"
#include "qbitmap.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qaccel.h"
#include "qmovie.h"
#include <ctype.h>
#include "qimage.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qml.h"


class QLabelPrivate
{
};


/*!
  \class QLabel qlabel.h
  \brief The QLabel widget displays a static text or pixmap.

  \ingroup realwidgets

  A label is a static text or pixmap field.

  It can have a frame (since QLabel inherits QFrame) and a "buddy" and
  an accelerator for moving keyboard focus to the buddy.

  The contents of a label can be specified as a normal text, as a
  numeric value (which is internally converted to a text) or, as a
  pixmap.  If the label is normal text and one of the letters is
  prefixed with '&', you can also specify a \e buddy for the label:

  \code
     QLineEdit * phone = new QLineEdit( this, "phone number" );
     QLabel * phoneLabel = new QLabel( phone, "&Phone", this );
  \endcode

  In this example, keyboard focus is transferred to the label's buddy
  (the QLineEdit) when the user presses <dfn>Alt-P.</dfn> This is
  handy for many dialogs.  You can also use the setBuddy() function to
  accomplish the same means.

  A label can be aligned in many different ways. The alignment setting
  specifies where to position the contents relative to the frame
  rectangle.  See setAlignment() for a description of the alignment
  flags.

  Enabling auto-resizing make the label resize itself whenever the
  contents change.  The top left corner does not move.

  This code sets up a sunken panel with a two-line text in the bottom
  right corner:

  \code
    QLabel *label = new QLabel;
    label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    label->setText( "first line\nsecond line" );
    label->setAlignment( AlignBottom | AlignRight );
  \endcode

  Both lines are flush with the right side of the label.

  <img src=qlabel-m.gif> <img src=qlabel-w.gif>

  \sa QLineEdit QMovie
  <a href="guibooks.html#fowler">GUI Design Handbook: Label</a>
*/


/*!
  Constructs an empty label which is left-aligned, vertically centered,
  has an automatic margin and with manual resizing.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.

  \sa setAlignment(), setFrameStyle(), setMargin(), setAutoResize()
*/

QLabel::QLabel( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
    init();
}


/*!
  Constructs a label with a text. The label is left-aligned, vertically
  centered, has an automatic margin and with manual resizing.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.

  \sa setAlignment(), setFrameStyle(), setMargin(), setAutoResize()
*/

QLabel::QLabel( const QString &text, QWidget *parent, const char *name, WFlags f )
	: QFrame( parent, name, f ), ltext(text)
{
    init();
}


/*!
  Constructs a label with an accelerator key.

  The \a parent, \a name and \a f arguments are passed to the QFrame
  constructor. Note that the \a parent argument does \e not default
  to 0.

  In a dialog, you might create two data entry widgets and a label
  for each, and set up the geometry so each label is just to the left
  of its data entry widget (its "buddy"), somewhat like this:

  \code
    QLineEdit *name    = new QLineEdit( this );
    QLabel    *name_l  = new QLabel( name, "&Name:", this );
    QLineEdit *phone   = new QLineEdit( this );
    QLabel    *phone_l = new QLabel( phone, "&Phone:", this );
    // geometry management setup not shown
  \endcode

  With the code above, the focus jumps to the Name field when the user
  presses Alt-N, and to the Phone field when the user presses Alt-P.

  \sa setText(), setBuddy()
*/

QLabel::QLabel( QWidget *buddy,  const QString &text,
		QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ), ltext("")
{
    init();
    align = ShowPrefix | AlignLeft | AlignVCenter | ExpandTabs;
    setBuddy( buddy );
    setText( text );
}


/*!
  Destroys the label.
*/

QLabel::~QLabel()
{
    unsetMovie();
    delete lpixmap;
    delete d;
}


void QLabel::init()
{
    lpixmap = 0;
    lmovie = 0;
    lbuddy = 0;
    lpixmap = 0;
    accel = 0;
    align = AlignLeft | AlignVCenter | ExpandTabs;
    extraMargin= -1;
    autoresize = FALSE;
    isqml = FALSE;
    qmlDoc = 0;
    d = 0;
}


/*!
  \fn QString QLabel::text() const

  Returns the label text. This may be either plain text or a QML
  document.

  \sa setText(), setQML()
*/

/*!
  Sets the label contents to \e text, updates the optional
  accelerator and redraws the contents.

  The label resizes itself if auto-resizing is enabled.  Nothing
  happens if \e text is the same as the current label.

  \sa text(), setQML(), setPixmap(), setAutoResize()
*/

void QLabel::setText( const QString &text )
{
    unsetMovie();
    if ( !isqml && ltext == text )
	return;
    ltext = text;
    isqml = FALSE;

    if (qmlDoc ) {
	delete qmlDoc;
	qmlDoc = 0;
    }
    if ( lpixmap ) {
	delete lpixmap;
	lpixmap = 0;
    }

    if ( accel )
	accel->clear();
    int p = QAccel::shortcutKey( ltext );
    if ( p ) {
	if ( !accel )
	    accel = new QAccel( this, "accel label accel" );
	accel->connectItem( accel->insertItem( p ),
			    this, SLOT(acceleratorSlot()) );
    }

    if ( autoresize ) {
	QSize s = sizeHint();
	if ( s.isValid() && s != size() )
	    resize( s );
	else
	    repaint();
    } else {
	updateLabel();
    }

    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(),
				 new QEvent( QEvent::LayoutHint ) );
}


/*!  Clears the label.  Equivalent with setText( "" ). */

void QLabel::clear()
{
    setText( "" );
}


/*!
  \fn QPixmap *QLabel::pixmap() const
  Returns the label pixmap.
  \sa setPixmap()
*/

/*!
  Sets the label contents to \e pixmap and redraws the contents.

  If the label has a buddy, the accelerator is disabled since the
  pixmap doesn't contain any suitable character.

  The label resizes itself if auto-resizing is enabled.	 Nothing
  happens if \e pixmap is the same as the current label.

  \sa pixmap(), setText(), setQML(), setAutoResize()
*/

void QLabel::setPixmap( const QPixmap &pixmap )
{
    unsetMovie();
    int w, h;
    if ( lpixmap ) {
	w = lpixmap->width();
	h = lpixmap->height();
    } else {
	lpixmap = new QPixmap;
	CHECK_PTR( lpixmap );
	w = h = -1;
    }
    bool sameSize = w == lpixmap->width() && h == lpixmap->height();
    *lpixmap = pixmap;
    if ( lpixmap->depth() == 1 && !lpixmap->mask() )
	lpixmap->setMask( *((QBitmap *)lpixmap) );
    if ( !ltext.isNull() )
	ltext = QString::null;
    if ( autoresize && !sameSize )
	adjustSize();
    else
	updateLabel();
    if ( accel )
	accel->clear();
}


/*!
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  If the label has a buddy, the accelerator is disabled since the
  number doesn't contain any suitable character.

  The label resizes itself if auto-resizing is enabled.	 Nothing
  happens if \e num reads the same as the current label.

  \sa setAutoResize()
*/

void QLabel::setNum( int num )
{
    QString str;
    str.setNum( num );
    if ( str != ltext ) {
	setText( str );
	if ( autoresize )
	    adjustSize();
	else
	    updateLabel();
    }
}

/*!
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  If the label has a buddy, the accelerator is disabled since the
  number doesn't contain any suitable character.

  The label resizes itself if auto-resizing is enabled.

  \sa setAutoResize()
*/

void QLabel::setNum( double num )
{
    QString str;
    str.sprintf( "%g", num );
    if ( str != ltext ) {
	setText( str );
	if ( autoresize )
	    adjustSize();
	else
	    updateLabel();
    }
}


/*!
  \fn int QLabel::alignment() const
  Returns the alignment setting.

  The default alignment is <code>AlignLeft | AlignVCenter |
  ExpandTabs</code> if the label doesn't have a buddy and
  <code>AlignLeft | AlignVCenter | ExpandTabs | ShowPrefix </code> if
  the label has a buddy.

  \sa setAlignment()
*/

/*!
  Sets the alignment of the label contents and redraws itself.

  The \e alignment is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c ExpandTabs expands tabulators.
  <li> \c WordBreak enables automatic word breaking.
  </ul>

  If the label has a buddy, \c ShowPrefix is forced to TRUE.

  \sa alignment() setBuddy() setText()
*/

void QLabel::setAlignment( int alignment )
{
    if ( lbuddy )
	align = alignment | ShowPrefix;
    else
	align = alignment;
    updateLabel();
}


/*!
  \fn int QLabel::margin() const

  Returns the margin of the label.

  \sa setMargin()
*/

/*!
  Sets the margin of the label to \e margin pixels.

  The margin applies to the left edge if alignment() is \c AlignLeft,
  to the right edge if alignment() is \c AlignRight, to the top edge
  if alignment() is \c AlignTop, and to to the bottom edge if
  alignment() is \c AlignBottom.

  If \e margin is negative (as it is by default), the label computes the
  margin as follows: If the \link frameWidth() frame width\endlink is zero,
  the effective margin becomes 0. If the frame style is greater than zero,
  the effective margin becomes half the width of the "x" character (of the
  widget's current \link font() font\endlink.

  Setting a non-negative margin gives the specified margin in pixels.

  \sa margin(), frameWidth(), font()
*/

void QLabel::setMargin( int margin )
{
    extraMargin = margin;
}


/*!
  \fn bool QLabel::autoResize() const
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/

/*!
  Enables auto-resizing if \e enable is TRUE, or disables it if \e
  enable is FALSE.

  When auto-resizing is enabled, the label will resize itself whenever the
  contents change.  The top left corner is not moved.

  \sa autoResize(), adjustSize()
*/

void QLabel::setAutoResize( bool enable )
{
    if ( (bool)autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}


/*!
  Returns a size which fits the contents of the label.

  \bug Does not work well with the WordBreak flag
*/

QSize QLabel::sizeHint() const
{
    QPainter p( this );
    QRect br;
    QPixmap *pix = pixmap();
    QMovie *mov = movie();
    if ( pix ) {
	br = QRect( 0, 0, pix->width(), pix->height() );
    } else if ( mov ) {
	br = QRect( 0, 0, mov->framePixmap().width(),
		mov->framePixmap().height() );
    } else if (isqml ){
	if ( qmlDoc ) {
	    QRect cr = contentsRect();
	    int m = margin();
	    if ( m < 0 ) {
		if ( frameWidth() > 0 )
		    m = p.fontMetrics().width("x")/2;
		else
		    m = 0;
	    }
	    if ( m > 0 ) {
		if ( align & AlignLeft )
		    cr.setLeft( cr.left() + m );
		if ( align & AlignRight )
		    cr.setRight( cr.right() - m );
		if ( align & AlignTop )
		    cr.setTop( cr.top() + m );
		if ( align & AlignBottom )
		    cr.setBottom( cr.bottom() - m );
	    }
	    if ( qmlDoc->width() != cr.width() ) {
		QPainter p( this );
		qmlDoc->setWidth( &p, cr.width() );
	    }
	    br = QRect( 0, 0, qmlDoc->width(), qmlDoc->height() );
	}
    }
    else {
	br = p.boundingRect( 0,0, 1000,1000, alignment(), text() );
	// adjust so "Yes" and "yes" will have the same height
	int h = fontMetrics().lineSpacing();
	br.setHeight( ((br.height() + h-1) / h)*h - fontMetrics().leading() );
    }
    int m  = 2*margin();
    int fw = frameWidth();
    if ( m < 0 ) {
	if ( fw > 0 )
	    m = p.fontMetrics().width( "x" );
	else
	    m = 0;
    }
    int w = br.width()	+ m + 2*fw;
    int h = br.height() + m + 2*fw;

    return QSize( w, h );
}


/*!
  Specifies that this widget may stretch horizontally and
  vertically beyond the sizeHint().
*/

QSizePolicy QLabel::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, isqml );
}


/*!
  Draws the label contents using the painter \e p.
*/

void QLabel::drawContents( QPainter *p )
{
    QRect cr = contentsRect();
    int m = margin();
    if ( m < 0 ) {
	if ( frameWidth() > 0 )
	    m = p->fontMetrics().width("x")/2;
	else
	    m = 0;
    }
    if ( m > 0 ) {
	if ( align & AlignLeft )
	    cr.setLeft( cr.left() + m );
	if ( align & AlignRight )
	    cr.setRight( cr.right() - m );
	if ( align & AlignTop )
	    cr.setTop( cr.top() + m );
	if ( align & AlignBottom )
	    cr.setBottom( cr.bottom() - m );
    }

    QMovie *mov = movie();
    if ( mov ) {
	// ### should add movie to qDrawItem
 	QRect r = qItemRect( p, style(),
 			cr.x(), cr.y(), cr.width(), cr.height(),
 			align, isEnabled(), &(mov->framePixmap()), ltext );
	// ### could resize movie frame at this point
	p->drawPixmap(r.x(), r.y(), mov->framePixmap() );
    }
    else if ( isqml ) {
	if (qmlDoc) {
	    qmlDoc->setWidth(p, cr.width() );
	    qmlDoc->draw(p, cr.x(), cr.y(), cr, colorGroup(), 0);
	}
    } else {
	// ordinary text label
	qDrawItem( p, style(), cr.x(), cr.y(), cr.width(), cr.height(),
		   align, colorGroup(), isEnabled(),
		   lpixmap, ltext );
    }
}


void QLabel::setAutoMask(bool b)
{
    if (b)
	setBackgroundMode( PaletteText );
    else
	setBackgroundMode( PaletteBackground );
    QFrame::setAutoMask(b);
}

/*!
  Draws the label contents mask using the painter \e p.
  Used only in transparent mode.

  \sa QWidget::setAutoMask();
*/
void QLabel::drawContentsMask( QPainter *p )
{
    QRect cr = contentsRect();
    int m = margin();
    if ( m < 0 ) {
	if ( frameWidth() > 0 )
	    m = p->fontMetrics().width("x")/2;
	else
	    m = 0;
    }
    if ( m > 0 ) {
	if ( align & AlignLeft )
	    cr.setLeft( cr.left() + m );
	if ( align & AlignRight )
	    cr.setRight( cr.right() - m );
	if ( align & AlignTop )
	    cr.setTop( cr.top() + m );
	if ( align & AlignBottom )
	    cr.setBottom( cr.bottom() - m );
    }

    QMovie *mov = movie();
    if ( mov ) {
	// ### should add movie to qDrawItem
	QRect r = qItemRect( p, style(),
			cr.x(), cr.y(), cr.width(), cr.height(),
			align, isEnabled(), &(mov->framePixmap()), ltext );
	// ### could resize movie frame at this point
	QPixmap pm = mov->framePixmap();
	if ( pm.mask() ) {
	    p->setPen( color1);
	    p->drawPixmap(r.x(), r.y(), *pm.mask() );
	}
	else
	    p->fillRect( r, color1 );
	return;
    }

    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);

    QBitmap bm;
    if (lpixmap) {
	if (lpixmap->mask()) {
	    bm = *lpixmap->mask();
	}
	else {
	    bm.resize( lpixmap->size() );
	    bm.fill(color1);
	}
    }

    qDrawItem( p, style(), cr.x(), cr.y(), cr.width(), cr.height(),
	       align, g, isEnabled(), bm.isNull()?0:&bm, ltext );
}


/*!
  Updates the label, not the frame.
*/

void QLabel::updateLabel()
{
    repaint(contentsRect());
    if ( autoMask() )
	updateMask();
}


/*!
  Internal slot, used to set focus for accelerator labels.
*/

void QLabel::acceleratorSlot()
{
    if ( !lbuddy )
	return;
    QWidget * w = lbuddy;
    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( !w->hasFocus() &&
	 w->isEnabledToTLW() &&
	 w->isVisibleToTLW() &&
	 w->focusPolicy() != NoFocus )
	w->setFocus();
}


/*!
  Internal slot, used to clean up if the buddy widget dies.
*/

void QLabel::buddyDied() // I can't remember if I cried.
{
    lbuddy = 0;
}


/*!
  Sets the buddy of this label to \a buddy.

  When the user presses the accelerator key indicated by this label,
  the keyboard focus is transferred to the label's buddy.

  \sa label(), setText()
*/

void QLabel::setBuddy( QWidget *buddy )
{
    if ( buddy )
	setAlignment( alignment() | ShowPrefix );
    else
	setAlignment( alignment() & ~ShowPrefix );

    if ( lbuddy )
	disconnect( lbuddy, SIGNAL(destroyed()), this, SLOT(buddyDied()) );

    lbuddy = buddy;

    if ( !lbuddy )
	return;

    int p = QAccel::shortcutKey( ltext );
    if ( p ) {
	if ( !accel )
	    accel = new QAccel( this, "accel label accel" );
	accel->connectItem( accel->insertItem( p ),
			    this, SLOT(acceleratorSlot()) );
    }

    connect( lbuddy, SIGNAL(destroyed()), this, SLOT(buddyDied()) );
}


/*!
  Returns the buddy of this label.
*/

QWidget * QLabel::buddy() const
{
    return lbuddy;
}


void QLabel::movieUpdated(const QRect& rect)
{
    QMovie *mov = movie();
    if ( mov && !mov->isNull() ) {
	QRect r = contentsRect();
	r = qItemRect( 0, style(), r.x(), r.y(), r.width(), r.height(),
		       align, isEnabled(), &(mov->framePixmap()), ltext );
	r.moveBy(rect.x(), rect.y());
	r.setWidth(QMIN(r.width(), rect.width()));
	r.setHeight(QMIN(r.height(), rect.height()));
	repaint( r, mov->framePixmap().mask() != 0 );
	if ( autoMask() )
	    updateMask();
    }
}

void QLabel::movieResized(const QSize& size)
{
    if (autoresize) adjustSize();
    movieUpdated(QRect(QPoint(0,0),size));
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );
}

/*!
  Sets a QMovie to display in the label, or removes any existing movie
  if the given movie QMovie::isNull().

  Any current pixmap or text label is cleared.

  If the label has a buddy, the accelerator is disabled since the
  movie doesn't contain any suitable character.
*/
void QLabel::setMovie( const QMovie& movie )
{
    unsetMovie();

    if ( movie.isNull() ) {
	return;
    } else {
	if ( !lmovie )
	    lmovie = new QMovie;
	*lmovie = movie;
    }

    delete accel;
    accel = 0;
    delete lpixmap;
    lpixmap = 0;
    ltext = QString::null;

    if ( lmovie ) {
	lmovie->connectResize(this, SLOT(movieResized(const QSize&)));
	lmovie->connectUpdate(this, SLOT(movieUpdated(const QRect&)));
    }
}

/*
  Unset the movie.
*/
void QLabel::unsetMovie()
{
    if ( lmovie ) {
	lmovie->disconnectResize(this, SLOT(movieResized(const QSize&)));
	lmovie->disconnectUpdate(this, SLOT(movieUpdated(const QRect&)));
	delete lmovie;
	lmovie = 0;
    }
}

/*!
  Returns the QMovie currently displaying in the label, or 0
  if none has been set.
*/
QMovie* QLabel::movie() const
{
    return lmovie;
}

/*!
  Reimplemented for QML labels
*/
int QLabel::heightForWidth(int w) const
{
    if (isqml && qmlDoc ) {
	QPainter p( this );
	w -= 2*frameWidth();
	int m = margin();
	if ( m < 0 ) {
	    if ( frameWidth() > 0 )
		m = p.fontMetrics().width("x")/2;
	    else
		m = 0;
	}
	int add = 0;
	if ( m > 0 ) {
	    if ( align & AlignLeft )
		w -= m;
	    if ( align & AlignRight )
		w -= m;
	    if ( align & AlignTop )
		add = m;
	    if ( align & AlignBottom )
		add = m;
	}
	qmlDoc->setWidth(&p, w);
	return qmlDoc->height() + 2*frameWidth() + add;
    }
    return QWidget::heightForWidth(w);
}


/*!
  Sets the label contents to QML document \e qml and
  redraws the contents.

  The label resizes itself if auto-resizing is enabled. Nothing
  happens if \e qml is the same as the current label.

  Note that a QLabel is only useful for rather small QML documents
  with one or maximal two lines of text.  If you need to display
  larger documents, a QMLView is the widget of choice. It will flicker
  less on resize and can also provide a scrollbar if necessary.

  \sa text(), setText(), setPixmap(), setAutoResize(), QMLView
*/
void QLabel::setQML( const QString & qml )
{
    unsetMovie();
    if ( qml && ltext == qml )
	return;
    ltext = qml;
    isqml = TRUE;
    if ( lpixmap ) {
	delete lpixmap;
	lpixmap = 0;
    }
    delete qmlDoc;
    qmlDoc = new QMLSimpleDocument( ltext, this );
    if ( accel )
	accel->clear();
    if ( autoresize ) {
	QSize s = sizeHint();
	if ( s.isValid() && s != size() )
	    resize( s );
	else
	    repaint();
    } else {
	updateLabel();
    }
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint ) );
}
