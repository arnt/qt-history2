/****************************************************************************
**
** Implementation of date and time edit classes
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/*! \file datetime/main.cpp */

#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

#include "../kernel/qrichtext_p.h"
#include "qapplication.h"
#include "qpushbutton.h"
#include "qbitmap.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qapplication.h"
#include "qvaluelist.h"
#include "qstring.h"

#include "math.h" // floor()

#define QDATETIMEEDIT_HIDDEN_CHAR '0'

class QDateTimeEditBase::Private
{
public:
    Private()
	: frm( TRUE ),
	  parag( new QTextParag( 0, 0, 0, FALSE ) ),
	  pm(0),
	  focusSec(0),
	  sep("-")
    {
	parag->formatter()->setWrapEnabled( FALSE );
	cursor = new QTextCursor( 0 );
	cursor->setParag( parag );
	pm = new QPixmap;
    }
    ~Private()
    {
	delete parag;
	delete cursor;
	delete pm;
    }

    void appendSection( const QNumberSection& sec )
    {
	sections.append( sec );
    }
    void setSectionSelection( int sec, int selstart, int selend )
    {
	if ( sec < 0 || sec > (int)sections.count() )
	    return;
	sections[sec].setSelectionStart( selstart );
	sections[sec].setSelectionEnd( selend );
    }
    uint sectionCount() const { return sections.count(); }
    void setSeparator( const QString& s ) { sep = s; }
    QString separator() const { return sep; }

    void setFrame( bool f ) { frm = f; }
    bool frame() const { return frm; }

    int focusSection() const { return focusSec; }
    int section( const QPoint& p )
    {
	cursor->place( p, parag );
	int idx = cursor->index();
	for ( uint i = 0; i < sections.count(); ++i ) {
	    if ( idx >= sections[i].selectionStart() &&
		 idx <= sections[i].selectionEnd() )
		return i;
	}
	return -1;
    }
    bool setFocusSection( int idx )
    {
	if ( idx > (int)sections.count()-1 || idx < 0 )
	    return FALSE;
	if ( idx != focusSec ) {
	    focusSec = idx;
	    applyFocusSelection();
	    return TRUE;
	}
	return FALSE;
    }

    bool inSectionSelection( int idx )
    {
	for ( uint i = 0; i < sections.count(); ++i ) {
	    if ( idx >= sections[i].selectionStart() &&
		 idx <= sections[i].selectionEnd() )
		return TRUE;
	}
	return FALSE;
    }

    void paint( const QString& txt, bool focus, QPainter& p,
		const QColorGroup& cg, const QRect& rect, QStyle& style )
    {
	int fw = 0;
	if ( frm )
	    fw = style.defaultFrameWidth();

	parag->truncate( 0 );
	parag->append( txt );
	if ( !focus )
	    parag->removeSelection( QTextDocument::Standard );
	else {
	    applyFocusSelection();
	}

	/* color all QDATETIMEEDIT_HIDDEN_CHAR chars to background color */
	QTextFormat *fb = parag->formatCollection()->format( p.font(),
							     cg.base() );
	for ( uint i = 0; i < txt.length(); ++i ) {
	    if ( inSectionSelection( i ) )
		continue;
	    if ( txt.at(i) == QDATETIMEEDIT_HIDDEN_CHAR )
		parag->setFormat( i, 1, fb );
	}
	fb->removeRef();

	QRect r( rect.x(), rect.y(), rect.width() - 2 * ( 2 + fw ), rect.height() );
	parag->setDocumentRect( r );
	parag->invalidate(0);
	parag->format();

	int xoff = 2 + fw;
	int yoff = (rect.height() - parag->rect().height())/2;
	if ( yoff < 0 )
	    yoff = 0;

	p.translate( xoff, yoff );
	parag->paint( p, cg, 0, TRUE );
	if ( frm )
	    p.translate( -xoff, -yoff );
    }

    void resize( const QSize& size )
    {
	pm->resize( size );
    }

    QPixmap* pixmap() { return pm; }

protected:
    void applyFocusSelection()
    {
	if ( focusSec > -1 ) {
	    int selstart = sections[ focusSec ].selectionStart();
	    int selend = sections[ focusSec ].selectionEnd();
	    parag->setSelection( QTextDocument::Standard, selstart, selend );
	}
    }
private:
    bool frm;
    QTextParag *parag;
    QTextCursor *cursor;
    QPixmap *pm;
    int focusSec;
    QValueList< QNumberSection > sections;
    QString sep;
};

/*!

  \class QDateTimeEditBase qdatetimeedit.h

  \brief The QDateTimeEditBase class provides common functionality for
  date/time editors.

  \module ???

  The QDateTimeEditBase class provides some common functionality for
  date/time editors.  For date/time editing widgets, see QDateEdit,
  QTimeEdit and QDateTimeEdit.

  Terminology: A QDateEdit widget is comprised of three 'sections', one
  each for the year, month and day. Similarly a QTimeWidget is comprised
  of three sections, one each for the hour, minute and second.

*/

/*!  Constructs an empty datetime base with parent \a parent and name \a
   name.

*/

QDateTimeEditBase::QDateTimeEditBase( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = new Private();
    init();
}


/*! Destroys the object and frees any allocated resources.

*/

QDateTimeEditBase::~QDateTimeEditBase()
{
    delete d;
}


/*! \internal

*/

void QDateTimeEditBase::init()
{
    setBackgroundMode(PaletteBase);

    up   = new QPushButton( this );
    up->setFocusPolicy( QWidget::NoFocus );
    up->setAutoDefault( FALSE );
    up->setAutoRepeat( TRUE );

    down = new QPushButton( this );
    down->setFocusPolicy( QWidget::NoFocus );
    down->setAutoDefault( FALSE );
    down->setAutoRepeat( TRUE );

    connect( up, SIGNAL( clicked() ), SLOT( stepUp() ) );
    connect( down, SIGNAL( clicked() ), SLOT( stepDown() ) );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );

    setFocusPolicy( StrongFocus );
    setFocusSection( -1 );
    setKeyCompression( TRUE );
}


/*! \reimp

*/

bool QDateTimeEditBase::event( QEvent *e )
{
    if ( e->type() == QEvent::FocusIn ) {
	repaint( rect(), FALSE);
    }
    if( e->type() == QEvent::FocusOut ){
	repaint( rect(), FALSE );
    }
    return QWidget::event( e );
}


/*! \internal

*/

