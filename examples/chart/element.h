#ifndef ELEMENT_H
#define ELEMENT_H

#include <qcolor.h>
#include <qstring.h>
#include <qvaluevector.h>

class Element;

typedef QValueVector<Element> ElementVector;

/*
 * Elements are valid if they have a value which is > EPSILON.
 * Negative values are not allowed.
*/

class Element
{
public:
    Element( double value = -EPSILON,	QColor valueColour = Qt::gray,
	     const QString& label = QString::null, QColor labelColour = Qt::black,
	     int x = 0, int y = 0 ) {
	_value = value;
	_valueColour = valueColour;
	_label = label;
	_labelColour = labelColour;
	_x = x;
	_y = y;
    }
    ~Element() {}

    bool isValid() { return _value > EPSILON; }

    double getValue() { return _value; }
    QColor getValueColour() { return _valueColour; }
    QString getLabel() { return _label; }
    QColor getLabelColour() { return _labelColour; }
    int getX() { return _x; }
    int getY() { return _y; }

    void set( double value = -EPSILON,	QColor valueColour = Qt::gray,
	      const QString& label = QString::null, QColor labelColour = Qt::black,
	      int x = 0, int y = 0 ) {
	_value = value;
	_valueColour = valueColour;
	_label = label;
	_labelColour = labelColour;
	_x = x;
	_y = y;
    }
    void setValue( double value ) { _value = value; }
    void setValueColour( QColor valueColour ) { _valueColour = valueColour; }
    void setLabel( const QString& label ) { _label = label; }
    void setLabelColour( QColor labelColour ) { _labelColour = labelColour; }
    void setX( int x ) { _x = x; }
    void setY( int y ) { _y = y; }

private:
    double _value;
    QColor _valueColour;
    QString _label;
    QColor _labelColour;
    int _x; // Label x-position
    int _y; // Label y-position
};

#endif
