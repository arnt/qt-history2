/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.h#7 $
**
** Definition of 
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include "qobject.h"
#include "qstring.h"


class QValidator: public QObject
{
    Q_OBJECT
public:
    QValidator( QWidget * parent, const char * name = 0 );
    ~QValidator();

    enum State { Invalid, Valid, Acceptable };

    virtual State isValid( const char * ) = 0;
    virtual void fixup( QString & );
};


class QIntValidator: public QValidator
{
    Q_OBJECT
public:
    QIntValidator( QWidget * parent, const char * name = 0 );
    QIntValidator( int bottom, int top,
		   QWidget * parent, const char * name = 0 );
    ~QIntValidator();

    QValidator::State isValid( const char * );

    virtual void setRange( int bottom, int top );

    int bottom() const { return b; }
    int top() const { return t; }
    
private:
    int b, t;
};


class QDoubleValidator: public QValidator
{
    Q_OBJECT
public:
    QDoubleValidator( QWidget * parent, const char * name = 0 );
    QDoubleValidator( double bottom, double top, int decimals,
		      QWidget * parent, const char * name = 0 );
    ~QDoubleValidator();

    QValidator::State isValid( const char * );

    virtual void setRange( double bottom, double top, int decimals = 0 );

    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }
    
private:
    double b, t;
    int d;
};

#endif
