/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.cpp#5 $
**
** Implementation of QLineEdit class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (c) 1994 by Troll Tech AS.	 All rights reserved.
**
***********************************************************************/

#include "qlined.h"
#include "qpainter.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlineedit.cpp#5 $";
#endif


static const int blinkTime = 500;

#define LEFT_MARGIN 4
#define RIGHT_MARGIN 4
#define TOP_MARGIN 4
#define BOTTOM_MARGIN 4


static uint xPosToCursorPos( char *s, const QFont &f, uint xPos, uint width )
{
    QFontMetrics  fm( f );
    char	 *tmp;
    int		  dist;

    if( xPos > width )
	xPos = width;
    dist = xPos;
    tmp	 = s;
    while( *tmp && dist >= 0 )
	dist -= fm.width( tmp++, 1 );
    if( dist < 0 && ( xPos - dist > width ||
		      fm.width( tmp - 1, 1)/2 < -dist ) )
	tmp--;
    return tmp - s;
}

static uint showLastPartOffset( char *s, const QFont &f, int width )
{
    if ( !s || s[0] == '\0' )
	return 0;

    QFontMetrics  fm( f );
    char	 *tmp = &s[strlen( s ) - 1];

    do {
	width -= fm.width( tmp--, 1 );
    } while ( tmp >=s && width >=0 );

    return width < 0 ? tmp - s + 2 : 0;
}


QLineEdit::QLineEdit( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    init();
}

void QLineEdit::init()
{
    pm		= 0;
    cursorPos	= 0;
    offset	= 0;
    maxLen	= ~0;
    cursorOn	= TRUE;
    inTextFocus = FALSE;
    t		= "";
    setBackgroundColor( lightGray );
}

QLineEdit::~QLineEdit()
{
    delete pm;
}


void QLineEdit::keyFocusEvent( bool inFocus )
{
    if ( inFocus && inTextFocus || !inFocus && !inTextFocus )
	return;

    inTextFocus = inFocus;
    if ( inFocus ) {
	pm = new QPixMap( clientSize().width(), clientSize().height());
	CHECK_PTR( pm );
	startTimer( blinkTime );
    } else {
	killTimers();
	delete pm;
	pm = 0;
    }
    cursorOn = TRUE;
    paint();
}


void QLineEdit::setText( const char *s )
{
    if ( t == s )				// no change
	return;
    t = s;
    if ( t.length() > maxLen )
	t.resize( maxLen+1 );
    cursorPos = 0;
    offset    = 0;
    paint();
    emit textChanged( t.data() );
}

char *QLineEdit::text() const
{
    return t.data();
}


void QLineEdit::setMaxLength( int m )
{
    maxLen = (uint) m;
    if ( t.length() > maxLen ) {
	t.resize( maxLen + 1 ); // Include \0
	if ( cursorPos > maxLen ) {
	    offset = maxLen;
	    end();
	}
	paint();
    }
}

int QLineEdit::maxLength() const
{
    return maxLen;
}


bool QLineEdit::event( QEvent *e )
{
    if ( e->type() == Event_Enter ) {
	keyFocusEvent( TRUE );
	return TRUE;
    }
    if ( e->type() == Event_Leave ) {
	keyFocusEvent( FALSE );
	return TRUE;
    }
    return QWidget::event( e );
}


bool QLineEdit::keyPressEvent( QKeyEvent *k )
{
    if ( !inTextFocus )
	keyFocusEvent( TRUE );
    if ( k->ascii() >= 32 && k->key() != Key_Delete ) {
	if ( t.length() >= maxLen )
	    return TRUE;
	t.insert( cursorPos, k->ascii() );
	cursorRight();
	paint();
	emit textChanged( t.data() );
	return TRUE;
    }
    bool p = FALSE;
    switch ( k->key() ) {
	case Key_Left:
		 p = cursorLeft();
		 break;
	case Key_Right:
		 p = cursorRight();
		 break;
	case Key_Backspace:
		 p = backspace();
		 if ( p )
		     emit textChanged( t.data() );
		 break;
	case Key_Home:
		 p = home();
		 break;
	case Key_End:
		 p = end();
		 break;
	case Key_Delete:
		 p = remove();
		 if ( p )
		     emit textChanged( t.data() );
		 break;
	case Key_A:
		 if ( k->state() == ControlButton )
		     p = home();
		 break;
	case Key_B:
		 if ( k->state() == ControlButton )
		     p = cursorLeft();
		 break;
	case Key_D:
		 if ( k->state() == ControlButton ) {
		     p = remove();
		     if ( p )
			 emit textChanged( t.data() );
		 }
		 break;
	case Key_E:
		 if ( k->state() == ControlButton )
		     p = end();
		 break;
	case Key_F:
		 if ( k->state() == ControlButton )
		     p = cursorRight();
		 break;
	case Key_H:
		 if ( k->state() == ControlButton ) {
		     p = backspace();
		     if ( p )
			 emit textChanged( t.data() );
		 }
	default:
		 return FALSE;
		 break;
    }

    if ( p )
	paint();
    return TRUE;
}


void QLineEdit::paintEvent( QPaintEvent * )
{
    paint( TRUE );
}