void QDateTimeEditBase::updateArrows()
{
    QString key( QString::fromLatin1( "$qt$qdatetimeedit$" ) );
    key += QString::fromLatin1( "^v" );
    key += QString::number( down->height() );
    QString upKey = key + QString::fromLatin1( "$up" );
    QString dnKey = key + QString::fromLatin1( "$down" );
    QBitmap upBm;
    QBitmap dnBm;

    bool found = QPixmapCache::find( dnKey, dnBm )
		 && QPixmapCache::find( upKey, upBm );

    if ( !found ) {
	QPainter p;
	int w = down->width()-4;
	if ( w < 3 )
	    return;
	else if ( !(w & 1) )
	    w--;
	w -= ( w / 7 ) * 2;     // Empty border
	int h = w/2 + 2;        // Must have empty row at foot of arrow
	dnBm.resize( w, h );
	p.begin( &dnBm );
	p.eraseRect( 0, 0, w, h );
	QPointArray a;
	a.setPoints( 3,  0, 1,  w-1, 1,  h-2, h-1 );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
#ifndef QT_NO_TRANSFORMATIONS
	QWMatrix wm;
	wm.scale( 1, -1 );
	upBm = dnBm.xForm( wm );
#else
	upBm.resize( w, h );
	p.begin( &upBm );
	p.eraseRect( 0, 0, w, h );
	a.setPoints( 3,  0, h-2,  w-1, h-2,  h-2, 0 );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
#endif
	QPixmapCache::insert( dnKey, dnBm );
	QPixmapCache::insert( upKey, upBm );
    }
    down->setPixmap( dnBm );
    up->setPixmap( upBm );

}



/*! \internal

  Lays out the arrows according to the size \a s.

*/

void QDateTimeEditBase::layoutArrows( const QSize& s )
{
    int fw = 0;
    if ( frame() )
	fw = style().defaultFrameWidth();

    QSize bs;
    bs.setHeight( height()/2 - fw );
    if ( bs.height() < 5 )
	bs.setHeight( 5 );
    bs.setWidth( bs.height() * 8 / 5 ); // 1.6 - approximate golden mean

    int y = fw;
    int x, lx;
    if ( QApplication::reverseLayout() ) {
	x = y;
	lx = x + bs.width() + fw;
    } else {
	x = width() - y - bs.width();
	lx = fw;
    }

    if ( up->size() != bs || down->size() != bs ) {
	up->resize( bs );
	down->resize( bs );
	updateArrows();
    }

    up->move( x, y );
    down->move( x, height() - y - up->height() );
    QSize pmSize( s.width(), s.height() );
    d->resize( pmSize );
}

/*! \reimp

*/

void QDateTimeEditBase::resizeEvent( QResizeEvent *e )
{
    layoutArrows( e->size() );
    QWidget::resizeEvent( e );

#if 0
    int fw       = 0;
    if ( frame() )
	fw = style().defaultFrameWidth();
    int h        = height() - fw*2;
    int numWidth = QFontMetrics(font()).width("X");
    int offset   = width() - up->width() - fw;
    int sepWidth = QFontMetrics(font()).width( separator );
#endif
}


/*! \reimp

*/

void QDateTimeEditBase::paintEvent( QPaintEvent * )
{
    if ( d->pixmap()->isNull() )
	return;
    QString txt;
    for ( uint i = 0; i < d->sectionCount(); ++i ) {
	txt += sectionFormattedText( i );
	if ( i < d->sectionCount()-1 )
	    txt += d->separator();
    }
    const QColorGroup & g = colorGroup();
    QPainter p( d->pixmap() );
    p.setPen( colorGroup().text() );
    QBrush bg = g.brush( QColorGroup::Base );
    p.fillRect( 0, 0, width(), height(), bg );
    d->paint( txt, hasFocus(), p, colorGroup(), rect(), style() );
    if ( frame() ) {
	style().drawPanel( &p, 0, 0, rect().width(), rect().height(), colorGroup(),
			   TRUE, style().defaultFrameWidth() );
    }
    p.end();
    bitBlt( this, 0, 0, d->pixmap() );
}


/*! \reimp

*/

void QDateTimeEditBase::mousePressEvent( QMouseEvent *e )
{
    QPoint p( e->pos().x(), 0 );
    int sec = d->section( p );
    if ( sec != -1 ) {
	setFocusSection( sec );
	repaint( rect(), FALSE );
    }
}

/*!  Returns TRUE if the editor draws itself inside a frame, FALSE if
  it draws itself without any frame.

  The default is to use a frame.

  \sa setFrame()
*/

bool QDateTimeEditBase::frame() const
{
    return d ? d->frame() : TRUE;
}

/*!  Sets the editor to draw itself inside a two-pixel frame if \a
  enable is TRUE and to draw itself without any frame if \a enable is
  FALSE.

  The default is TRUE.

  \sa frame()
*/

void QDateTimeEditBase::setFrame( bool enable )
{
    if ( d->frame() == enable )
	return;

    d->setFrame( enable );
    update();
}

/*! \reimp

*/

void QDateTimeEditBase::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Right:
	if ( d->focusSection() < 2 ) {
	    if ( setFocusSection( focusSection()+1 ) ) {
		repaint( rect(), FALSE );
		return;
	    }
	}
	break;
    case Key_Left:
	if ( d->focusSection() > 0 ) {
	    if ( setFocusSection( focusSection()-1 ) ) {
		repaint( rect(), FALSE );
		return;
	    }
	}
	break;
    case Key_Up:
	stepUp();
	break;
    case Key_Down:
	stepDown();
	break;
    case Key_Backspace:
    case Key_Delete:
	removeLastNumber( d->focusSection() );
	break;
    default:
	QString txt = e->text();
	int num = txt[0].digitValue();
	if ( num != -1 ) {
	    addNumber( d->focusSection(), num );
	    return;
	}
    }
    QWidget::keyPressEvent( e );
}


/*! Appends the number section \a sec to the editor.

*/

void QDateTimeEditBase::appendSection( const QNumberSection& sec )
{
    d->appendSection( sec );
}


/*! Virtual function which returns the formatted text of section \a
  sec.  The default implementation returns QString::null

*/

QString QDateTimeEditBase::sectionFormattedText( int )
{
    return QString::null;
}


/*! Sets the selection of \a sec to start at \a selstart and end at \a
  selend.

*/

void QDateTimeEditBase::setSectionSelection( int sec, int selstart, int selend )
{
    d->setSectionSelection( sec, selstart, selend );
}

