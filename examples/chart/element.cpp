#include "element.h"

#include <qstringlist.h>
#include <qtextstream.h>

const char FIELD_SEP = ':';
const char POINT_SEP = ';';
const char XY_SEP = ',';


void Element::init( double value, QColor valueColour, int valuePattern,
		    const QString& label, QColor labelColour )
{
    m_value = value;
    m_valueColour = valueColour;
    if ( valuePattern < Qt::SolidPattern || valuePattern > Qt::DiagCrossPattern )
	valuePattern = Qt::SolidPattern;
    m_valuePattern = valuePattern;
    m_label = label;
    m_labelColour = labelColour;
}


void Element::setValuePattern( int valuePattern )
{
    if ( valuePattern < Qt::SolidPattern || valuePattern > Qt::DiagCrossPattern )
	valuePattern = Qt::SolidPattern;
    m_valuePattern = valuePattern;
}


int Element::getX( int index ) const
{
    Q_ASSERT(index >= 0 && index < MAX_POINTS);
    return m_points[2 * index];
}


int Element::getY( int index ) const
{
    Q_ASSERT(index >= 0 && index < MAX_POINTS);
    return m_points[(2 * index) + 1];
}


void Element::setX( int index, int value )
{
    Q_ASSERT(index >= 0 && index < MAX_POINTS);
    m_points[2 * index] = value;
}


void Element::setY( int index, int value )
{
    Q_ASSERT(index >= 0 && index < MAX_POINTS);
    m_points[(2 * index) + 1] = value;
}


QTextStream &operator<<( QTextStream &s, const Element &element )
{
    s << element.getValue() << FIELD_SEP
      << element.getValueColour().name() << FIELD_SEP
      << element.getValuePattern() << FIELD_SEP
      << element.getLabelColour().name() << FIELD_SEP;

    for ( int i = 0; i < Element::MAX_POINTS; ++i ) {
	s << element.getX( i ) << XY_SEP << element.getY( i );
	s << ( i == Element::MAX_POINTS - 1 ? FIELD_SEP : POINT_SEP );
    }

    s << element.getLabel() << '\n';

    return s;
}


QTextStream &operator>>( QTextStream &s, Element &element )
{
    QString data = s.readLine();

    int errors = 0;
    bool ok;

    double value = data.section( FIELD_SEP, 0, 0 ).toDouble( &ok );
    if ( !ok )
	errors++;
    QColor valueColour = QColor( data.section( FIELD_SEP, 1, 1 ) );
    if ( !valueColour.isValid() )
	errors++;
    int valuePattern = data.section( FIELD_SEP, 2, 2 ).toInt( &ok );
    if ( !ok )
	errors++;
    QColor labelColour = QColor( data.section( FIELD_SEP, 3, 3 ) );
    if ( !labelColour.isValid() )
	errors++;
    QStringList points = QStringList::split(
			    POINT_SEP, data.section( FIELD_SEP, 4, 4 ) );
    QString label = data.section( FIELD_SEP, 5 );

    if ( !errors ) {
	element.set( value, valueColour, valuePattern, label, labelColour );
	int i = 0;
	for ( QStringList::iterator it = points.begin();
	    i < Element::MAX_POINTS && it != points.end(); ++i, ++it ) {
	    element.setX( i, (*it).section( XY_SEP, 0, 0 ).toInt() );
	    element.setY( i, (*it).section( XY_SEP, 1, 1 ).toInt() );
	}
    }

    return s;
}
