/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrbar.cpp#3 $
**
** Implementation of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qscrbar.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qwxfmat.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qscrbar.cpp#3 $";
#endif


//
// NOTE!!! THIS CODE IS NOT FINAL. SOME TESTING & TUNING REMAINS!
//

enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
		     SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
		     SLIDER   = 0x40, NONE     = 0x80 };


class QScrollBar_Private : public QScrollBar
{
public:
    void	  sliderMinMax( int *, int * ) const;
    void	  metrics( int *, int *, int * ) const;

    ScrollControl pointOver( const QPoint &p ) const;

    int		  rangeValueToSliderPos( int val ) const;
    int		  sliderPosToRangeValue( int val ) const;

    void	  action( ScrollControl control );

    void	  drawControls( uint controls, uint activeControl ) const;
    void	  drawControls( uint controls, uint activeControl,
				QPainter &p ) const;
};


#undef PRIV
#define PRIV ( ( QScrollBar_Private * ) this )

const int thresholdTime = 500;
const int repeatTime	= 100;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define BORDER		2
#define SLIDER_MIN	6


QScrollBar::QScrollBar( QView *parent,Orientation d ) : QWidget( parent )
{
    orient = d;
    initialize();
}

QScrollBar::QScrollBar( int minVal, int maxVal, int lineStep, int pageStep,
			int value, QView *parent, Orientation d )
   : QWidget(parent) ,QRangeControl(minVal, maxVal, lineStep, pageStep, value)
{
    orient = d;
    initialize();
}

void QScrollBar::initialize()
{
    track	     = TRUE;
    sliderPos	     = 0;
    pressedControl   = NONE;
    clickedAt	     = FALSE;
    setForegroundColor(lightGray);
    setBackgroundColor(foregroundColor().dark(1.12));
}


void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE , pressedControl );
    emit newValue(value());
}

void QScrollBar::stepChange()
{
    rangeChange();
}

void QScrollBar::rangeChange()
{
    positionSliderFromValue();
    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
}


void QScrollBar::timerEvent( QTimerEvent * )
{
    if ( !isTiming )
	return;
    if ( !thresholdReached ) {
	thresholdReached = TRUE;  // control has been pressed for a time
	killTimers();		  // kill the threshold time timer
	startTimer( repeatTime );   // and start repeating
    }
    if ( clickedAt )
	PRIV->action( (ScrollControl) pressedControl );
}


bool QScrollBar::keyPressEvent( QKeyEvent * )
{
    return TRUE;
}


void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    p.drawShadePanel( clientRect(), foregroundColor().dark(),
		      foregroundColor().light() );
    PRIV->drawControls( ADD_LINE | SUB_LINE | ADD_PAGE | SUB_PAGE | SLIDER,
			pressedControl, p );
    p.end();
}


void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    clickedAt	   = TRUE;
    pressedControl = PRIV->pointOver( e->pos() );
    switch( pressedControl ) {
	case SLIDER:
		 clickOffset = ( HORIZONTAL ? e->pos().x() : e->pos().y() )
			       - sliderPos;
		 break;
	case NONE:
		 break;
	default:
		 PRIV->drawControls( pressedControl, pressedControl );
		 PRIV->action( (ScrollControl) pressedControl );
		 thresholdReached = FALSE; // wait before starting repeat
		 startTimer(thresholdTime);
		 isTiming = TRUE;
		 break;
    }
}


void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    ScrollControl tmp = (ScrollControl) pressedControl;
    clickedAt	      = FALSE;
    if ( isTiming )
	killTimers();
    mouseMoveEvent( e );  // Might have moved since last mouse move event.
    pressedControl = NONE;

    switch( tmp ) {
	case SLIDER: // Set value directly, we know we don't have to redraw.
	    directSetValue( calculateValueFromSlider() );
	    if ( value() != previousValue() )
		emit newValue( value() );
	    break;
	case ADD_LINE:
	case SUB_LINE:
	    PRIV->drawControls( tmp, pressedControl );
	    break;
	default:
	    break;
    }
}


