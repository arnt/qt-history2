/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrbar.cpp#44 $
**
** Implementation of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qscrbar.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qbitmap.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qscrbar.cpp#44 $")


/*----------------------------------------------------------------------------
  \class QScrollBar qscrbar.h

  \brief The QScrollBar widget class provides a vertical or horizontal scroll
  bar.

  A scroll bar is used to let the user control a value within a
  program-definable range, and to give the user visible indication of
  the current value of a \link QRangeControl range control \endlink.

  QScrollBar only offers integer ranges, and at present only Motif
  look and feel is implemented.

  \ingroup realwidgets
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn void QScrollBar::valueChanged( int value )
  This signal is emitted when the scroll bar value is changed, with the
  new scroll bar value as an argument.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::sliderMoved( int value )
  This signal is emitted when the slider is dragged, with the
  new scroll bar value as an argument.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::nextLine()
  This signal is emitted when the scroll bar scrolls one line down/right.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::prevLine()
  This signal is emitted when the scroll bar scrolls one line up/left.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::nextPage()
  This signal is emitted when the scroll bar scrolls one page down/right.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::prevPage()
  This signal is emitted when the scroll bar scrolls one page up/left.
 ----------------------------------------------------------------------------*/


enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
		     SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
		     SLIDER   = 0x40, NONE     = 0x80 };


class QScrollBar_Private : public QScrollBar
{
public:
    void	  sliderMinMax( int *, int * )		const;
    void	  metrics( int *, int *, int * )	const;

    ScrollControl pointOver( const QPoint &p )		const;

    int		  rangeValueToSliderPos( int val )	const;
    int		  sliderPosToRangeValue( int  val )	const;

    void	  action( ScrollControl control );

    void	  drawControls( uint controls, uint activeControl ) const;
    void	  drawControls( uint controls, uint activeControl,
				QPainter *p ) const;
};


#undef PRIV
#define PRIV	((QScrollBar_Private *)this)

const int thresholdTime = 500;
const int repeatTime	= 100;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	6


/*----------------------------------------------------------------------------
  Constructs a vertical scroll bar.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QScrollBar::QScrollBar( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = Vertical;
    init();
}

/*----------------------------------------------------------------------------
  Constructs a scroll bar.

  The \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QScrollBar::QScrollBar( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name )
{
    orient = orientation;
    init();
}

/*----------------------------------------------------------------------------
  Constructs a scroll bar.

  \arg \e minValue is the minimum scroll bar value.
  \arg \e maxValue is the maximum scroll bar value.
  \arg \e lineStep is the line step value.
  \arg \e pageStep is the page step value.
  \arg \e value is the initial value.
  \arg \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QScrollBar::QScrollBar( int minValue, int maxValue, int lineStep, int pageStep,
			int value,  Orientation orientation,
			QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, lineStep, pageStep, value )
{
    orient = orientation;
    init();
}

void QScrollBar::init()
{
    track	     = TRUE;
    sliderPos	     = 0;
    pressedControl   = NONE;
    clickedAt	     = FALSE;
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}


/*----------------------------------------------------------------------------
  Sets the scroll bar orientation.  The \e orientation must be
  QScrollBar::Vertical or QScrollBar::Horizontal.
  \sa orientation()
 ----------------------------------------------------------------------------*/

void QScrollBar::setOrientation( Orientation orientation )
{
    orient = orientation;
    update();
}

/*----------------------------------------------------------------------------
  \fn Orientation QScrollBar::orientation() const
  Returns the scroll bar orientation; QScrollBar::Vertical or
  QScrollBar::Horizontal.
  \sa setOrientation()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QScrollBar::setTracking( bool enable )
  Enables scroll bar tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the scroll bar emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the scroll bar emits the valueChanged() signal
  when the user relases the mouse button (unless the value happens to
  be the same sa before).

  \sa tracking()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QScrollBar::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style scroll bars.
 ----------------------------------------------------------------------------*/

void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}


