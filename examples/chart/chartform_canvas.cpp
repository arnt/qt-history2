#include "canvastext.h"
#include "chartform.h"

#include <qbrush.h>
#include <qcanvas.h>


void ChartForm::drawElements()
{
    QCanvasItemList list = canvas->allItems();
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	delete *it;

	// 360 * 16 for pies; Qt works with 16ths of degrees
    int scaleFactor = chartType == PIE ? 5760 :
			chartType == VERTICAL_BAR ? canvas->height() :
			    canvas->width();
    double biggest = 0.0;
    int count = 0;
    double total = 0.0;
    static double scales[MAX_ELEMENTS];

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    double value = elements[i].getValue();
	    count++;
	    total += value;
	    if ( value > biggest )
		biggest = value;
	    scales[i] = elements[i].getValue() * scaleFactor;
	}
    }

    if ( count ) {
	    // 2nd loop because of total and biggest
	for ( int i = 0; i < MAX_ELEMENTS; ++i )
	    if ( elements[i].isValid() )
		if ( chartType == PIE )
		    scales[i] = (elements[i].getValue() * scaleFactor) / total;
		else
		    scales[i] = (elements[i].getValue() * scaleFactor) / biggest;

	switch ( chartType ) {
	    case PIE:
		drawPieChart( scales, total, count );
		break;
	    case VERTICAL_BAR:
		drawVerticalBarChart( scales, total, count );
		break;
	    case HORIZONTAL_BAR:
		drawHorizontalBarChart( scales, total, count );
		break;
	}
    }

    canvas->update();
}


void ChartForm::drawPieChart( const double scales[], double total, int )
{
    int width = canvas->width();
    int height = canvas->height();
    int size = width > height ? height : width;
    int x = width / 2;
    int y = height / 2;
    int angle = 0;

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasEllipse *arc = new QCanvasEllipse(
					    size, size, angle, extent, canvas );
	    arc->setX( x );
	    arc->setY( y );
	    arc->setZ( 0 );
	    arc->setBrush( QBrush( elements[i].getValueColour(),
				   BrushStyle(elements[i].getValuePattern()) ) );
	    arc->show();
	    angle += extent;
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( PIE );
		int labelY = elements[i].getY( PIE );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    QRect rect = arc->boundingRect();
		    labelX = ( abs( rect.right() - rect.left() ) / 2 ) + rect.x();
		    labelY = ( abs( rect.top() - rect.bottom() ) / 2 ) + rect.y();
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( PIE, labelX );
		elements[i].setY( PIE, labelY );
	    }
	}
    }
}


void ChartForm::drawVerticalBarChart(
	const double scales[], double total, int count )
{
    int width = canvas->width() / count;
    int x = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    int y = canvas->height() - extent;
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    x, y, width, extent, canvas );
	    rect->setBrush( QBrush( elements[i].getValueColour(),
				    BrushStyle(elements[i].getValuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( VERTICAL_BAR );
		int labelY = elements[i].getY( VERTICAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = x;
		    labelY = y;
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( VERTICAL_BAR, labelX );
		elements[i].setY( VERTICAL_BAR, labelY );
	    }
	    x += width;
	}
    }
}


void ChartForm::drawHorizontalBarChart(
	const double scales[], double total, int count )
{
    int height = canvas->height() / count;
    int y = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    0, y, extent, height, canvas );
	    rect->setBrush( QBrush( elements[i].getValueColour(),
				    BrushStyle(elements[i].getValuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( HORIZONTAL_BAR );
		int labelY = elements[i].getY( HORIZONTAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = 0;
		    labelY = y;
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( HORIZONTAL_BAR, labelX );
		elements[i].setY( HORIZONTAL_BAR, labelY );
	    }
	    y += height;
	}
    }
}


QString ChartForm::valueLabel(
	    const QString& label, double value, double total )
{
    if ( addValues == NO )
	return label;

    QString newLabel = label;
    if ( !label.isEmpty() )
	if ( chartType == VERTICAL_BAR )
	    newLabel += '\n';
	else
	    newLabel += ' ';
    if ( addValues == YES )
	newLabel += QString::number( value, 'f', decimalPlaces );
    else if ( addValues == AS_PERCENTAGE )
	newLabel += QString::number( (value / total) * 100, 'f', decimalPlaces )
		    + '%';
    return newLabel;
}

