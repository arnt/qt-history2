/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.h#4 $
**
** Definition of 
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include "qobject.h"
#include "qstring.h"


class QValidator
{
public:
    QValidator();
    virtual ~QValidator();

    virtual bool isValid( const char * ) = 0;
    virtual void fixup( QString & );
};


class QIntValidator: public QValidator
{
public:
    QIntValidator();
    QIntValidator( int bottom, int top );
    ~QIntValidator();

    bool isValid( const char * );

    virtual void setRange( int bottom, int top );

    int bottom() const { return b; }
    int top() const { return t; }
    
private:
    int b, t;
};


class QDoubleValidator: public QValidator
{
public:
    QDoubleValidator();
    QDoubleValidator( double bottom, double top, int decimals = 0 );
    ~QDoubleValidator();

    bool isValid( const char * );

    virtual void setRange( double bottom, double top, int decimals = 0 );

    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }
    
private:
    double b, t;
    int d;
};

#endif
