/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmessagefile.h#6 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QMESSAGEFILE_H
#define QMESSAGEFILE_H

#include "qobject.h"
#include "qintdict.h"


class QMessageFilePrivate;


class QMessageFile: public QObject
{
    Q_OBJECT
public:
    QMessageFile( QObject * parent, const char * name = 0 );
    ~QMessageFile();

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
    QMessageFilePrivate * d;

    friend class QMessageFileIterator;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMessageFile( const QMessageFile & );
    QMessageFile &operator=( const QMessageFile & );
#endif
};


class QMessageFileIterator
{
public:
    QMessageFileIterator( QMessageFile & );
    ~QMessageFileIterator();
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
    QMessageFileIterator( const QMessageFileIterator & );
    QMessageFileIterator &operator=( const QMessageFileIterator & );
#endif
};


#endif
