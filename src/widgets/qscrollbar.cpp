/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.cpp#22 $
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qscrollbar.cpp#22 $";
#endif


/*!
\class QScrollBar qscrbar.h
\brief The QScrollBar widget class provides a vertical or horizontal scroll
bar.

The documentation of the QScrollBar class will be completed later.
*/


enum ScrollControl { ADD_LINE = 0x1 , SUB_LINE = 0x2 , ADD_PAGE = 0x4,
		     SUB_PAGE = 0x8 , FIRST    = 0x10, LAST	= 0x20,
		     SLIDER   = 0x40, NONE     = 0x80 };


class QScrollBar_Private : public QScrollBar
{
public:
    void	  sliderMinMax( int *, int * ) const;
    void	  metrics( int *, int *, int * ) const;

    ScrollControl pointOver( const QPoint &p ) const;

    int		  rangeValueToSliderPos( long val ) const;
    long	  sliderPosToRangeValue( int  val ) const;

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


QScrollBar::QScrollBar( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    orient = Vertical;
    init();
}

QScrollBar::QScrollBar( Orientation o, QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    orient = o;
    init();
}

QScrollBar::QScrollBar( long minVal, long maxVal, long lineStep, long pageStep,
			long value,  Orientation o,
			QWidget *parent, const char *name )
	: QWidget( parent, name ),
	  QRangeControl( minVal, maxVal, lineStep, pageStep, value )
{
    orient = o;
    init();
}

void QScrollBar::init()
{
    track	     = TRUE;
    sliderPos	     = 0;
    pressedControl   = NONE;
    clickedAt	     = FALSE;
    setBackgroundColor( colorGroup().mid() );
}


void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    setBackgroundColor( colorGroup().mid() );
}


void QScrollBar::setOrientation( Orientation o )
{
    orient = o;
    update();
}


void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE , pressedControl );
    emit valueChanged(value());
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


void QScrollBar::keyPressEvent( QKeyEvent * )
{
}


void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColorGroup g = colorGroup();
    p.begin( this );
    p.drawShadePanel( rect(), g.dark(), g.light() );
    PRIV->drawControls( ADD_LINE | SUB_LINE | ADD_PAGE | SUB_PAGE | SLIDER,
			pressedControl, p );
    p.end();
}


void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    clickedAt	   = TRUE;
    pressedControl = PRIV->pointOver( e->pos() );
    switch( pressedControl ) {
	case SLIDER:
		 clickOffset = ( HORIZONTAL ? e->pos().x() : e->pos().y() )
			       - sliderPos;
                 slidePreviousVal = value();
                 emit sliderPressed();
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
	    if ( value() != previousValue() )
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
	    long newVal = PRIV->sliderPosToRangeValue(newSliderPos);
            if ( newVal != slidePreviousVal )
                emit sliderMoved( newVal );
	    if ( track && newVal != value() ) {
		directSetValue( newVal ); // Set directly, painting done below
		emit valueChanged( value() );
	    }
            slidePreviousVal = newVal;
	    sliderPos = newSliderPos;
	    PRIV->drawControls( ADD_PAGE | SLIDER | SUB_PAGE, pressedControl );
	}
    }
}

QRect QScrollBar::sliderRect() const
{
    int sliderMin, sliderMax, sliderLength;
    PRIV->metrics( &sliderMin, &sliderMax, &sliderLength );

    if ( HORIZONTAL )
        return QRect( sliderStart(), BORDER, 
                      sliderLength, height() - BORDER*2 );
    else
        return QRect( BORDER, sliderStart(),
                      width() - BORDER*2, sliderLength );
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = PRIV->rangeValueToSliderPos( value() );
}

long QScrollBar::calculateValueFromSlider() const
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

    int length = HORIZONTAL ? width()  : height();
    int extent = HORIZONTAL ? height() : width();

    if ( length > ( extent - BORDER*2 - 1 )*2 + BORDER*2 + SLIDER_MIN )
	buttonDim = extent - BORDER*2;
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


