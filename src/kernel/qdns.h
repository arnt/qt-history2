/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdns.h#3 $
**
** Definition of the DNS support class
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
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

#ifndef QDNS_H
#define QDNS_H

#include "qobject.h"


class Q_EXPORT QDns: public QObject
{
    Q_OBJECT
public:
    QDns( QObject  );

    void setName( const QString & name );
    QString name();

    // insert a method to get the IP address list associated with the
    // list.  This is generally 0 or 1, but can be more - www.vg.no
    // has two.

    void query( QObject * receiver, const char * slot );

    enum Status { Ready, Waiting, Error, Nonexistent };
    Status status() const;

signals:
    void statusChanged();

private:

};


#endif