/*----------------------------------------------------------------------------
  \internal
  Implements the virtual QRangeControl function.
 ----------------------------------------------------------------------------*/

void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE , pressedControl );
    emit valueChanged(value());
}

/*----------------------------------------------------------------------------
  \internal
  Implements the virtual QRangeControl function.
 ----------------------------------------------------------------------------*/

void QScrollBar::stepChange()
{
    rangeChange();
}

/*----------------------------------------------------------------------------
  \internal
  Implements the virtual QRangeControl function.
 ----------------------------------------------------------------------------*/

void QScrollBar::rangeChange()
{
    positionSliderFromValue();
    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
}


/*----------------------------------------------------------------------------
  Handles timer events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::timerEvent( QTimerEvent * )
{
    if ( !isTiming )
	return;
    if ( !thresholdReached ) {
	thresholdReached = TRUE;	// control has been pressed for a time
	killTimers();			// kill the threshold time timer
	startTimer( repeatTime );	//   and start repeating
    }
    if ( clickedAt )
	PRIV->action( (ScrollControl) pressedControl );
}


/*----------------------------------------------------------------------------
  Handles key press events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::keyPressEvent( QKeyEvent * )
{
}


/*----------------------------------------------------------------------------
  Handles resize events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


/*----------------------------------------------------------------------------
  Handles paint events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    drawShadePanel( &p, rect(), colorGroup(), TRUE );
    PRIV->drawControls( ADD_LINE | SUB_LINE | ADD_PAGE | SUB_PAGE | SLIDER,
			pressedControl, &p );
    p.end();
}


/*----------------------------------------------------------------------------
  Handles mouse press events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    clickedAt	   = TRUE;
    pressedControl = PRIV->pointOver( e->pos() );
    switch( pressedControl ) {
	case SLIDER:
	    clickOffset = (QCOORD)( (HORIZONTAL ? e->pos().x() : e->pos().y())
				    - sliderPos );
	    slidePrevVal = value();
	    emit sliderPressed();
	    break;
	case NONE:
	    break;
	default:
	    PRIV->drawControls( pressedControl, pressedControl );
	    PRIV->action( (ScrollControl) pressedControl );
	    thresholdReached = FALSE;	// wait before starting repeat
	    startTimer(thresholdTime);
	    isTiming = TRUE;
	    break;
    }
}


/*----------------------------------------------------------------------------
  Handles mouse release events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    ScrollControl tmp = (ScrollControl) pressedControl;
    clickedAt	      = FALSE;
    if ( isTiming )
	killTimers();
    mouseMoveEvent( e );  // Might have moved since last mouse move event.
    pressedControl = NONE;

    switch( tmp ) {
	case SLIDER: // Set value directly, we know we don't have to redraw.
	    directSetValue( calculateValueFromSlider() );
	    emit sliderReleased();
	    if ( value() != prevValue() )
		emit valueChanged( value() );
	    break;
	case ADD_LINE:
	case SUB_LINE:
	    PRIV->drawControls( tmp, pressedControl );
	    break;
	default:
	    break;
    }
}


/*----------------------------------------------------------------------------
  Handles mouse move events for the scroll bar.
 ----------------------------------------------------------------------------*/

void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state() & LeftButton) )
	return;					// left mouse button is up

    int newSliderPos;
    if ( pressedControl == SLIDER ) {
	int sliderMin, sliderMax;
	PRIV->sliderMinMax( &sliderMin, &sliderMax );
	newSliderPos = (HORIZONTAL ? e->pos().x() : e->pos().y()) -clickOffset;
	if ( newSliderPos < sliderMin )
	    newSliderPos = sliderMin;
	else if ( newSliderPos > sliderMax )
	    newSliderPos = sliderMax;
	if ( newSliderPos != sliderPos ) {
	    int newVal = PRIV->sliderPosToRangeValue(newSliderPos);
	    if ( newVal != slidePrevVal )
		emit sliderMoved( newVal );
	    if ( track && newVal != value() ) {
		directSetValue( newVal ); // Set directly, painting done below
		emit valueChanged( value() );
	    }
	    slidePrevVal = newVal;
	    sliderPos = (QCOORD)newSliderPos;
	    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
	}
    }
}