void QLineEdit::timerEvent( QTimerEvent * )
{
    if ( inTextFocus ) {
	cursorOn = !cursorOn;
	paint();
    }
}


void QLineEdit::resizeEvent( QResizeEvent *rs )
{
    if ( inTextFocus ) {
	delete pm;
	pm = new QPixMap( rs->size().width(), rs->size().height() );
    }
    paint();
}


void QLineEdit::mousePressEvent( QMouseEvent *m )
{
    cursorPos = offset +
		xPosToCursorPos( &t[ offset ], font(),
			    m->pos().x() - LEFT_MARGIN,
			    clientSize().width() - LEFT_MARGIN - RIGHT_MARGIN);
    if ( !inTextFocus )
	keyFocusEvent( TRUE );
    else
	paint();
}


void QLineEdit::paint( bool frame )
{
    QPainter p;

    if ( inTextFocus ) {
	pixmapPaint();
    } else {
	p.begin( this );
	p.setFont( font() );
	if ( !frame )
	    p.eraseRect( QRect( LEFT_MARGIN, TOP_MARGIN,
			 clientWidth()	- LEFT_MARGIN - RIGHT_MARGIN,
			 clientHeight() - TOP_MARGIN  - BOTTOM_MARGIN));
	paintText( &p, clientSize(), frame );
	p.end();
    }
}


void QLineEdit::pixmapPaint()
{
    QPainter p;

    p.begin( pm );
    p.setFont( font() );
    p.fillRect( clientRect(), backgroundColor() );
    paintText( &p, pm->size() , TRUE );
    p.end();
    p.begin( this );
    p.drawPixMap( 0, 0 , *pm );
    p.end();
}


void QLineEdit::paintText( QPainter *p, const QSize &sz, bool frame)
{
    QFontMetrics fm( font() );
    char *displayText = &t[ offset ];

    if ( frame )
	p->drawShadePanel( QRect( QPoint( 0, 0 ), sz ),
			   backgroundColor().dark(),
			   backgroundColor().light());
    p->setClipRect( QRect(
		      LEFT_MARGIN, TOP_MARGIN,
		      sz.width()  - LEFT_MARGIN - RIGHT_MARGIN + 1,
		      sz.height() - TOP_MARGIN - BOTTOM_MARGIN + 1) );

    int tDispWidth = sz.width() - LEFT_MARGIN - RIGHT_MARGIN;
    int displayLength = xPosToCursorPos( displayText, font(),
					 tDispWidth, tDispWidth );
    if ( displayText[ displayLength ] != '\0' )
	displayLength++;

    p->drawText( LEFT_MARGIN, sz.height() - BOTTOM_MARGIN - fm.descent(),
		 displayText, displayLength );

    p->setClipping( FALSE );
    if( cursorOn ) {
	uint curPos = LEFT_MARGIN +
		      fm.width( displayText, cursorPos - offset ) - 1;
	if ( style() == MotifStyle ) {
	    if ( !inTextFocus ) {
		p->pen().setStyle( DotLine );
		p->setBackgroundMode( OpaqueMode );
	    }
	    p->drawLine( curPos - 2, TOP_MARGIN, curPos + 2, TOP_MARGIN );
	    p->drawLine( curPos	  , TOP_MARGIN,
			 curPos	  , sz.height() - BOTTOM_MARGIN );
	    p->drawLine( curPos - 2, sz.height() - BOTTOM_MARGIN,
			 curPos + 2, sz.height() - BOTTOM_MARGIN );
	} else {
	    p->drawLine( curPos	  , TOP_MARGIN,
			 curPos	  , sz.height() - BOTTOM_MARGIN );
	}
    }
}


bool QLineEdit::cursorLeft()
{
    if ( cursorPos != 0 ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos--;
	if ( cursorPos < offset )
	    offset = cursorPos;
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}


bool QLineEdit::cursorRight()
{
    QFontMetrics fm(font());

    if ( strlen( t ) > cursorPos ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos++;
	int surplusWidth = clientSize().width() - LEFT_MARGIN - RIGHT_MARGIN
			   - fm.width( &t[ offset ], cursorPos - offset);
	if ( surplusWidth < 0 ) {
	    while ( surplusWidth < 0 && offset < cursorPos - 1 ) {
		surplusWidth += fm.width( &t[ offset ], 1 );
		offset++;
	    }
	}
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}


bool QLineEdit::backspace()
{
    return cursorLeft() ? remove() : FALSE;
}


bool QLineEdit::remove()
{
    if ( cursorPos != strlen(t) ) {
	t.remove( cursorPos, 1 );
	return TRUE;
    }
    return FALSE;
}


bool QLineEdit::home()
{
    if ( cursorPos != 0 ) {
	cursorPos = 0;
	offset	  = 0;
	return TRUE;
    }
    return FALSE;
}


bool QLineEdit::end()
{
    if ( cursorPos != strlen(t) ) {
	offset += showLastPartOffset( &t[offset], font(),
		      clientSize().width() - LEFT_MARGIN - RIGHT_MARGIN );
	cursorPos = strlen( t );
	return TRUE;
    }
    return FALSE;
}