int QScrollBar_Private::rangeValueToSliderPos( long val ) const
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

long QScrollBar_Private::sliderPosToRangeValue( int pos ) const
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
            emit nextLine();
	    addLine();
	    break;
	case SUB_LINE:
            emit previousLine();
	    subtractLine();
	    break;
	case ADD_PAGE:
            emit nextPage();
	    addPage();
	    break;
	case SUB_PAGE:
            emit previousPage();
	    subtractPage();
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QScrollBar_Private::action internal error" );
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
    QColorGroup g  = colorGroup();

    int sliderMin, sliderMax, sliderLength;
    metrics( &sliderMin, &sliderMax, &sliderLength );

    int dimB = sliderMin - BORDER;
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
	subX = BORDER;
	addX = length - dimB - BORDER;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = BORDER;
	addY = length - dimB - BORDER;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart() + sliderLength;
    int sliderW = extent - BORDER*2;
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
	    if ( controls & ADD_LINE )
		qDrawMotifArrow( &p, VERTICAL ? MotifDownArrow:MotifRightArrow,
				 ADD_LINE_ACTIVE, addB.x(), addB.y(),
				 addB.width(), addB.height(),
				 g.background(), g.mid(), g.light(), g.dark());
	    if ( controls & SUB_LINE )
		qDrawMotifArrow( &p, VERTICAL ? MotifUpArrow : MotifLeftArrow,
				 SUB_LINE_ACTIVE, subB.x(), subB.y(),
				 subB.width(), subB.height(),
				 g.background(), g.mid(), g.light(), g.dark());
	    if ( controls & SUB_PAGE )
		p.fillRect( subPageR, g.mid() );
	    if ( controls & ADD_PAGE )
		p.fillRect( addPageR, g.mid() );
	    if ( controls & SLIDER )
		p.drawShadePanel( sliderR, g.light(), g.dark(), 2,
				  g.background(), TRUE );
	    break;
	}
    }
#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
}


void qDrawMotifArrow( QPainter *p, MotifArrow style, bool down,
		      int x, int y, int w, int h,
		      const QColor &upColor, const QColor &downColor,
		      const QColor &lightShadow, const QColor &darkShadow )
{
    QPointArray bFill;				// fill polygon
    QPointArray bTop;				// top shadow.
    QPointArray bBot;				// bottom shadow.
    QPointArray bLeft;				// left shadow.
    Q2DMatrix   matrix;				// xform matrix
    bool vertical = style == MotifUpArrow || style == MotifDownArrow;
    bool horizontal = !vertical;
    int  dim = w < h ? w : h;
    int  colspec = 0x0000;			// color specification array

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
	if ( dim & 1 )		// extra line if size is an odd number
	    bBot.putPoints( dim-1, 2, dim-3,dim/2, dim-1,dim/2 );
	if ( dim > 6 ) {	// must fill interior if dim > 6
	    bFill.putPoints( 0, 2, 1,dim-3, 1,2 );
	    if ( dim & 1 )	// if size is an odd number
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

    if ( style == MotifUpArrow || style == MotifLeftArrow ) {
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
    else if ( style == MotifDownArrow || style == MotifRightArrow ) {
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
    cols[1] = (QColor *)&upColor;
    cols[2] = (QColor *)&downColor;
    cols[3] = (QColor *)&lightShadow;
    cols[4] = (QColor *)&darkShadow;
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

    p->pen().setStyle( SolidLine );		// draw outline
    p->brush().setStyle( NoBrush );		// don't fill

    p->pen().setColor( CLEFT );
    p->drawLineSegments( bLeft );
    p->pen().setColor( CTOP );
    p->drawLineSegments( bTop );
    p->pen().setColor( CBOT );
    p->drawLineSegments( bBot );

    p->setWorldXForm( FALSE );			// turn off xform
    p->setBrush( saveBrush );			// restore brush
    p->setPen( savePen );			// restore pen
}
