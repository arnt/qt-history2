/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrbar.cpp#67 $
**
** Implementation of QScrollBar class
**
** Created : 940427
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qscrbar.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qbitmap.h"
#include "qkeycode.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qscrbar.cpp#67 $");


/*!
  \class QScrollBar qscrbar.h

  \brief The QScrollBar widget class provides a vertical or horizontal scroll
  bar.

  A scroll bar is used to let the user control a value within a
  program-definable range, and to give the user visible indication of
  the current value of a \link QRangeControl range control \endlink.

  QScrollBar only offers integer ranges.

  The recommended thickness of a scroll bar is 16 pixels.

  A scroll bar can be controlled by the keyboard, but it has a
  default focusPolicy() of \a NoFocus. Use setFocusPolicy() to
  enable keyboard focus.

  \ingroup realwidgets
*/


/*!
  \fn void QScrollBar::valueChanged( int value )
  This signal is emitted when the scroll bar value is changed, with the
  new scroll bar value as an argument.
*/

/*!
  \fn void QScrollBar::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
*/

/*!
  \fn void QScrollBar::sliderMoved( int value )
  This signal is emitted when the slider is dragged, with the
  new scroll bar value as an argument.
*/

/*!
  \fn void QScrollBar::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
*/

/*!
  \fn void QScrollBar::nextLine()
  This signal is emitted when the scroll bar scrolls one line down/right.
*/

/*!
  \fn void QScrollBar::prevLine()
  This signal is emitted when the scroll bar scrolls one line up/left.
*/

/*!
  \fn void QScrollBar::nextPage()
  This signal is emitted when the scroll bar scrolls one page down/right.
*/

/*!
  \fn void QScrollBar::prevPage()
  This signal is emitted when the scroll bar scrolls one page up/left.
*/


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

static const int thresholdTime = 500;
static const int repeatTime	= 100;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9 // ### motif says 6 but that's too small


/*!
  Constructs a vertical scroll bar.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = Vertical;
    init();
}

/*!
  Constructs a scroll bar.

  The \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name )
{
    orient = orientation;
    init();
}

/*!
  Constructs a scroll bar.

  \arg \e minValue is the minimum scroll bar value.
  \arg \e maxValue is the maximum scroll bar value.
  \arg \e lineStep is the line step value.
  \arg \e pageStep is the page step value.
  \arg \e value is the initial value.
  \arg \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

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
    setFocusPolicy( NoFocus );
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}


/*!
  Sets the scroll bar orientation.  The \e orientation must be
  QScrollBar::Vertical or QScrollBar::Horizontal.
  \sa orientation()
*/

void QScrollBar::setOrientation( Orientation orientation )
{
    orient = orientation;
    positionSliderFromValue();
    update();
}

/*!
  \fn Orientation QScrollBar::orientation() const
  Returns the scroll bar orientation; QScrollBar::Vertical or
  QScrollBar::Horizontal.
  \sa setOrientation()
*/

/*!
  \fn void QScrollBar::setTracking( bool enable )
  Enables scroll bar tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the scroll bar emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the scroll bar emits the valueChanged() signal
  when the user relases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

/*!
  \fn bool QScrollBar::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
*/


/*!
  Returns TRUE if the user has clicked the mouse on the slider
  and is currenly dragging it, or FALSE if not.
*/

bool QScrollBar::draggingSlider() const
{
    return pressedControl == SLIDER;
}


/*!
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style scroll bars.
*/

void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}


/*!
  Returns a size hint for this scroll bar.
*/

QSize QScrollBar::sizeHint() const
{
    QSize s( size() );
    if ( orient == Horizontal ) {
	s.setHeight( 16 );
    } else {
	s.setWidth( 16 );
    }
    return s;
}


/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE , pressedControl );
    emit valueChanged(value());
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::stepChange()
{
    rangeChange();
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::rangeChange()
{
    positionSliderFromValue();
    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
}


/*!
  Handles timer events for the scroll bar.
*/

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


/*!
  Handles key press events for the scroll bar.
*/

void QScrollBar::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	if ( orient == Horizontal )
	    setValue( value() - lineStep() );
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    setValue( value() + lineStep() );
	break;
    case Key_Up:
	if ( orient == Vertical )
	    setValue( value() - lineStep() );
	break;
    case Key_Down:
	if ( orient == Vertical )
	    setValue( value() + lineStep() );
	break;
    case Key_PageUp:
	if ( orient == Vertical )
	    setValue( value() - pageStep() );
	break;
    case Key_PageDown:
	if ( orient == Vertical )
	    setValue( value() + pageStep() );
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	break;
    }
}


/*!
  Handles resize events for the scroll bar.
*/

void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


/*!
  Handles paint events for the scroll bar.
*/

void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    qDrawShadePanel( &p, rect(), colorGroup(), TRUE );
    if ( hasFocus() ) {
	if ( style() != WindowsStyle ) {
	    p.setPen( black );
	    p.drawRect(  1, 1, width() - 2, height() - 2 );
	}
    }
    PRIV->drawControls( ADD_LINE | SUB_LINE | ADD_PAGE | SUB_PAGE | SLIDER,
			pressedControl, &p );
    p.end();
}


static QCOORD sliderStartPos = 0;

