/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#124 $
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
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
#include <math.h>
#include "qimage.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qsimplerichtext.h"
#include "qstylesheet.h"


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

  <img src=qlabel-m.png> <img src=qlabel-w.png>

  A label may also be used to provide a slightly larger amount of
  information, for example a few lines help text in a dialog. For this
  reason, QLabel also supports rich text rendering. The available
  styles are defined in the default stylesheet
  QStyleSheet::defaultSheet(). Usually the label autodetects from the
  set text whether rich text rendering is required. The format,
  however, can also be specified directly with setTextFormat(). Note
  that buddies will not work yet with rich text labels since there's
  not way to specify the accelerator (yet).

  \sa QLineEdit QMovie QTextView
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
	: QFrame( parent, name, f )
{
    init();
    setText( text );
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
    : QFrame( parent, name, f ), ltext(QString::fromLatin1(""))
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
    delete doc;
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
    textformat = Qt::AutoText;
    doc = 0;
    d = 0;
}


/*!
  \fn QString QLabel::text() const

  Returns the label text. This may be either plain text or a small
  rich text document.

  \sa textFormat(), setText(), setTextFormat()
*/

/*!
  Sets the label contents to \a text, updates the optional
  accelerator and redraws the contents.

  The label resizes itself if auto-resizing is enabled.  Nothing
  happens if \a text is the same as the current label.

  \a text may be interpreted either as plain text or as rich text,
  depending on the textFormat(). The default setting is \c AutoText,
  i.e. the label autodetects the format from \a text.

  Note that a label is only useful for rather small documents with one
  or maximal two lines of text.  If you need to display larger
  documents, a QTextView is the widget of choice. It will flicker less
  on resize and can also provide a scrollbar if necessary.

  \sa text(), setTextFormat(), setPixmap(), setAutoResize(), QTextView
*/

void QLabel::setText( const QString &text )
{
    unsetMovie();
    if ( ltext == text )
	return;
    ltext = text;

    if (doc ) {
	delete doc;
	doc = 0;
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

    if ( textformat == RichText ||
	 ( textformat == AutoText
	   && QStyleSheet::mightBeRichText( ltext ) ) )
	doc = new QSimpleRichText( ltext, font() );

    if ( autoresize ) {
	QSize s = sizeHint();
	if ( s.isValid() && s != size() )
	    resize( s );
	else
	    repaint();
    } else {
	updateLabel();
    }
    updateGeometry();
}


/*!  Clears the label.  Equivalent with setText( "" ). */

void QLabel::clear()
{
    setText( QString::fromLatin1("") );
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

  \sa pixmap(), setText(), setTextFormat(), setAutoResize()
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
  Returns the size that will be used if the width of the label is
  \a w. If \a w is -1, the sizeHint is returned.
*/

QSize QLabel::sizeForWidth( int w ) const
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
    } else if (doc ){
	if ( w < 0 ) {
	    w = 1000;
	    doc->setWidth(&p,w);
	    w = int(2*sqrt(doc->height()/3*doc->widthUsed()));
	} else {
	    w -= 2*frameWidth();
	}
	doc->setWidth(&p, w);
	br = QRect( 0, 0, doc->widthUsed(), doc->height() );
    } else {
	bool tryWidth = w < 0 && align&WordBreak;
	QFontMetrics fm = fontMetrics();
	if ( tryWidth)
	    w = fm.width( 'x' ) * 80;
	else if ( w < 0 )
	    w = 2000;
	br = p.boundingRect( 0,0, w ,2000, alignment(), text() );
	if ( tryWidth && br.height() < 4*fm.lineSpacing() && br.width() > w/2 )
	    	br = p.boundingRect( 0,0, w/2 ,2000, alignment(), text() );
	if ( tryWidth && br.height() < 2*fm.lineSpacing() && br.width() > w/4)
	    br = p.boundingRect( 0,0, w/4 ,2000, alignment(), text() );
	// adjust so "Yes" and "yes" will have the same height
	int h = fontMetrics().lineSpacing();
	br.setHeight( ((br.height() + h-1) / h)*h - fontMetrics().leading() );
    }
    int m  = 2*margin();
    int fw = frameWidth();
    if ( m < 0 ) {
	if ( fw > 0 )
	    m = p.fontMetrics().width( 'x' );
	else
	    m = 0;
    }
    int wid = br.width() + m + 2*fw;
    int hei = br.height() + m + 2*fw;

    return QSize( wid, hei );
}


/*!
  Reimplemented for rich text labels and WordBreak
*/
int QLabel::heightForWidth(int w) const
{
    if (doc  || align & WordBreak ) {
	return sizeForWidth( w ).height();
    }
    return QWidget::heightForWidth(w);
}



/*!
  Returns a size which fits the contents of the label.

  \bug Does not work well with the WordBreak flag, use heightForWidth().
*/

QSize QLabel::sizeHint() const
{
    return sizeForWidth( -1 );
}


/*!
  Specifies that this widget may stretch horizontally and
  vertically beyond the sizeHint().
*/

QSizePolicy QLabel::sizePolicy() const
{
    if ( doc || align & WordBreak )
	return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred,
			    TRUE );
    else
	return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum,
			    FALSE );
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
	    m = p->fontMetrics().width('x')/2;
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
    else if ( doc ) {
	doc->setWidth(p, cr.width() );
	int rw = doc->widthUsed();
	int rh = doc->height();
	int xo = 0;
	int yo = 0;
	if ( align & AlignVCenter )
	    yo = (cr.height()-rh)/2;
	else if ( align & AlignBottom )
	    yo = cr.height()-rh;
	if ( align & AlignRight )
	    xo = cr.width()-rw;
	else if ( align & AlignHCenter )
	    xo = (cr.width()-rw)/2;
	if ( style() == WindowsStyle && !isEnabled() ) {
	    QColorGroup cg = colorGroup();
	    cg.setColor( QColorGroup::Text, cg.light() );
	    doc->draw(p, cr.x()+xo+1, cr.y()+yo+1, cr, cg, 0);
	}
	doc->draw(p, cr.x()+xo, cr.y()+yo, cr, colorGroup(), 0);
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
	    m = p->fontMetrics().width('x')/2;
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
    updateGeometry();
}

/*!
  Sets a QMovie to display in the label, or removes any existing movie
  if the given movie QMovie::isNull().

  Any current pixmap or text label is cleared.

  If the label has a buddy, the accelerator is disabled since the
  movie doesn't contain any suitable character.

  \sa unsetMovie()
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

  \sa setMovie(), unsetMovie()
*/
QMovie* QLabel::movie() const
{
    return lmovie;
}


/*!
  Returns the current text format.

  \sa setTextFormat()
 */
Qt::TextFormat QLabel::textFormat() const
{
    return textformat;
}

/*!
  Sets the text format to \a format. Possible choices are
  <ul>
  <li> \c PlainText - all characters are displayed verbatimely,
  including all blanks and linebreaks. Word wrap is availbe
  with the \c WordBreak alignment flag (see setAlignment() for
  details).
  <li> \c RichText - rich text rendering. The available
  styles are defined in the default stylesheet
  QStyleSheet::defaultSheet().
  <li> \c AutoText - this is also the default. The label
  autodetects which rendering style suits best, \c PlainText
  or \c RichText. Technically, this is done by using the
  QStyleSheet::mightBeRichText() heuristic.
  </ul>
 */
void QLabel::setTextFormat( Qt::TextFormat format )
{
    textformat = format;
    QString tmp = ltext;
    ltext = QString::null;
    setText( tmp ); // trigger update
}
