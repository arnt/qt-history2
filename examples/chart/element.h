#ifndef ELEMENT_H
#define ELEMENT_H

#include <qcolor.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qvaluevector.h>

class Element;

typedef QValueVector<Element> ElementVector;

/*
    Elements are valid if they have a value which is > EPSILON.
*/
const double EPSILON = 0.0000001; // Must be > INVALID.


class Element
{
public:
    enum { INVALID = -1 };
    enum { NO_POSITION = -1 };
    enum { MAX_POINTS = 3 }; // One point per chart type

    Element( double value = INVALID, QColor valueColor = Qt::gray,
	     int valuePattern = Qt::SolidPattern,
	     const QString& label = QString::null,
	     QColor labelColor = Qt::black ) {
	init( value, valueColor, valuePattern, label, labelColor );
	for ( int i = 0; i < MAX_POINTS * 2; ++i )
	    m_points[i] = NO_POSITION;
    }
    ~Element() {}

    bool isValid() const { return m_value > EPSILON; }

    double value() const { return m_value; }
    QColor valueColor() const { return m_valueColor; }
    int valuePattern() const { return m_valuePattern; }
    QString label() const { return m_label; }
    QColor labelColor() const { return m_labelColor; }
    int x( int index ) const;
    int y( int index ) const;

    void set( double value = INVALID, QColor valueColor = Qt::gray,
	      int valuePattern = Qt::SolidPattern,
	      const QString& label = QString::null,
	      QColor labelColor = Qt::black ) {
	init( value, valueColor, valuePattern, label, labelColor );
    }
    void setValue( double value ) { m_value = value; }
    void setValueColor( QColor valueColor ) { m_valueColor = valueColor; }
    void setValuePattern( int valuePattern );
    void setLabel( const QString& label ) { m_label = label; }
    void setLabelColor( QColor labelColor ) { m_labelColor = labelColor; }
    void setX( int index, int value );
    void setY( int index, int value );

private:
    void init( double value, QColor valueColor, int valuePattern,
	       const QString& label, QColor labelColor );

    double m_value;
    QColor m_valueColor;
    int m_valuePattern;
    QString m_label;
    QColor m_labelColor;
    int m_points[2 * MAX_POINTS];
};


Q_EXPORT QTextStream &operator<<( QTextStream&, const Element& );
Q_EXPORT QTextStream &operator>>( QTextStream&, Element& );

#endif
