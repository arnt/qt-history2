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

    Element( double value = INVALID, QColor valueColour = Qt::gray,
	     int valuePattern = Qt::SolidPattern,
	     const QString& label = QString::null,
	     QColor labelColour = Qt::black ) {
	init( value, valueColour, valuePattern, label, labelColour );
	for ( int i = 0; i < MAX_POINTS * 2; ++i )
	    m_points[i] = NO_POSITION;
    }
    ~Element() {}

    bool isValid() const { return m_value > EPSILON; }

    double getValue() const { return m_value; }
    QColor getValueColour() const { return m_valueColour; }
    int getValuePattern() const { return m_valuePattern; }
    QString getLabel() const { return m_label; }
    QColor getLabelColour() const { return m_labelColour; }
    int getX( int index ) const;
    int getY( int index ) const;

    void set( double value = INVALID, QColor valueColour = Qt::gray,
	      int valuePattern = Qt::SolidPattern,
	      const QString& label = QString::null,
	      QColor labelColour = Qt::black ) {
	init( value, valueColour, valuePattern, label, labelColour );
    }
    void setValue( double value ) { m_value = value; }
    void setValueColour( QColor valueColour ) { m_valueColour = valueColour; }
    void setValuePattern( int valuePattern );
    void setLabel( const QString& label ) { m_label = label; }
    void setLabelColour( QColor labelColour ) { m_labelColour = labelColour; }
    void setX( int index, int value );
    void setY( int index, int value );

private:
    void init( double value, QColor valueColour, int valuePattern,
	       const QString& label, QColor labelColour );

    double m_value;
    QColor m_valueColour;
    int m_valuePattern;
    QString m_label;
    QColor m_labelColour;
    int m_points[2 * MAX_POINTS];
};


Q_EXPORT QTextStream &operator<<( QTextStream&, const Element& );
Q_EXPORT QTextStream &operator>>( QTextStream&, Element& );

#endif