/*! Sets the separator for all numbered sections to \a s.  Note that
  currently, only the first character of \a s is used.

*/

void QDateTimeEditBase::setSeparator( const QString& s )
{
    d->setSeparator( s );
}


/*! Returns the separator for the editor.

*/

QString QDateTimeEditBase::separator() const
{
    return d->separator();
}

/*! Returns the number of the currently focused section.

*/

int QDateTimeEditBase::focusSection() const
{
    return d->focusSection();
}


/*! Sets the currently focused section to \a sec.  If \a sec does not
  exist, nothing happens.

*/

bool QDateTimeEditBase::setFocusSection( int sec )
{
    return d->setFocusSection( sec );
}

/*! Virtual function which is called whenever the user increases a
  section number by pressing the widget's arrow buttons or the
  keyboard's arrow keys.

*/

void QDateTimeEditBase::stepUp()
{

}

/*! Virtual function which is called whenever the user decreases a
  section number by pressing the widget's arrow buttons or the
  keyboard's arrow keys.

*/

void QDateTimeEditBase::stepDown()
{

}

/*! Virtual function which is called whenever the user types a number.
  \a sec indicates the section where the number should be added.  \a
  num is the number that was pressed.

*/

void QDateTimeEditBase::addNumber( int , int  )
{

}

/*! Virtual function which is called whenever the user tries to remove
   the last number from \a sec by pressing the backspace or delete
   key.

*/

void QDateTimeEditBase::removeLastNumber( int  )
{

}

/*!
  \reimp
*/
const QColor & QDateTimeEditBase::foregroundColor() const
{
    return foregroundColorForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEditBase::setForegroundColor( const QColor & color )
{
    setForegroundColorForMode(PaletteBase, color);
}

/*!
  \reimp
*/
const QColor & QDateTimeEditBase::backgroundColor() const
{
    return backgroundColorForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEditBase::setBackgroundColor( const QColor & color )
{
    setBackgroundColorForMode(PaletteBase, color);
}

/*!
  \reimp
*/
const QPixmap* QDateTimeEditBase::backgroundPixmap() const
{
    return backgroundPixmapForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEditBase::setBackgroundPixmap( const QPixmap & pixmap )
{
    setBackgroundPixmapForMode(PaletteBase, pixmap);
}

////////////////

class QDateEdit::Private
{
public:
    int y;
    int m;
    int d;
    int yearSection;
    int monthSection;
    int daySection;
    Order ord;
    bool overwrite;
    bool adv;
    int timerId;
    bool typing;
    QDate min;
    QDate max;
    bool changed;
};


/*!
  \class QDateEdit qdatetimeedit.h

  \brief The QDateEdit class provides a date editor.

  QDateEdit allows the user to edit dates by using the keyboard or the
  arrow keys to increase/decrease date values. The arrow keys can be
  used to move from section to section within the QDateEdit box. Dates
  appear in year, month, day order by default. It is recommended that
  the QDateEdit be initialised with a date, e.g.

    \code
    QDateEdit *dateedit = new QDateEdit( QDate::currentDate(), this );
    dateedit->setRange( QDate::currentDate().addDays( -365 ),
			QDate::currentDate().addDays(  365 ) );
    dateedit->setOrder( QDateEdit::MDY );
    dateedit->setAutoAdvance( TRUE );
    \endcode

    Here we've created a new QDateEdit object initialised with today's
    date and restricted the valid date range to today plus or minus 365
    days. We've set the order to month, day, year. If the auto advance
    property is TRUE (as we've set it here) when the user completes a
    section of the date, e.g. enters two digits for the month, they are
    automatically taken to the next section.

  Terminology: A QDateEdit widget comprises three 'sections', one each
  for the year, month and day.

  \sa QTimeEdit QDateTimeEdit
*/

/*!
  \enum QDateEdit::Order

  This enum defines the order in which the sections that comprise a date
  appear.
  \value MDY month-day-year
  \value DMY day-month-year
  \value YMD year-month-day (the default)
  \value YDM year-day-month
*/


/*! Constructs an empty date editor which is a child of \a parent and the
  name \a name.

*/

QDateEdit::QDateEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
}

/*!  
    \overload

Constructs a date editor with the initial value \a date, parent \a
parent and name \a name.

   The date editor is initialize with \a date.
*/

QDateEdit::QDateEdit( const QDate& date, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setDate( date );
}

/*! \internal
*/

void QDateEdit::init()
{
    d = new Private();
    appendSection( QNumberSection( 0,4 ) );
    appendSection( QNumberSection( 5,7 ) );
    appendSection( QNumberSection( 8,10 ) );

    d->yearSection = -1;
    d->monthSection = -1;
    d->daySection = -1;

    d->y = 0;
    d->m = 0;
    d->d = 0;
    setOrder( YMD );
    setFocusSection( 0 );
    d->overwrite = TRUE;
    d->adv = FALSE;
    d->timerId = 0;
    d->typing = FALSE;
    d->changed = FALSE;
}

/*! Destroys the object and frees any allocated resources.

*/

QDateEdit::~QDateEdit()
{
    delete d;
}

/*! \fn void QDateEdit::setMinValue( const QDate& d )

  Sets the minimum date value to \a d.  Equivalent to calling
  QDateEdit::setRange( d, maxValue() );
*/

/*! Returns the minimum date value for the editor, or an invalid date
  if there is none.

*/

QDate QDateEdit::minValue() const
{
    return d->min;
}

/*! \fn void QDateEdit::setMaxValue( const QDate& d )

  Sets the maximum date value to \a d.  Equivalent to calling
  QDateEdit::setRange( minValue(), d );
*/

/*! Returns the maximum date value for the editor, or an invalid date
  if there is none.

*/

QDate QDateEdit::maxValue() const
{
    return d->max;
}


/*! Sets the valid input range for the editor to be from \a min to \a
  max inclusive.  If \a min is invalid no minimum date will be set.
  Similarly, if \a max is invalid no maximum date will be set.

*/

void QDateEdit::setRange( const QDate& min, const QDate& max )
{
    d->min = min;
    d->max = max;
}


/*! \reimp

*/

QSize QDateEdit::sizeHint() const
{
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.width( 'x' ) * 14;
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 22 )
	    h = 22;
	return QSize( w + 8, h ).expandedTo( QApplication::globalStrut() );
    } else {
	return QSize( w + 4, h + 4 ).expandedTo( QApplication::globalStrut() );
    }
}


