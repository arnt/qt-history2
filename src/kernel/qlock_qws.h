/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlock_qws.h $
**
** Definition of QLock class. This manages interprocess locking
**
** Created : 20000406
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QLOCK_QWS_H
#define QLOCK_QWS_H

#ifndef QT_H
#include <qstring.h>
#endif // QT_H

class QLockData;

class QLock
{
public:
    QLock( const QString &filename, char id, bool create = FALSE );
    ~QLock();

    enum Type { Read, Write };

    bool isValid() const;
    void lock( Type type );
    void unlock();
    bool locked() const;

private:
    Type type;
    QLockData *data;
};


// Nice class for ensuring the lock is released.
// Just create one on the stack and the lock is automatically released
// when QLockHolder is destructed.
class QLockHolder
{
public:
    QLockHolder( QLock *l, QLock::Type type ) : qlock(l) {
	qlock->lock( type );
    }
    ~QLockHolder() { if ( locked() ) qlock->unlock(); }

    void lock( QLock::Type type ) { qlock->lock( type ); }
    void unlock() { qlock->unlock(); }
    bool locked() const { return qlock->locked(); }

private:
    QLock *qlock;
};

#endif

