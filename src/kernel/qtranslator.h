/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.h#6 $
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

    bool load( const QString & filename,
	       const QString & directory = QString::null,
	       const QString & search_delimiters = QString::null,
	       const QString & suffix = QString::null );
    bool save( const QString & filename );
    void clear();

    void insert( const char *, const char *, const QString & );
    void remove( const char *, const char * );
    bool contains( const char *, const char * ) const;

    static uint hash( const char *, const char * );

private:
    QTranslatorPrivate * d;
    void squeeze();
    void unsqueeze();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTranslator( const QTranslator & );
    QTranslator &operator=( const QTranslator & );
#endif
};

#endif