/*! Returns the formatted number for section \a sec.  This will
  correspond to either the year, month or day section, depending on
  the current display order.

  \sa setOrder()

*/

QString QDateEdit::sectionFormattedText( int sec )
{
    QString txt;
    txt = sectionText( sec );
    if ( d->typing && sec == focusSection() )
	setSectionSelection( sec, sectionOffsetEnd( sec ) - txt.length(),
			     sectionOffsetEnd( sec ) );
    else
	setSectionSelection( sec, sectionOffsetEnd( sec ) - sectionLength( sec ),
			     sectionOffsetEnd( sec ) );
    txt = txt.rightJustify( sectionLength( sec ), QDATETIMEEDIT_HIDDEN_CHAR );
    return txt;
}


/*! Returns the desired length (number of digits) of section \a sec.
   This will correspond to either the year, month or day section,
   depending on the current display order.

  \sa setOrder()
*/

int QDateEdit::sectionLength( int sec )
{
    int val = 0;
    if ( sec == d->yearSection ) {
	val = 4;
    } else if ( sec == d->monthSection ) {
	val = 2;
    } else if ( sec == d->daySection ) {
	val = 2;
    }
    return val;
}

/*! Returns the text of section \a sec.  This will correspond to
  either the year, month or day section, depending on the current
  display order.

  \sa setOrder()

*/

QString QDateEdit::sectionText( int sec )
{
    int val = 0;
    if ( sec == d->yearSection ) {
	val = d->y;
    } else if ( sec == d->monthSection ) {
	val = d->m;
    } else if ( sec == d->daySection ) {
	val = d->d;
    }
    return QString::number( val );
}

/*! \internal

  Returns the end of the section offset \a sec.

*/

int QDateEdit::sectionOffsetEnd( int sec )
{
    if ( sec == d->yearSection ) {
	switch( d->ord ) {
	case DMY:
	case MDY:
	    return 10;
	case YMD:
	case YDM:
	    return 4;
	}
    } else if ( sec == d->monthSection ) {
	switch( d->ord ) {
	case DMY:
	    return 5;
	case YMD:
	    return 7;
	case MDY:
	    return 2;
	case YDM:
	    return 10;
	}
    } else if ( sec == d->daySection ) {
	switch( d->ord ) {
	case DMY:
	    return 2;
	case YMD:
	    return 10;
	case MDY:
	    return 5;
	case YDM:
	    return 7;
	}
    }
    return 0;
}


/*! 

    Sets the display order of the numbered section of the date editor to
    \a order.

  \sa Order

*/

void QDateEdit::setOrder( QDateEdit::Order order )
{
    d->ord = order;
    switch( d->ord ) {
    case DMY:
	d->yearSection = 2;
	d->monthSection = 1;
	d->daySection = 0;
	break;
    case MDY:
	d->yearSection = 2;
	d->monthSection = 0;
	d->daySection = 1;
	break;
    case YMD:
	d->yearSection = 0;
	d->monthSection = 1;
	d->daySection = 2;
	break;
    case YDM:
	d->yearSection = 0;
	d->monthSection = 2;
	d->daySection = 1;
	break;
    }
    if ( isVisible() )
	repaint( rect(), FALSE );
}


/*!  Returns the current order of the numbered section in the date
  editor.  The default is QDateEdit::YMD.

  \sa setOrder()

*/

QDateEdit::Order QDateEdit::order() const
{
    return d->ord;
}


/*! \reimp

*/

void QDateEdit::stepUp()
{
    int sec = focusSection();
    if ( sec == d->yearSection ) {
	if ( !outOfRange( d->y+1, d->m, d->d ) )
	    setYear( d->y+1 );
    } else if ( sec == d->monthSection ) {
	if ( !outOfRange( d->y, d->m+1, d->d ) )
	    setMonth( d->m+1 );
    } else if ( sec == d->daySection ) {
	if ( !outOfRange( d->y, d->m, d->d+1 ) )
	    setDay( d->d+1 );
    }
    d->changed = TRUE;
    repaint( rect(), FALSE );
}



/*! \reimp

*/

void QDateEdit::stepDown()
{
    int sec = focusSection();
    if ( sec == d->yearSection ) {
	if ( !outOfRange( d->y-1, d->m, d->d ) )
	    setYear( d->y-1 );
    } else if ( sec == d->monthSection ) {
	if ( !outOfRange( d->y, d->m-1, d->d ) )
	    setMonth( d->m-1 );
    } else if ( sec == d->daySection ) {
	if ( !outOfRange( d->y, d->m, d->d-1 ) )
	    setDay( d->d-1 );
    }
    d->changed = TRUE;
    repaint( rect(), FALSE );
}


/*!  Sets the year to \a year, which must be a valid year.
    The range currently supported is from 1752 to 8000.

  \sa QDate

*/

void QDateEdit::setYear( int year )
{
    if ( year < 1752 )
	year = 1752;
    if ( year > 8000 )
	year = 8000;
    if ( !outOfRange( year, d->m, d->d ) ) {
	d->y = year;
	setMonth( d->m );
	setDay( d->d );
    }
}


/*! Sets the month to \a month, which must be a valid month, i.e.
   between 1 and 12.

*/

void QDateEdit::setMonth( int month )
{
    if ( month < 1 )
	month = 1;
    if ( month > 12 )
	month = 12;
    if ( !outOfRange( d->y, month, d->d ) ) {
	d->m = month;
	setDay( d->d );
    }
}


/*! Sets the day to \a day, which must be a valid day.
    The function will ensure that the \a day set is valid for the
    month and year.

*/

void QDateEdit::setDay( int day )
{
    if ( day < 1 )
	day = 1;
    if ( day > 31 )
	day = 31;
    if ( d->m > 0 && d->y > 1752 ) {
	bool valid = FALSE;
	while ( !(valid = QDate::isValid( d->y, d->m, day ) ) )
	    --day;
	if ( !outOfRange( d->y, d->m, day ) )
	    d->d = day;
    } else if ( d->m > 0 ) {
	if ( day > 0 && day < 32 ) {
	    if ( !outOfRange( d->y, d->m, day ) )
		d->d = day;
	}
    }
}


/*!  Sets the date to \a date.  If \a date is not valid, the editor
  displays all zeroes and QDateEdit::date() will return an invalid
  date.

*/

