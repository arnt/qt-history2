/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.h#2 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#include "qobject.h"
#include "qintdict.h"


class QTranslatorPrivate;


class Q_EXPORT QTranslator: public QObject
{
    Q_OBJECT
public:
    QTranslator( QObject * parent, const char * name = 0 );
    ~QTranslator();

    virtual QString find( uint, const char *, const char * ) const;

    void load( const QString & filename, const QString & directory = 0 );
    void save( const QString & filename );
    void clear();

    void squeeze();
    void unsqueeze();

    void insert( uint, const QString & );
    void remove( uint );
    bool contains( uint, const char *, const char * ) const;

    static uint hash( const char *, const char * );

private:
    QTranslatorPrivate * d;

    friend class QTranslatorIterator;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTranslator( const QTranslator & );
    QTranslator &operator=( const QTranslator & );
#endif
};


class Q_EXPORT QTranslatorIterator
{
public:
    QTranslatorIterator( QTranslator & );
    ~QTranslatorIterator();
    uint count() const;
    bool isEmpty() const;
    QString * toFirst();
    operator QString *() const;
    QString * current() const;
    uint currentKey() const;
    QString * operator++();
    QString * operator+=( uint jump );

private:
    QIntDictIterator<QString> * it;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTranslatorIterator( const QTranslatorIterator & );
    QTranslatorIterator &operator=( const QTranslatorIterator & );
#endif
};


#endif
