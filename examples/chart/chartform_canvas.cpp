#include "canvastext.h"
#include "chartform.h"

#include <qbrush.h>
#include <qcanvas.h>


void ChartForm::drawElements()
{
    QCanvasItemList list = m_canvas->allItems();
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	delete *it;

	// 360 * 16 for pies; Qt works with 16ths of degrees
    int scaleFactor = m_chartType == PIE ? 5760 :
			m_chartType == VERTICAL_BAR ? m_canvas->height() :
			    m_canvas->width();
    double biggest = 0.0;
    int count = 0;
    double total = 0.0;
    static double scales[MAX_ELEMENTS];

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    double value = m_elements[i].value();
	    count++;
	    total += value;
	    if ( value > biggest )
		biggest = value;
	    scales[i] = m_elements[i].value() * scaleFactor;
	}
    }

    if ( count ) {
	    // 2nd loop because of total and biggest
	for ( int i = 0; i < MAX_ELEMENTS; ++i )
	    if ( m_elements[i].isValid() )
		if ( m_chartType == PIE )
		    scales[i] = (m_elements[i].value() * scaleFactor) / total;
		else
		    scales[i] = (m_elements[i].value() * scaleFactor) / biggest;

	switch ( m_chartType ) {
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

    m_canvas->update();
}


void ChartForm::drawPieChart( const double scales[], double total, int )
{
    int width = m_canvas->width();
    int height = m_canvas->height();
    int size = width > height ? height : width;
    int x = width / 2;
    int y = height / 2;
    int angle = 0;

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasEllipse *arc = new QCanvasEllipse(
					    size, size, angle, extent, m_canvas );
	    arc->setX( x );
	    arc->setY( y );
	    arc->setZ( 0 );
	    arc->setBrush( QBrush( m_elements[i].valueColor(),
				   BrushStyle(m_elements[i].valuePattern()) ) );
	    arc->show();
	    angle += extent;
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		int labelX = m_elements[i].x( PIE );
		int labelY = m_elements[i].y( PIE );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    QRect rect = arc->boundingRect();
		    labelX = ( abs( rect.right() - rect.left() ) / 2 ) + rect.x();
		    labelY = ( abs( rect.top() - rect.bottom() ) / 2 ) + rect.y();
		}
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		text->setColor( m_elements[i].labelColor() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		m_elements[i].setX( PIE, labelX );
		m_elements[i].setY( PIE, labelY );
	    }
	}
    }
}


void ChartForm::drawVerticalBarChart(
	const double scales[], double total, int count )
{
    int width = m_canvas->width() / count;
    int x = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    int y = m_canvas->height() - extent;
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    x, y, width, extent, m_canvas );
	    rect->setBrush( QBrush( m_elements[i].valueColor(),
				    BrushStyle(m_elements[i].valuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		int labelX = m_elements[i].x( VERTICAL_BAR );
		int labelY = m_elements[i].y( VERTICAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = x;
		    labelY = y;
		}
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		text->setColor( m_elements[i].labelColor() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		m_elements[i].setX( VERTICAL_BAR, labelX );
		m_elements[i].setY( VERTICAL_BAR, labelY );
	    }
	    x += width;
	}
    }
}


void ChartForm::drawHorizontalBarChart(
	const double scales[], double total, int count )
{
    int height = m_canvas->height() / count;
    int y = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    0, y, extent, height, m_canvas );
	    rect->setBrush( QBrush( m_elements[i].valueColor(),
				    BrushStyle(m_elements[i].valuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		int labelX = m_elements[i].x( HORIZONTAL_BAR );
		int labelY = m_elements[i].y( HORIZONTAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = 0;
		    labelY = y;
		}
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		text->setColor( m_elements[i].labelColor() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		m_elements[i].setX( HORIZONTAL_BAR, labelX );
		m_elements[i].setY( HORIZONTAL_BAR, labelY );
	    }
	    y += height;
	}
    }
}


QString ChartForm::valueLabel(
	    const QString& label, double value, double total )
{
    if ( m_addValues == NO )
	return label;

    QString newLabel = label;
    if ( !label.isEmpty() )
	if ( m_chartType == VERTICAL_BAR )
	    newLabel += '\n';
	else
	    newLabel += ' ';
    if ( m_addValues == YES )
	newLabel += QString::number( value, 'f', m_decimalPlaces );
    else if ( m_addValues == AS_PERCENTAGE )
	newLabel += QString::number( (value / total) * 100, 'f', m_decimalPlaces )
		    + '%';
    return newLabel;
}