void QDateEdit::setDate( const QDate& date )
{
    if ( !date.isValid() ) {
	d->y = 0;
	d->m = 0;
	d->d = 0;
	return;
    }
    if ( (maxValue().isValid() && date > maxValue()) ||
	 (minValue().isValid() && date < minValue()) )
	return;
    d->y = date.year();
    d->m =  date.month();
    d->d = date.day();
    emit valueChanged( date );
    d->changed = FALSE;
    repaint( rect(), FALSE );
}

/*!  Returns the current date in the editor, or an invalid date if the
  editor does not contain a valid date.

  \sa setDate()

*/

QDate QDateEdit::date() const
{
    if ( QDate::isValid( d->y, d->m, d->d ) )
	return QDate( d->y, d->m, d->d );
    return QDate();
}

/*!  Returns TRUE if \a y, \a m, \a d is out of range, otherwise 
    returns FALSE.

  \sa setRange()

*/

/*! \internal */
bool QDateEdit::outOfRange( int y, int m, int d ) const
{
    if ( QDate::isValid( y, m, d ) ) {
	QDate currentDate( y, m, d );
	if ( ( maxValue().isValid() && currentDate > maxValue() ) ||
	     ( minValue().isValid() && currentDate < minValue() ) ) {
	    //## outOfRange should set overwrite?
	    return TRUE;
	}
	return FALSE;
    }
    return FALSE; /* assume ok */
}

/*!  \reimp

*/

void QDateEdit::addNumber( int sec, int num )
{
    if ( sec == -1 )
	return;
    killTimer( d->timerId );
    bool overwrite = FALSE;
    d->typing = TRUE;
    QString txt;
    if ( sec == d->yearSection ) {
	txt = QString::number( d->y );
	if ( d->overwrite || txt.length() == 4 ) {
	    d->y = num;
	} else {
	    txt += QString::number( num );
	    if ( txt.length() == 4 && outOfRange( txt.toInt(), d->m, d->d ) ) {
		txt = QString::number( d->y );
	    } else {
		d->y = txt.toInt();
	    }
	    if ( d->adv && txt.length() == 4 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == d->monthSection ) {
	txt = QString::number( d->m );
	if ( d->overwrite || txt.length() == 2 )
	    d->m = num;
	else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 12 )
		temp = num;
	    if ( outOfRange( d->y, temp, d->d ) )
		txt = QString::number( d->m );
	    else
		d->m = temp;
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == d->daySection ) {
	txt = QString::number( d->d );
	if ( d->overwrite || txt.length() == 2 )
	    d->d = num;
	else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 31 )
		temp = num;
	    if ( outOfRange( d->y, d->m, temp ) )
		txt = QString::number( d->d );
	    else
		d->d = temp;
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    }
    d->changed = TRUE;
    d->overwrite = overwrite;
    d->timerId = startTimer( qApp->doubleClickInterval()*4 );
    repaint( rect(), FALSE );
}


/*! \reimp

*/

bool QDateEdit::setFocusSection( int s )
{
    if ( s != focusSection() ) {
	killTimer( d->timerId );
	d->overwrite = TRUE;
	d->typing = FALSE;
	fix(); // will emit valueChanged if necessary
    }
    return QDateTimeEditBase::setFocusSection( s );
}


/*! Attempts to fix any invalid date entries.

    The rules applied are as follows:

    <ul>
    <li>If the year has four digits it is left unchanged.
    <li>If the year has two digits in the range 70..99, the previous
    century, i.e. 1900, will be added giving a year in the range
    1970..1999.
    <li>If the year has two digits in the range 0..69, the current
    century, i.e. 2000, will be added giving a year in the range
    2000..2069.
    <li>If the year is in the range 100..999, the current century, i.e.
    2000, will be added giving a year in the range 2100..2999.
    </ul>

*/

void QDateEdit::fix()
{
    bool changed = FALSE;
    QDate currentDate = QDate::currentDate();
    int year = d->y;
    if ( year < 100 ) {
	int currentCentury = (int) floor( (double)currentDate.year()/100 );
	int loFullYear = currentDate.year() - 70;
	int loCentury = (int) ( floor(loFullYear/100 ) < currentCentury ) ?
			(int) floor( loFullYear/100 ) : currentCentury;
	int loYear = loFullYear - ( loCentury * 100 );
	int hiCentury = currentCentury;
	if ( loCentury == currentCentury )
	    ++hiCentury;
	if ( year >= loYear )
	    year = ( loCentury*100 ) + year;
	else
	    year = ( hiCentury*100 ) + year;
	changed = TRUE;
    } else if ( year > 99 && year <= 999 ) {
	int currentCentury = (int) floor( (double)currentDate.year()/100 );
	year = ( currentCentury*100 ) + year;
	changed = TRUE;
    }
    if ( changed && outOfRange( year, d->m, d->d ) ) {
	if ( minValue().isValid() && date() < minValue() ) {
	    d->d =  minValue().day();
	    d->m = minValue().month();
	    d->y = minValue().year();
	}
	if ( date() > maxValue() ) {
	    d->d =  maxValue().day();
	    d->m = maxValue().month();
	    d->y = maxValue().year();
	}
    } else if ( changed )
	setYear( year );
    if ( changed ) {
	emit valueChanged( date() );
	d->changed = FALSE;
    }
}


/*! \reimp

*/

bool QDateEdit::event( QEvent *e )
{
    if( e->type() == QEvent::FocusOut ) {
	d->typing = FALSE;
	fix();
	if ( d->changed ) {
	    emit valueChanged( date() );
	    d->changed = FALSE;
	}
    }
    return QDateTimeEditBase::event( e );
}

/*! \reimp

*/

void QDateEdit::removeLastNumber( int sec )
{
    if ( sec == -1 )
	return;
    QString txt;
    if ( sec == d->yearSection ) {
	txt = QString::number( d->y );
	txt = txt.mid( 0, txt.length()-1 );
	d->y = txt.toInt();
    } else if ( sec == d->monthSection ) {
	txt = QString::number( d->m );
	txt = txt.mid( 0, txt.length()-1 );
	d->m = txt.toInt();
    } else if ( sec == d->daySection ) {
	txt = QString::number( d->d );
	txt = txt.mid( 0, txt.length()-1 );
	d->d = txt.toInt();
    }
    repaint( rect(), FALSE );
}

/*! Sets the auto advance property of the editor to \a advance.  If
  set to TRUE, the editor will automatically advance focus to the next
  date section if a user has completed a section.

  \sa autoAdvance()
*/

