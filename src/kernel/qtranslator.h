/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtranslator.h#9 $
**
** Definition of the translator class
**
** Created : 980906
**
** Copyright (C) 1998-99 by Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/


#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qintdict.h"
#endif // QT_H


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