/*!
  Handles mouse press events for the scroll bar.
*/

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
	    slidePrevVal   = value();
	    sliderStartPos = sliderPos;
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


/*!
  Handles mouse release events for the scroll bar.
*/

void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton || !clickedAt )
	return;
    ScrollControl tmp = (ScrollControl) pressedControl;
    clickedAt = FALSE;
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


/*!
  Handles mouse move events for the scroll bar.
*/

void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !isVisible() ) {
	clickedAt = FALSE;
	return;
    }
    if ( !(e->state() & LeftButton) || !clickedAt )
	return;
    int newSliderPos;
    if ( pressedControl == SLIDER ) {
	int sliderMin, sliderMax;
	PRIV->sliderMinMax( &sliderMin, &sliderMax );
	QRect r = rect();
	if ( orientation() == Horizontal )
	    r.setRect( r.x() - 20, r.y() - 30, r.width() + 40, r.height() + 60 );
	else
	    r.setRect( r.x() - 30, r.y() - 20, r.width() + 60, r.height() + 40 );
	if ( style() == WindowsStyle && !r.contains( e->pos() ) )
	    newSliderPos = sliderStartPos;
        else
	    newSliderPos = (HORIZONTAL ? e->pos().x() : 
			                 e->pos().y()) -clickOffset;
	if ( newSliderPos < sliderMin )
	    newSliderPos = sliderMin;
	else if ( newSliderPos > sliderMax )
	    newSliderPos = sliderMax;
	if ( newSliderPos == sliderPos )
	    return;
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


/*!
  \fn int QScrollBar::sliderStart() const
  Returns the pixel position where the scroll bar slider starts.

  It is equivalent to sliderRect().y() for vertical
  scroll bars or sliderRect().x() for horizontal scroll bars.
*/

/*!
  Returns the scroll bar slider rectangle.
  \sa sliderStart()
*/

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


/*****************************************************************************
  QScrollBar_Private member functions
 *****************************************************************************/

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


int QScrollBar_Private::rangeValueToSliderPos( int v ) const
{
    int smin, smax;
    sliderMinMax( &smin, &smax );
    if ( maxValue() == minValue() )
	return smin;
    int sliderMin=smin, sliderMax=smax;
    return ((sliderMax-sliderMin)*2*(v-minValue())+1)/
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
		qDrawWinPanel( p, addB.x(), addB.y(),
			       addB.width(), addB.height(), g,
			       ADD_LINE_ACTIVE );
		qDrawArrow( p, VERTICAL ? DownArrow : RightArrow,WindowsStyle,
			    ADD_LINE_ACTIVE, addB.x()+2, addB.y()+2,
			    addB.width()-4, addB.height()-4, g );
	    }
	    if ( controls & SUB_LINE ) {
		qDrawWinPanel( p, subB.x(), subB.y(),
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
		qDrawWinPanel( p, sliderR.x(), sliderR.y(),
			       sliderR.width(), sliderR.height(), g,
			       FALSE, &fill );
		if ( hasFocus() ) {
		    //### drawWinFocusFrame
		    qDrawPlainRect( p, sliderR.x() + 2 , sliderR.y() + 2,
		    	       sliderR.width() - 5, sliderR.height() - 5, black );
		}
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
		qDrawShadePanel( p, sliderR, g, FALSE, 2, &fill );
	    }
	    break;
    }
#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
}


static void qDrawWinArrow( QPainter *p, ArrowType type, bool down,
			   int x, int y, int w, int h,
			   const QColorGroup &g )
{
    QPointArray a;				// arrow polygon
    switch ( type ) {
	case UpArrow:
	    a.setPoints( 7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2 );
	    break;
	case DownArrow:
	    a.setPoints( 7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2 );
	    break;
	case LeftArrow:
	    a.setPoints( 7, 1,-3, 1,3, 0,-2, 0,2, -1,-1, -1,1, -2,0 );
	    break;
	case RightArrow:
	    a.setPoints( 7, -1,-3, -1,3, 0,-2, 0,2, 1,-1, 1,1, 2,0 );
	    break;
    }
    if ( a.isNull() )
	return;

    if ( down ) {
	x++;
	y++;
    }
    a.translate( x+w/2, y+h/2 );

    QPen savePen = p->pen();			// save current pen
    p->fillRect( x, y, w, h, g.background() );
    p->setPen( g.foreground() );
    p->drawLineSegments( a, 0, 3 );		// draw arrow
    p->drawPoint( a[6] );
    p->setPen( savePen );			// restore pen
}


#if defined(_CC_MSVC_)
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

    QPen     savePen   = p->pen();		// save current pen
    QBrush   saveBrush = p->brush();		// save current brush
    QWMatrix wxm = p->worldMatrix();
    QPen     pen( NoPen );
    QBrush   brush( CMID );

    p->setPen( pen );
    p->setBrush( brush );
    p->setWorldMatrix( matrix, TRUE );		// set transformation matrix
    p->drawPolygon( bFill );			// fill arrow
    p->setBrush( NoBrush );			// don't fill

    p->setPen( CLEFT );
    p->drawLineSegments( bLeft );
    p->setPen( CTOP );
    p->drawLineSegments( bTop );
    p->setPen( CBOT );
    p->drawLineSegments( bBot );

    p->setWorldMatrix( wxm );
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