void QDateEdit::setAutoAdvance( bool advance )
{
    d->adv = advance;
}


/*! Returns TRUE if auto-advance is enabled, otherwise returns FALSE.

  \sa setAutoAdvance()

*/

bool QDateEdit::autoAdvance() const
{
    return d->adv;
}

/*! \reimp

*/

void QDateEdit::timerEvent( QTimerEvent * )
{
    d->overwrite = TRUE;
}


/*! \fn void QDateEdit::valueChanged( const QDate& date )

  This signal is emitted whenever the editor's value changes.  The \a date
  parameter is the new value.

*/

///////////

class QTimeEdit::Private
{
public:
    int h;
    int m;
    int s;
    bool adv;
    bool overwrite;
    int timerId;
    bool typing;
    QTime min;
    QTime max;
    bool changed;
};

/*!
  \class QTimeEdit qdatetimeedit.h

  \brief The QTimeEdit class provides a time editor.

  QTimeEdit allows the user to edit times by using the keyboard or the
  arrow keys to increase/decrease time values. The arrow keys can be
  used to move from section to section within the QTimeEdit box. Times
  appear in hour, minute, second order by default, but . It is recommended
  that the QTimeEdit be initialised with a time, e.g.

    \code
    QTime timeNow = QTime::currentTime();
    QTimeEdit *timeedit = new QTimeEdit( timeNow, this );
    timeedit->setRange( timeNow, timeNow.addSecs( 60 * 60 ) );
    \endcode

    Here we've created a QTimeEdit widget set to the current time. We've
    also set the minimum value to the current time and the maximum time
    to one hour from now.

  Terminology: A QTimeWidget consists of three sections, one each for the
  hour, minute and second.

  \sa QDateEdit QDateTimeEdit

*/


/*!  Constructs an empty time edit with parent \a parent and name \a
   name.

*/

QTimeEdit::QTimeEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
}

/*!  
    \overload
    
    Constructs a time edit with the initial value \a time, parent \a
    parent and name \a name.

  The time edit is initialized with \a time.
*/

QTimeEdit::QTimeEdit( const QTime& time, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setTime( time );
}

/*! \internal
 */

void QTimeEdit::init()
{
    d = new Private();
    appendSection( QNumberSection( 0,0 ) );
    appendSection( QNumberSection( 0,0 ) );
    appendSection( QNumberSection( 0,0 ) );
    setSeparator( ":" );

    d->h = 0;
    d->m = 0;
    d->s = 0;
    d->adv = FALSE;
    d->overwrite = FALSE;
    d->timerId = 0;
    d->typing = FALSE;
    d->changed = FALSE;
}

/*! Destroys the object and frees any allocated resources.

*/

QTimeEdit::~QTimeEdit()
{
    delete d;
}

/*! \fn void QTimeEdit::setMinValue( const QTime& d )

  Sets the minimum time value to \a d.  Equivalent to calling
  QTimeEdit::setRange( d, maxValue() );
*/

/*! Returns the minimum time value for the editor, or an invalid time
  if there is none.

  \sa setMinValue()

*/

QTime QTimeEdit::minValue() const
{
    return d->min;
}

/*! \fn void QTimeEdit::setMaxValue( const QTime& d )

  Sets the maximum time value to \a d.  Equivalent to calling
  QTimeEdit::setRange( minValue(), d );
*/

/*! Returns the maximum time value for the editor, or an invalid time
  if there is none.

    \sa setMaxValue()
*/

QTime QTimeEdit::maxValue() const
{
    return d->max;
}


/*! Sets the valid input range for the editor to be from \a min to \a
  max inclusive.  If \a min is invalid no minimum time is set.
  Similarly, if \a max is invalid no maximum time is set.

*/

void QTimeEdit::setRange( const QTime& min, const QTime& max )
{
    d->min = min;
    d->max = max;
}



/*!  Sets the time to \a time.  If \a time is not valid, the editor
  displays all zeroes and QTimeEdit::time() will return an invalid
  time.

*/

void QTimeEdit::setTime( const QTime& time )
{
    if ( !time.isValid() ) {
	d->h = 0;
	d->m = 0;
	d->s = 0;
	return;
    }
    if ( time > maxValue() || time < maxValue() )
	return;
    d->h = time.hour();
    d->m = time.minute();
    d->s = time.second();
    emit valueChanged( time );
    d->changed = FALSE;
    repaint( rect(), FALSE );
}


/*!  Returns the current time in the editor, or an invalid time if the
  editor does not contain a valid time.

*/

QTime QTimeEdit::time() const
{
    if ( QTime::isValid( d->h, d->m, d->s ) )
	return QTime( d->h, d->m, d->s );
    return QTime();
}

/*! Sets the auto advance property of the editor to \a advance.  If
  set to TRUE, the editor will automatically advance focus to the next
  time section if a user has completed a section.

  \sa autoAdvance()
*/

void QTimeEdit::setAutoAdvance( bool advance )
{
    d->adv = advance;
}

/*! Returns TRUE if auto-advance is enabled, otherwise FALSE is
  returned.

  \sa setAutoAdvance()

*/

bool QTimeEdit::autoAdvance() const
{
    return d->adv;
}


/*! \fn void QTimeEdit::valueChanged( const QTime& time )

  This signal is emitted whenever the editor's value changes.  The \a
  time parameter is the new value.

*/

/*! \reimp

*/

bool QTimeEdit::event( QEvent *e )
{
    if( e->type() == QEvent::FocusOut ) {
	d->typing = FALSE;
	if ( d->changed ) {
	    emit valueChanged( time() );
	    d->changed = FALSE;
	}
    }
    return QDateTimeEditBase::event( e );
}


/*! \reimp

*/

void QTimeEdit::timerEvent( QTimerEvent * )
{
    d->overwrite = TRUE;
}


/*! \reimp

*/

void QTimeEdit::stepUp()
{
    int sec = focusSection();
    switch( sec ) {
    case 0:
	if ( !outOfRange( d->h+1, d->m, d->s ) )
	     setHour( d->h+1 );
	break;
    case 1:
	if ( !outOfRange( d->h, d->m+1, d->s ) )
	    setMinute( d->m+1 );
	break;
    case 2:
	if ( !outOfRange( d->h, d->m, d->s+1 ) )
	    setSecond( d->s+1 );
	break;
    }
    d->changed = TRUE;
    repaint( rect(), FALSE );
}


/*! \reimp

*/