/*----------------------------------------------------------------------------
  \fn int QScrollBar::sliderStart() const
  Returns the pixel position where the scroll bar slider starts.

  It is equivalent to sliderRect().y() for vertical
  scroll bars or sliderRect().x() for horizontal scroll bars.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the scroll bar slider rectangle.
  \sa sliderStart()
 ----------------------------------------------------------------------------*/

QRect QScrollBar::sliderRect() const
{
    int sliderMin, sliderMax, sliderLength;
    PRIV->metrics( &sliderMin, &sliderMax, &sliderLength );
    int b = style() == MotifStyle ? MOTIF_BORDER : 0;

    if ( HORIZONTAL )
	return QRect( sliderStart(), b,
		      sliderLength, height() - b*2 );
    else
	return QRect( b, sliderStart(),
		      width() - b*2, sliderLength );
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = (QCOORD)PRIV->rangeValueToSliderPos( value() );
}

int QScrollBar::calculateValueFromSlider() const
{
    return PRIV->sliderPosToRangeValue( sliderPos );
}


// --------------------------------------------------------------------------
// QScrollBar_Private member functions
//

void QScrollBar_Private::sliderMinMax( int *sliderMin, int *sliderMax) const
{
    int dummy;
    metrics( sliderMin, sliderMax, &dummy );
}


void QScrollBar_Private::metrics( int *sliderMin, int *sliderMax,
				  int *sliderLength ) const
{
    int buttonDim, maxLength;
    int b = style() == MotifStyle ? MOTIF_BORDER : 0;
    int length = HORIZONTAL ? width()  : height();
    int extent = HORIZONTAL ? height() : width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2 + SLIDER_MIN )
	buttonDim = extent - b*2;
    else
	buttonDim = ( length - b*2 - SLIDER_MIN )/2 - 1;

    *sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( maxValue() == minValue() ) {
	*sliderLength = maxLength;
    } else {
	*sliderLength = (pageStep()*maxLength)/
			(maxValue()-minValue()+pageStep());
	if ( *sliderLength < SLIDER_MIN )
	    *sliderLength = SLIDER_MIN;
	if ( *sliderLength > maxLength )
	    *sliderLength = maxLength;
    }
    *sliderMax = *sliderMin + maxLength - *sliderLength;
}


ScrollControl QScrollBar_Private::pointOver(const QPoint &p) const
{
    if ( !rect().contains( p ) )
	return NONE;
    int sliderMin, sliderMax, sliderLength, pos;
    metrics( &sliderMin, &sliderMax, &sliderLength );
    pos = HORIZONTAL ? p.x() : p.y();
    if ( pos < sliderMin )
	return SUB_LINE;
    if ( pos < sliderStart() )
	return SUB_PAGE;
    if ( pos < sliderStart() + sliderLength )
	return SLIDER;
    if ( pos < sliderMax + sliderLength )
	return ADD_PAGE;
    return ADD_LINE;
}


int QScrollBar_Private::rangeValueToSliderPos( int val ) const
{
    int smin, smax;
    sliderMinMax( &smin, &smax );
    if ( maxValue() == minValue() )
	return smin;
    int sliderMin=smin, sliderMax=smax;
    return ((sliderMax-sliderMin)*2*(val-minValue())+1)/
	   ((maxValue()-minValue())*2) + sliderMin;
}

int QScrollBar_Private::sliderPosToRangeValue( int pos ) const
{
    int sliderMin, sliderMax;
    sliderMinMax( &sliderMin, &sliderMax );
    if ( pos <= sliderMin || sliderMax == sliderMin )
	return minValue();
    if ( pos >= sliderMax )
	return maxValue();
    return (maxValue() - minValue() + 1)*(pos - sliderMin)/
	   (sliderMax - sliderMin) + minValue();
}


