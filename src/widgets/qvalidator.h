/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvalidator.h#18 $
**
** Definition of validator classes.
**
** Created : 970610
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QValidator: public QObject
{
    Q_OBJECT
public:
    QValidator( QWidget * parent, const char *name = 0 );
    ~QValidator();

    enum State { Invalid, Intermediate, Valid=Intermediate, Acceptable };

    virtual State validate( QString &, int & ) const = 0;
    virtual void fixup( QString & ) const;
};


class Q_EXPORT QIntValidator: public QValidator
{
    Q_OBJECT
    Q_PROPERTY( int, "bottom", bottom, setBottom )
    Q_PROPERTY( int, "top", top, setTop )

public:
    QIntValidator( QWidget * parent, const char *name = 0 );
    QIntValidator( int bottom, int top,
		   QWidget * parent, const char *name = 0 );
    ~QIntValidator();

    QValidator::State validate( QString &, int & ) const;

    void setBottom( int );
    void setTop( int );
    virtual void setRange( int bottom, int top );

    int bottom() const { return b; }
    int top() const { return t; }

private:
    int b, t;
};


class Q_EXPORT QDoubleValidator: public QValidator
{
    Q_OBJECT
    Q_PROPERTY( double, "bottom", bottom, setBottom )
    Q_PROPERTY( double, "top", top, setTop )
    Q_PROPERTY( int, "decimals", decimals, setDecimals )
	
public:
    QDoubleValidator( QWidget * parent, const char *name = 0 );
    QDoubleValidator( double bottom, double top, int decimals,
		      QWidget * parent, const char *name = 0 );
    ~QDoubleValidator();

    QValidator::State validate( QString &, int & ) const;

    virtual void setRange( double bottom, double top, int decimals = 0 );
    void setBottom( int );
    void setTop( int );
    void setDecimals( int );
    
    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }

private:
    double b, t;
    int d;
};


#endif // QVALIDATOR_H