void QTimeEdit::stepDown()
{
    int sec = focusSection();
    switch( sec ) {
    case 0:
	if ( !outOfRange( d->h-1, d->m, d->s ) )
	     setHour( d->h-1 );
	break;
    case 1:
	if ( !outOfRange( d->h, d->m-1, d->s ) )
	    setMinute( d->m-1 );
	break;
    case 2:
	if ( !outOfRange( d->h, d->m, d->s-1 ) )
	    setSecond( d->s-1 );
	break;
    }
    d->changed = TRUE;
    repaint( rect(), FALSE );
}


/*! Returns the formatted number for section \a sec.  This will
  correspond to either the hour, minute or second section, depending
  on \a sec.

*/

QString QTimeEdit::sectionFormattedText( int sec )
{
    QString txt;
    txt = sectionText( sec );
    int offset = sec*3 + 2;
    if ( d->typing && sec == focusSection() )
	setSectionSelection( sec, offset - txt.length(), offset );
    else
	setSectionSelection( sec, offset - 2, offset );
    txt = txt.rightJustify( 2, QDATETIMEEDIT_HIDDEN_CHAR );
    return txt;
}


/*! \reimp

*/

bool QTimeEdit::setFocusSection( int s )
{
    if ( s != focusSection() ) {
	killTimer( d->timerId );
	d->overwrite = TRUE;
	d->typing = FALSE;
	int offset = s*3 + 2;
	setSectionSelection( s, offset - 2, offset );
	if ( d->changed ) {
	    emit valueChanged( time() );
	    d->changed = FALSE;
	}
    }
    return QDateTimeEditBase::setFocusSection( s );
}


/*! Sets the hour to \a h, which must be a valid hour, i.e. in the range
 0 to 24.

*/

void QTimeEdit::setHour( int h )
{
    if ( h < 0 )
	h = 0;
    if ( h > 23 )
	h = 23;
    d->h = h;
}


/*! Sets the minute to \a m, which must be a valid minute, i.e. in the
 range 0 to 59.

*/

void QTimeEdit::setMinute( int m )
{
    if ( m < 0 )
	m = 0;
    if ( m > 59 )
	m = 59;
    d->m = m;
}


/*! Sets the second to \a s, which must be a valid second, i.e. in the
   range 0 to 59.

*/

void QTimeEdit::setSecond( int s )
{
    if ( s < 0 )
	s = 0;
    if ( s > 59 )
	s = 59;
    d->s = s;
}


/*! \internal

  Returns the text of section \a sec.

*/

QString QTimeEdit::sectionText( int sec )
{
    QString txt;
    switch( sec ) {
    case 0:
	txt = QString::number( d->h );
	break;
    case 1:
	txt = QString::number( d->m );
	break;
    case 2:
	txt = QString::number( d->s );
	break;
    }
    return txt;
}

/*! Returns TRUE if \a h, \a m, \a s is out of range.
 */

/*! \internal */
bool QTimeEdit::outOfRange( int h, int m, int s ) const
{
    if ( QTime::isValid( h, m, s ) ) {
	QTime currentTime( h, m, s );
	if ( ( maxValue().isValid() && currentTime > maxValue() ) ||
	     ( minValue().isValid() && currentTime < minValue() ) )
	    return TRUE;
	else
	    return FALSE;
    }
    return FALSE;
}

/*! \reimp

*/

void QTimeEdit::addNumber( int sec, int num )
{
    if ( sec == -1 )
	return;
    killTimer( d->timerId );
    bool overwrite = FALSE;
    d->typing = TRUE;
    QString txt;
    if ( sec == 0 ) {
	txt = QString::number( d->h );
	if ( d->overwrite || txt.length() == 2 ) {
	    if ( !outOfRange( num, d->m, d->s ) )
		d->h = num;
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 23 )
		temp = num;
	    if ( outOfRange( temp, d->m, d->s ) )
		txt = QString::number( d->h );
	    else
		d->h = temp;
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == 1 ) {
	txt = QString::number( d->m );
	if ( d->overwrite || txt.length() == 2 ) {
	    if ( !outOfRange( d->h, num, d->s ) )
		d->m = num;
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 59 )
		temp = num;
	    if ( outOfRange( d->h, temp, d->s ) )
		txt = QString::number( d->m );
	    else
		d->m = temp;
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == 2 ) {
	txt = QString::number( d->s );
	if ( d->overwrite || txt.length() == 2 ) {
	    if ( !outOfRange( d->h, d->m, num ) )
		d->s = num;
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 59 )
		temp = num;
	    if ( outOfRange( d->h, d->m, temp ) )
		txt = QString::number( d->s );
	    else
		d->s = temp;
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    }
    d->changed = TRUE;
    d->overwrite = overwrite;
    d->timerId = startTimer( qApp->doubleClickInterval()*4 );
    repaint( rect(), FALSE );
}


/*! \reimp

*/

void QTimeEdit::removeLastNumber( int sec )
{
    if ( sec == -1 )
	return;
    QString txt;
    switch( sec ) {
    case 0:
	txt = QString::number( d->h );
	break;
    case 1:
	txt = QString::number( d->m );
	break;
    case 2:
	txt = QString::number( d->s );
	break;
    }
    txt = txt.mid( 0, txt.length()-1 );
    switch( sec ) {
    case 0:
	d->h = txt.toInt();
	break;
    case 1:
	d->m = txt.toInt();
	break;
    case 2:
	d->s = txt.toInt();
	break;
    }
    repaint( rect(), FALSE );
}


/*! \reimp

*/

QSize QTimeEdit::sizeHint() const
{
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.width( 'x' ) * 10;
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 22 )
	    h = 22;
	return QSize( w + 8, h ).expandedTo( QApplication::globalStrut() );
    } else {
	return QSize( w + 4, h + 4 ).expandedTo( QApplication::globalStrut() );
    }
}


class QDateTimeEdit::Private
{
public:
    bool adv;
};

