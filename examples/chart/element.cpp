#include "element.h"

#include <qstringlist.h>
#include <qtextstream.h>

const char FIELD_SEP = ':';
const char PROPOINT_SEP = ';';
const char XY_SEP = ',';


void Element::init( double value, QColor valueColor, int valuePattern,
		    const QString& label, QColor labelColor )
{
    m_value = value;
    m_valueColor = valueColor;
    if ( valuePattern < Qt::SolidPattern || valuePattern > Qt::DiagCrossPattern )
	valuePattern = Qt::SolidPattern;
    m_valuePattern = valuePattern;
    m_label = label;
    m_labelColor = labelColor;
}


void Element::setValuePattern( int valuePattern )
{
    if ( valuePattern < Qt::SolidPattern || valuePattern > Qt::DiagCrossPattern )
	valuePattern = Qt::SolidPattern;
    m_valuePattern = valuePattern;
}


double Element::proX( int index ) const
{
    Q_ASSERT(index >= 0 && index < MAX_PROPOINTS);
    return m_propoints[2 * index];
}


double Element::proY( int index ) const
{
    Q_ASSERT(index >= 0 && index < MAX_PROPOINTS);
    return m_propoints[(2 * index) + 1];
}


void Element::setProX( int index, double value )
{
    Q_ASSERT(index >= 0 && index < MAX_PROPOINTS);
    m_propoints[2 * index] = value;
}


void Element::setProY( int index, double value )
{
    Q_ASSERT(index >= 0 && index < MAX_PROPOINTS);
    m_propoints[(2 * index) + 1] = value;
}


QTextStream &operator<<( QTextStream &s, const Element &element )
{
    s << element.value() << FIELD_SEP
      << element.valueColor().name() << FIELD_SEP
      << element.valuePattern() << FIELD_SEP
      << element.labelColor().name() << FIELD_SEP;

    for ( int i = 0; i < Element::MAX_PROPOINTS; ++i ) {
	s << element.proX( i ) << XY_SEP << element.proY( i );
	s << ( i == Element::MAX_PROPOINTS - 1 ? FIELD_SEP : PROPOINT_SEP );
    }

    s << element.label() << '\n';

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
    QColor valueColor = QColor( data.section( FIELD_SEP, 1, 1 ) );
    if ( !valueColor.isValid() )
	errors++;
    int valuePattern = data.section( FIELD_SEP, 2, 2 ).toInt( &ok );
    if ( !ok )
	errors++;
    QColor labelColor = QColor( data.section( FIELD_SEP, 3, 3 ) );
    if ( !labelColor.isValid() )
	errors++;
    QStringList propoints = QStringList::split(
			    PROPOINT_SEP, data.section( FIELD_SEP, 4, 4 ) );
    QString label = data.section( FIELD_SEP, 5 );

    if ( !errors ) {
	element.set( value, valueColor, valuePattern, label, labelColor );
	int i = 0;
	for ( QStringList::iterator it = propoints.begin();
	    i < Element::MAX_PROPOINTS && it != propoints.end(); ++i, ++it ) {
	    element.setProX( i, (*it).section( XY_SEP, 0, 0 ).toDouble() );
	    element.setProY( i, (*it).section( XY_SEP, 1, 1).toDouble() );
	}
    }

    return s;
}