void QScrollBar_Private::action( ScrollControl control )
{
    switch( control ) {
	case ADD_LINE:
	    emit nextLine();
	    addLine();
	    break;
	case SUB_LINE:
	    emit prevLine();
	    subtractLine();
	    break;
	case ADD_PAGE:
	    emit nextPage();
	    addPage();
	    break;
	case SUB_PAGE:
	    emit prevPage();
	    subtractPage();
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QScrollBar_Private::action internal error" );
#endif
    }
}


void QScrollBar_Private::drawControls( uint controls,
				       uint activeControl ) const
{
    QPainter p;
    p.begin( this );
    drawControls( controls, activeControl, &p );
    p.end();
}


void QScrollBar_Private::drawControls( uint controls, uint activeControl,
				       QPainter *p ) const
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = colorGroup();

    int sliderMin, sliderMax, sliderLength;
    metrics( &sliderMin, &sliderMax, &sliderLength );

    int b = style() == MotifStyle ? MOTIF_BORDER : 0;
    int dimB = sliderMin - b;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? width()  : height();
    int extent = HORIZONTAL ? height() : width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = b;
	addX = length - dimB - b;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = b;
	addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart() + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, b,
			  sliderStart() - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart(), b, sliderLength, sliderW );
    } else {
	subPageR.setRect( b, subB.bottom() + 1, sliderW,
			  sliderStart() - subB.bottom() - 1 );
	addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( b, sliderStart(), sliderW, sliderLength );
    }

    switch ( style() ) {
	case WindowsStyle:
	    if ( controls & ADD_LINE ) {
		drawWinPanel( p, addB.x(), addB.y(),
			      addB.width(), addB.height(), g,
			      ADD_LINE_ACTIVE );
		qDrawArrow( p, VERTICAL ? DownArrow : RightArrow,WindowsStyle,
			    ADD_LINE_ACTIVE, addB.x()+2, addB.y()+2,
			    addB.width()-4, addB.height()-4, g );
	    }
	    if ( controls & SUB_LINE ) {
		drawWinPanel( p, subB.x(), subB.y(),
			      subB.width(), subB.height(), g,
			      SUB_LINE_ACTIVE );
		qDrawArrow( p, VERTICAL ? UpArrow : LeftArrow, WindowsStyle,
			    SUB_LINE_ACTIVE, subB.x()+2, subB.y()+2,
			    subB.width()-4, subB.height()-4, g );
	    }
	    p->setBrush( QBrush(white,Dense4Pattern) );
	    p->setPen( NoPen );
	    p->setBackgroundMode( OpaqueMode );
	    if ( controls & SUB_PAGE )
		p->drawRect( subPageR );
	    if ( controls & ADD_PAGE )
		p->drawRect( addPageR );
	    if ( controls & SLIDER ) {
		QBrush fill( g.background() );
		drawWinPanel( p, sliderR.x(), sliderR.y(),
			      sliderR.width(), sliderR.height(), g,
			      FALSE, &fill );
	    }
	    break;
	default:
	case MotifStyle:
	    if ( controls & ADD_LINE )
		qDrawArrow( p, VERTICAL ? DownArrow : RightArrow, MotifStyle,
			    ADD_LINE_ACTIVE, addB.x(), addB.y(),
			    addB.width(), addB.height(), g );
	    if ( controls & SUB_LINE )
		qDrawArrow( p, VERTICAL ? UpArrow : LeftArrow, MotifStyle,
			    SUB_LINE_ACTIVE, subB.x(), subB.y(),
			    subB.width(), subB.height(), g );
	    if ( controls & SUB_PAGE )
		p->fillRect( subPageR, g.mid() );
	    if ( controls & ADD_PAGE )
		p->fillRect( addPageR, g.mid() );
	    if ( controls & SLIDER ) {
		QBrush fill( g.background() );
		drawShadePanel( p, sliderR, g, FALSE, 2, &fill );
	    }
	    break;
    }