/*!

  \class QDateTimeEdit qdatetimeedit.h

  \brief The QDateTimeEdit class combines a QDateEdit and QTimeEdit
  widget into a single widget for editing datetimes.

  QDateTimeEdit consists of a QDateEdit and QTimeEdit widget placed
  side by side and offers the functionality of both. The user can edit
  the date and time by using the keyboard or the arrow keys to
  increase/decrease date or time values. The Tab key can be used to
  move from field to field within the QDateTimeEdit widget. Dates appear
  in year, month, day order by default, see setOrder() to change this.
  Times appear in the order hours, minutes, seconds using the 24 hour
  clock. It is recommended that the QDateTimeEdit is initialised with a
  datetime, e.g.

    \code
    QDateTimeEdit *datetimeedit = new QDateTimeEdit( QDateTime::currentDateTime(), this );
    dateedit->setRange( QDateTime::currentDateTime(),
			QDateTime::currentDateTime().addDays( 7 ) );
    \endcode

    Here we've created a new QDateTimeEdit object with a minimum
    date/time of now and a maximum date/time of a week from now.

  Terminology: A QDateEdit widget consists of three 'sections', one each
  for the year, month and day. Similarly a QTimeEdit consists of three
  sections, one each for the hour, minute and second.

  \sa QDateEdit QTimeEdit
*/

/*!  Constructs an empty datetime edit with parent \a parent and name
  \a name.

*/
QDateTimeEdit::QDateTimeEdit( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
}


/*!
  \overload
  Constructs a datetime edit with the initial value \a datetime, parent
  \a parent and name \a name.

*/
QDateTimeEdit::QDateTimeEdit( const QDateTime& datetime,
			      QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
    setDateTime( datetime );
}



/*! Destroys the object and frees any allocated resources.

*/

QDateTimeEdit::~QDateTimeEdit()
{
    delete d;
}


/*! \reimp

   Intercepts and handles resize events which have special meaning for
   the QDateTimeEdit.

*/

void QDateTimeEdit::resizeEvent( QResizeEvent * )
{
    layoutEditors();
}

/*! \reimp
*/

QSize QDateTimeEdit::minimumSizeHint() const
{
    QSize dsh = de->minimumSizeHint();
    QSize tsh = te->minimumSizeHint();
    return QSize( dsh.width() + tsh.width() + (frameWidth()*4),
		  QMAX( dsh.height(), tsh.height() ) );
}

/*! \internal

Moves and resizes the internal date and time editors.
*/

void QDateTimeEdit::layoutEditors()
{
    int h        = height() - frameWidth()*2;
    int dWidth = (width() - frameWidth()*2) * 9/16;
    int tWidth = (width() - frameWidth()*2) * 7/16;
    int fw       = frameWidth();

    de->resize( dWidth, h );
    te->resize( tWidth, h );

    de->move( fw, fw );
    te->move( de->x() + de->width() + fw, fw );
}

/*!  \internal
 */

void QDateTimeEdit::init()
{
    d = new Private();
    de = new QDateEdit( this );
    te = new QTimeEdit( this );
    d->adv = FALSE;
    connect( de, SIGNAL( valueChanged( const QDate& ) ),
	     this, SLOT( newValue( const QDate& ) ) );
    connect( te, SIGNAL( valueChanged( const QTime& ) ),
	     this, SLOT( newValue( const QTime& ) ) );
    setFocusProxy( de );
    layoutEditors();
}

/*! \reimp
 */

QSize QDateTimeEdit::sizeHint() const
{
    QSize dsh = de->sizeHint();
    QSize tsh = te->sizeHint();
    return QSize( dsh.width() + tsh.width() + (frameWidth()*4),
		  QMAX( dsh.height(), tsh.height() ) );
}

/*!  Sets the datetime to \a dt.  If \a dt is not valid, the editor
  displays all zeroes and QDateTimeEdit::dateTime() will return an invalid
  datetime.

*/

void QDateTimeEdit::setDateTime( const QDateTime & dt )
{
    if ( dt.isValid() ) {
	de->setDate( dt.date() );
	te->setTime( dt.time() );
	emit valueChanged( dt );
    }
}

/*!  Returns the current datetime in the editor, or an invalid
  datetime if the editor does not contain a valid datetime.

    \sa setDateTime()
*/

QDateTime QDateTimeEdit::dateTime() const
{
    return QDateTime( de->date(), te->time() );
}

/*!  Set the separator for the date in this QDateTimeEdit to \a s.

    \sa dateSeparator()
 */
void QDateTimeEdit::setDateSeparator( const QString & s )
{
    de->setSeparator( s );
}

/*!  Returns the separator for the date in this QDateTimeEdit.
    The default is "-".

    \sa setDateSeparator()

 */
QString QDateTimeEdit::dateSeparator() const
{
    return de->separator();
}

/*!  Set the separator for the time in this QDateTimeEdit to \a s.

    \sa timeSeparator()
 */
void QDateTimeEdit::setTimeSeparator( const QString & s )
{
    te->setSeparator( s );
}

/*!  Returns the separator for the time in this QDateTimeEdit. The
    default is ":".

    \sa setTimeSeparator()
 */
QString QDateTimeEdit::timeSeparator() const
{
    return te->separator();
}

/*! \fn void QDateTimeEdit::valueChanged( const QDateTime& datetime )

  This signal is emitted every time the date or time changes.  The
  \a datetime argument is the new datetime.
*/


/*! \internal

  Re-emits the value \a d.
 */

void QDateTimeEdit::newValue( const QDate& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}

/*! \internal
  \overload
  Re-emits the value \a t.
 */

void QDateTimeEdit::newValue( const QTime& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}


/*! Sets the auto advance property of the editor to \a advance.  If
  set to TRUE, the editor will automatically advance focus to the next
  date or time section if a user has completed a section.

*/

void QDateTimeEdit::setAutoAdvance( bool advance )
{
    de->setAutoAdvance( advance );
    te->setAutoAdvance( advance );
}

/*! Returns TRUE if auto-advance is enabled, otherwise returns FALSE.

  \sa setAutoAdvance()

*/

bool QDateTimeEdit::autoAdvance() const
{
    return de->autoAdvance();
}

/*!
  \reimp
*/
const QColor & QDateTimeEdit::foregroundColor() const
{
    return foregroundColorForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEdit::setForegroundColor( const QColor & color )
{
    setForegroundColorForMode(PaletteBase, color);
}

/*!
  \reimp
*/
const QColor & QDateTimeEdit::backgroundColor() const
{
    return backgroundColorForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEdit::setBackgroundColor( const QColor & color )
{
    setBackgroundColorForMode(PaletteBase, color);
}

/*!
  \reimp
*/
const QPixmap* QDateTimeEdit::backgroundPixmap() const
{
    return backgroundPixmapForMode(PaletteBase);
}

/*!
  \reimp
*/
void QDateTimeEdit::setBackgroundPixmap( const QPixmap & pixmap )
{
    setBackgroundPixmapForMode(PaletteBase, pixmap);
}


#endif