void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
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
	    if ( track && newVal != value() ) {
		directSetValue( newVal ); // Set directly, painting done below
		emit newValue( value() );
	    }
	    sliderPos = newSliderPos;
	    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
	}
    }
}


void QScrollBar::positionSliderFromValue()
{
    sliderPos = PRIV->rangeValueToSliderPos( value() );
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

    int length = HORIZONTAL ? clientWidth()  : clientHeight();
    int width = HORIZONTAL ? clientHeight() : clientWidth();

    if ( length > ( width - BORDER*2 - 1 )*2 + BORDER*2 + SLIDER_MIN )
	buttonDim = width - BORDER*2;
    else
	buttonDim = ( length - BORDER*2 - SLIDER_MIN )/2 - 1;

    *sliderMin	  = BORDER + buttonDim;
    maxLength	  = length - BORDER*2 - buttonDim*2;

    if ( maxValue() == minValue() ) {
	*sliderLength = maxLength;
    } else {
	*sliderLength = maxLength*pageStep()/
			    ( maxValue() - minValue() + pageStep() );
	if ( *sliderLength < SLIDER_MIN )
	    *sliderLength = SLIDER_MIN;
	if ( *sliderLength > maxLength )
	    *sliderLength = maxLength;
    }
    *sliderMax = *sliderMin + maxLength - *sliderLength;
//    debug( "metrics: min = %3i, max = %3i, len = %3i, start = %3i",
//	     *sliderMin, *sliderMax,*sliderLength, sliderStart());
}


ScrollControl QScrollBar_Private::pointOver(const QPoint &p) const
{
    if ( !clientRect().contains( p ) )
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
    int sliderMin, sliderMax;
    sliderMinMax( &sliderMin, &sliderMax );
    if ( maxValue() == minValue() )
	return sliderMin;
#if 0	// ###!!! DEBUGGING
    debug( "rangeValueToSliderPos, val = %3i, pos = %3i ", val ,
	   (sliderMax - sliderMin)*2*( val - minValue() + 1 )/
	   ( ( maxValue() - minValue() )*2 ) + sliderMin);
#endif
    return ( ( sliderMax - sliderMin )*2*( val - minValue() ) + 1 )/
		( 2*( maxValue() - minValue() ) ) + sliderMin;
}

int QScrollBar_Private::sliderPosToRangeValue( int pos ) const
{
    int sliderMin, sliderMax;
    sliderMinMax( &sliderMin, &sliderMax );
    if ( pos <= sliderMin || sliderMax == sliderMin )
	return minValue();
    if ( pos >= sliderMax )
	return maxValue();
    return ( maxValue() - minValue() + 1 )*( pos - sliderMin ) /
		( sliderMax - sliderMin ) + minValue();
}


void QScrollBar_Private::action( ScrollControl control )
{
    switch( control ) {
	case ADD_LINE:
	    addLine();
	    break;
	case SUB_LINE:
	    subtractLine();
	    break;
	case ADD_PAGE:
	    addPage();
	    break;
	case SUB_PAGE:
	    subtractPage();
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QScrollBar: Internal action error" );
#endif
    }
}

void QScrollBar_Private::drawControls(uint controls, uint activeControl) const
{
    QPainter p;

    p.begin( this );
    drawControls( controls, activeControl, p );
    p.end();
}


