/****************************************************************************
**
** Definition of validator classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h" // char*->QString conversion
#include "qregexp.h" // QString->QRegExp conversion
#endif // QT_H

#ifndef QT_NO_VALIDATOR


class Q_GUI_EXPORT QValidator : public QObject
{
    Q_OBJECT
public:
    QValidator( QObject * parent, const char *name = 0 );
    ~QValidator();

    enum State { Invalid, Intermediate, Valid=Intermediate, Acceptable };

    virtual State validate( QString &, int & ) const = 0;
    virtual void fixup( QString & ) const;

private:
#if defined(Q_DISABLE_COPY)
    QValidator( const QValidator & );
    QValidator& operator=( const QValidator & );
#endif
};


class Q_GUI_EXPORT QIntValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY( int bottom READ bottom WRITE setBottom )
    Q_PROPERTY( int top READ top WRITE setTop )

public:
    QIntValidator( QObject * parent, const char *name = 0 );
    QIntValidator( int bottom, int top,
		   QObject * parent, const char *name = 0 );
    ~QIntValidator();

    QValidator::State validate( QString &, int & ) const;

    void setBottom( int );
    void setTop( int );
    virtual void setRange( int bottom, int top );

    int bottom() const { return b; }
    int top() const { return t; }

private:
#if defined(Q_DISABLE_COPY)
    QIntValidator( const QIntValidator & );
    QIntValidator& operator=( const QIntValidator & );
#endif

    int b, t;
};

#ifndef QT_NO_REGEXP

class Q_GUI_EXPORT QDoubleValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY( double bottom READ bottom WRITE setBottom )
    Q_PROPERTY( double top READ top WRITE setTop )
    Q_PROPERTY( int decimals READ decimals WRITE setDecimals )

public:
    QDoubleValidator( QObject * parent, const char *name = 0 );
    QDoubleValidator( double bottom, double top, int decimals,
		      QObject * parent, const char *name = 0 );
    ~QDoubleValidator();

    QValidator::State validate( QString &, int & ) const;

    virtual void setRange( double bottom, double top, int decimals = 0 );
    void setBottom( double );
    void setTop( double );
    void setDecimals( int );

    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }

private:
#if defined(Q_DISABLE_COPY)
    QDoubleValidator( const QDoubleValidator & );
    QDoubleValidator& operator=( const QDoubleValidator & );
#endif

    double b, t;
    int d;
};


class Q_GUI_EXPORT QRegExpValidator : public QValidator
{
    Q_OBJECT
    // Q_PROPERTY( QRegExp regExp READ regExp WRITE setRegExp )

public:
    QRegExpValidator( QObject *parent, const char *name = 0 );
    QRegExpValidator( const QRegExp& rx, QObject *parent,
		      const char *name = 0 );
    ~QRegExpValidator();

    virtual QValidator::State validate( QString& input, int& pos ) const;

    void setRegExp( const QRegExp& rx );
    const QRegExp& regExp() const { return r; }

private:
#if defined(Q_DISABLE_COPY)
    QRegExpValidator( const QRegExpValidator& );
    QRegExpValidator& operator=( const QRegExpValidator& );
#endif

    QRegExp r;
};
#endif // QT_NO_REGEXP


#endif // QT_NO_VALIDATOR

#endif // QVALIDATOR_H