#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
}


static QPixmap *win_u_arrow = 0;
static QPixmap *win_d_arrow = 0;
static QPixmap *win_l_arrow = 0;
static QPixmap *win_r_arrow = 0;

static void cleanupWinArrows()
{
    delete win_u_arrow;
    delete win_d_arrow;
    delete win_l_arrow;
    delete win_r_arrow;
}

static void initWinArrows()
{
    static uchar u_bits[] = {0x00, 0x00, 0x08, 0x1c, 0x3e, 0x7f, 0x00, 0x00};
    static uchar d_bits[] = {0x00, 0x00, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x00};
    static uchar l_bits[] = {0x20, 0x30, 0x38, 0x3c, 0x38, 0x30, 0x20, 0x00};
    static uchar r_bits[] = {0x04, 0x0c, 0x1c, 0x3c, 0x1c, 0x0c, 0x04, 0x00};
    win_u_arrow = new QBitmap( 8, 8, (char *)u_bits, TRUE );
    win_d_arrow = new QBitmap( 8, 8, (char *)d_bits, TRUE );
    win_l_arrow = new QBitmap( 8, 8, (char *)l_bits, TRUE );
    win_r_arrow = new QBitmap( 8, 8, (char *)r_bits, TRUE );
    qAddPostRoutine( cleanupWinArrows );
}


static void qDrawWinArrow( QPainter *p, ArrowType type, bool down,
			   int x, int y, int w, int h,
			   const QColorGroup &g )
{
    if ( !win_u_arrow )
	initWinArrows();
    QPixmap *a = 0;
    switch ( type ) {
	case UpArrow:
	    a = win_u_arrow;
	    break;
	case DownArrow:
	    a = win_d_arrow;
	    break;
	case LeftArrow:
	    a = win_l_arrow;
	    break;
	case RightArrow:
	    a = win_r_arrow;
	    break;
    }
    if ( !a )
	return;
    QPen   oldPen   = p->pen();
    QBrush oldBrush = p->brush();
    p->setPen( NoPen );
    p->setBrush( g.background() );
    p->drawRect( x, y, w, h );
    p->setPen( g.foreground() );
    p->setBackgroundMode( TransparentMode );
    if ( down ) {
	x++;
	y++;
    }
    p->drawPixmap( x+w/2-4, y+h/2-4, *a );
    p->setBrush( oldBrush );
    p->setPen( oldPen );
}


#if defined(_CC_MSC_)
#pragma warning(disable: 4244)
#endif

