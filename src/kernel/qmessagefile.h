/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmessagefile.h#1 $
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


class QMessageFile: public QObject
{
    Q_OBJECT
public:
    QMessageFile( QObject * parent, const char * name = 0 );
    ~QMessageFile();

    void open( const QString & filename, const QString & directory = 0 );

    const QString find( const QString & key );
    static uint hash( const QString & key );

    // handy statics
    static const QString translate( const QString &, const QString & );

private:
    const char * t;
    int l;
};


#endif