void QScrollBar_Private::drawControls( uint controls, uint activeControl,
				       QPainter &p ) const
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColor shadowC = foregroundColor().dark();
    QColor lightC  = foregroundColor().light();
    QColor upC	   = foregroundColor();
    QColor downC   = backgroundColor();

    int i;
    int sliderMin, sliderMax, sliderLength;
    metrics( &sliderMin, &sliderMax, &sliderLength );

    int dimB = sliderMin - BORDER;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? clientWidth()  : clientHeight();
    int width  = HORIZONTAL ? clientHeight() : clientWidth();

    if ( HORIZONTAL ) {
	subY = addY = ( width - dimB ) / 2;
	subX = BORDER;
	addX = length - dimB - BORDER;
    } else {
	subX = addX = ( width - dimB ) / 2;
	subY = BORDER;
	addY = length - dimB - BORDER;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart() + sliderLength;
    int sliderW = width - BORDER*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, BORDER,
			  sliderStart() - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, BORDER, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart(), BORDER, sliderLength, sliderW );
    } else {
	subPageR.setRect( BORDER, subB.bottom() + 1, sliderW,
			  sliderStart() - subB.bottom() - 1 );
	addPageR.setRect( BORDER, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( BORDER, sliderStart(), sliderW, sliderLength );
    }

#if 0
    showRect( "subB", subB );		// FOR DEBUGGING!!!###
    showRect( "addB", addB );
    showRect( "subPageR", subPageR );
    showRect( "addPageR", addPageR );
    showRect( "sliderR" , sliderR );
#endif

    switch ( style() ) {
	default:
	case MotifStyle: {
	    QPointArray bFill;			// Button fill polygon
	    QPointArray bTop;			// Button top shadow.
	    QPointArray bBot;			// Button bottom shadow.
	    QPointArray bLeft;			// Button left shadow.

	    if ( (controls & (SUB_LINE | ADD_LINE)) && dimB > 1 ) {

		if ( dimB > 6 )
		    bFill.resize( dimB & 1 ? 3 : 4 );

		if ( dimB > 3 ) {
		    bTop.resize( ( dimB/2 )*2 );
		    bBot.resize( dimB & 1 ? dimB + 1 : dimB );
		    if ( dimB > 4 )
			bLeft.resize( 4 );
		    else
			bLeft.resize( 2 );
		} else {
		    bTop.resize( 2 );
		    bBot.resize( 2 );
		    if ( dimB == 3 )
			bLeft.resize( 4 );
		    else
			bLeft.resize( 2 );
		}

		bLeft.setPoint( 0, 0, 0 );
		bLeft.setPoint( 1, 0, dimB - 1);
		if ( dimB > 3 ) {
		    if ( dimB > 4 ) {
			bLeft.setPoint( 2, 1, 2 );
			bLeft.setPoint( 3, 1, dimB - 3 );
		    }
		    bTop.setPoint( 0, 1, 0 );
		    bTop.setPoint( 1, 1, 1 );
		    bTop.setPoint( 2, 2, 1 );
		    bTop.setPoint( 3, 3, 1 );

		    bBot.setPoint( 0, 1, dimB - 1 );
		    bBot.setPoint( 1, 1, dimB - 2 );
		    bBot.setPoint( 2, 2, dimB - 2 );
		    bBot.setPoint( 3, 3, dimB - 2 );

		    for( i = 0 ; i < dimB / 2 - 2 ; i++ ) {
			bTop.setPoint( i*2 + 4, 2 + i*2, 2 + i );
			bTop.setPoint( i*2 + 5, 5 + i*2, 2 + i );
			bBot.setPoint( i*2 + 4, 2 + i*2, dimB - 3 - i );
			bBot.setPoint( i*2 + 5, 5 + i*2, dimB - 3 - i );
		    }
		    if ( dimB & 1 ) {  // Extra line if size is an odd number
			bBot.setPoint( dimB - 1, dimB - 3, dimB / 2 );
			bBot.setPoint( dimB, dimB - 1, dimB / 2 );
		    }
		    if ( dimB > 6 ) { // Must fill interior if dimB > 6
			bFill.setPoint( 0, 1, dimB - 3 );
			bFill.setPoint( 1, 1, 2 );
			if ( dimB & 1 ) {  // If size is an odd number
			    bFill.setPoint( 2, dimB - 3, dimB / 2 );
			} else {
			    bFill.setPoint( 2, dimB - 4, dimB / 2 - 1 );
			    bFill.setPoint( 3, dimB - 4, dimB / 2 );
			}
		    }
		} else {
		    if ( dimB == 3 ) {	// Hardcoded pattern for 3x3 arrow
			bLeft.setPoint( 2, 1, 1 );
			bLeft.setPoint( 3, 1, 1 );
			bTop .setPoint( 0, 1, 0 );
			bTop .setPoint( 1, 1, 0 );
			bBot .setPoint( 0, 1, 2 );
			bBot .setPoint( 1, 2, 1 );
		    } else {  // dimB must be 2, hardcoded pattern for 2x2
			bTop .setPoint( 0, 1, 0 );
			bTop .setPoint( 1, 1, 0 );
			bBot .setPoint( 0, 1, 1 );
			bBot .setPoint( 1, 1, 1 );
		    }
		}
	    }

	    if ( controls & SUB_LINE ) {
		QWXFMatrix m;
		if ( VERTICAL ) {
		    m.rotate( -90 );
		    m.translate( 0, addB.height() - 1 );
		} else {
		    m.rotate( 180 );
		    m.translate( addB.width() - 1, addB.height() - 1 );
		}
		m.translate( subB.x(), subB.y() );
		p.setWxfMatrix( m );
		p.setWorldXForm( TRUE );

		QColor cleft, ctop, cbot, cmid;

		if ( SUB_LINE_ACTIVE ) {
		    cmid = downC;
		    if ( HORIZONTAL ) {
			cleft = lightC;
			ctop  = lightC;
			cbot  = shadowC;
		    } else {
			cleft = lightC;
			ctop  = shadowC;
			cbot  = lightC;
		    }
		} else {
		    cmid = upC;
		    if ( HORIZONTAL ) {
			cleft = shadowC;
			ctop  = shadowC;
			cbot  = lightC;
		    } else {
			cleft = shadowC;
			ctop  = lightC;
			cbot  = shadowC;
		    }
		}
		QPen pen( NoPen );
		QBrush brush( cmid );
		p.setPen( pen );
		p.setBrush( brush );
		p.drawPolygon( bFill );
		pen.setStyle( SolidLine );
		brush.setStyle( NoBrush );

		pen.setColor( cleft );
		p.drawLineSegments( bLeft );
		pen.setColor( ctop );
		p.drawLineSegments( bTop );
		pen.setColor( cbot );
		p.drawLineSegments( bBot );

		p.setWorldXForm( FALSE );
	    }

	    if ( controls & ADD_LINE ) {
		QWXFMatrix m;
		if ( VERTICAL ) {
		    m.rotate( 90 );
		    m.translate( addB.width()-1, 0 );
		}
		m.translate( addB.x(), addB.y() );
		p.setWxfMatrix( m );
		p.setWorldXForm( TRUE );

		QColor cleft, ctop, cbot, cmid;

		if ( ADD_LINE_ACTIVE ) {
		    cmid = downC;
		    if ( HORIZONTAL ) {
			cleft = shadowC;
			ctop  = shadowC;
			cbot  = lightC;
		    } else {
			cleft = shadowC;
			ctop  = lightC;
			cbot  = shadowC;
		    }
		} else {
		    cmid = upC;
		    if ( HORIZONTAL ) {
			cleft = lightC;
			ctop  = lightC;
			cbot  = shadowC;
		    } else {
			cleft = lightC;
			ctop  = shadowC;
			cbot  = lightC;
		    }
		}
		QPen pen( NoPen );
		QBrush brush( cmid );
		p.setPen( pen );
		p.setBrush( brush );
		p.drawPolygon( bFill );
		pen.setStyle( SolidLine );
		brush.setStyle( NoBrush );

		pen.setColor( cleft );
		p.drawLineSegments( bLeft );
		pen.setColor( ctop );
		p.drawLineSegments( bTop );
		pen.setColor( cbot );
		p.drawLineSegments( bBot );

		p.setWorldXForm( FALSE );
	    }
	    if ( controls & SUB_PAGE )
		p.fillRect( subPageR, backgroundColor() );
	    if ( controls & ADD_PAGE )
		p.fillRect( addPageR, backgroundColor() );
	    if ( controls & SLIDER ) {
		QColor tmp = p.backgroundColor();
		p.setBackgroundColor( foregroundColor() );
		p.drawShadePanel( sliderR, foregroundColor().light(),
				       foregroundColor().dark(), 2, 2, TRUE );
		p.setBackgroundColor( tmp );
	    }
	    break;
	}
    }
#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
}
