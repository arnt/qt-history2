/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.h#7 $
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

    virtual QString find( const char *, const char * ) const;

    bool load( const QString & filename,
	       const QString & directory = QString::null,
	       const QString & search_delimiters = QString::null,
	       const QString & suffix = QString::null );

    enum SaveMode { Everything, Stripped };
    
    bool save( const QString & filename, SaveMode mode = Everything );

    void clear();

    void insert( const char *, const char *, const QString & );
    void remove( const char *, const char * );
    bool contains( const char *, const char * ) const;

    void squeeze();
    void unsqueeze();

private:
    QTranslatorPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTranslator( const QTranslator & );
    QTranslator &operator=( const QTranslator & );
#endif
};


#endif