static void qDrawMotifArrow( QPainter *p, ArrowType type, bool down,
			     int x, int y, int w, int h,
			     const QColorGroup &g )
{
    QPointArray bFill;				// fill polygon
    QPointArray bTop;				// top shadow.
    QPointArray bBot;				// bottom shadow.
    QPointArray bLeft;				// left shadow.
    QWMatrix	matrix;				// xform matrix
    bool vertical = type == UpArrow || type == DownArrow;
    bool horizontal = !vertical;
    int	 dim = w < h ? w : h;
    int	 colspec = 0x0000;			// color specification array

    if ( dim < 2 )				// too small arrow
	return;

    if ( dim > 3 ) {
	if ( dim > 6 )
	    bFill.resize( dim & 1 ? 3 : 4 );
	bTop.resize( (dim/2)*2 );
	bBot.resize( dim & 1 ? dim + 1 : dim );
	bLeft.resize( dim > 4 ? 4 : 2 );
	bLeft.putPoints( 0, 2, 0,0, 0,dim-1 );
	if ( dim > 4 )
	    bLeft.putPoints( 2, 2, 1,2, 1,dim-3 );
	bTop.putPoints( 0, 4, 1,0, 1,1, 2,1, 3,1 );
	bBot.putPoints( 0, 4, 1,dim-1, 1,dim-2, 2,dim-2, 3,dim-2 );

	for( int i=0; i<dim/2-2 ; i++ ) {
	    bTop.putPoints( i*2+4, 2, 2+i*2,2+i, 5+i*2, 2+i );
	    bBot.putPoints( i*2+4, 2, 2+i*2,dim-3-i, 5+i*2,dim-3-i );
	}
	if ( dim & 1 )				// odd number size: extra line
	    bBot.putPoints( dim-1, 2, dim-3,dim/2, dim-1,dim/2 );
	if ( dim > 6 ) {			// dim>6: must fill interior
	    bFill.putPoints( 0, 2, 1,dim-3, 1,2 );
	    if ( dim & 1 )			// if size is an odd number
		bFill.setPoint( 2, dim - 3, dim / 2 );
	    else
		bFill.putPoints( 2, 2, dim-4,dim/2-1, dim-4,dim/2 );
	}
    }
    else {
	if ( dim == 3 ) {			// 3x3 arrow pattern
	    bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
	    bTop .setPoints( 2, 1,0, 1,0 );
	    bBot .setPoints( 2, 1,2, 2,1 );
	}
	else {					// 2x2 arrow pattern
	    bLeft.setPoints( 2, 0,0, 0,1 );
	    bTop .setPoints( 2, 1,0, 1,0 );
	    bBot .setPoints( 2, 1,1, 1,1 );
	}
    }

    if ( type == UpArrow || type == LeftArrow ) {
	matrix.translate( x, y );
	if ( vertical ) {
	    matrix.translate( 0, h - 1 );
	    matrix.rotate( -90 );
	} else {
	    matrix.translate( w - 1, h - 1 );
	    matrix.rotate( 180 );
	}
	if ( down )
	    colspec = horizontal ? 0x2334 : 0x2343;
	else
	    colspec = horizontal ? 0x1443 : 0x1434;
    }
    else if ( type == DownArrow || type == RightArrow ) {
	matrix.translate( x, y );
	if ( vertical ) {
	    matrix.translate( w-1, 0 );
	    matrix.rotate( 90 );
	}
	if ( down )
	    colspec = horizontal ? 0x2443 : 0x2434;
	else
	    colspec = horizontal ? 0x1334 : 0x1343;
    }

    QColor *cols[5];
    cols[0] = 0;
    cols[1] = (QColor *)&g.background();
    cols[2] = (QColor *)&g.mid();
    cols[3] = (QColor *)&g.light();
    cols[4] = (QColor *)&g.dark();
#define CMID	*cols[ (colspec>>12) & 0xf ]
#define CLEFT	*cols[ (colspec>>8) & 0xf ]
#define CTOP	*cols[ (colspec>>4) & 0xf ]
#define CBOT	*cols[ colspec & 0xf ]

    QPen   savePen   = p->pen();		// save current pen
    QBrush saveBrush = p->brush();		// save current brush
    QPen   pen( NoPen );
    QBrush brush( CMID );

    p->setPen( pen );
    p->setBrush( brush );
    p->setWorldMatrix( matrix );		// set transformation matrix
    p->drawPolygon( bFill );			// fill arrow
    p->setBrush( NoBrush );			// don't fill

    p->setPen( CLEFT );
    p->drawLineSegments( bLeft );
    p->setPen( CTOP );
    p->drawLineSegments( bTop );
    p->setPen( CBOT );
    p->drawLineSegments( bBot );

    p->setWorldXForm( FALSE );			// turn off xform
    p->setBrush( saveBrush );			// restore brush
    p->setPen( savePen );			// restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
}


void qDrawArrow( QPainter *p, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h, const QColorGroup &g )
{
    switch ( style ) {
	case WindowsStyle:
	    qDrawWinArrow( p, type, down, x, y, w, h, g );
	    break;
	case MotifStyle:
	    qDrawMotifArrow( p, type, down, x, y, w, h, g );
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "qDrawArrow: Requested GUI style not supported" );
#endif
    }
}
