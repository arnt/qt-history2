/****************************************************************************
**
** Definition of __________.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSREGIONMANAGER_QWS_H
#define QWSREGIONMANAGER_QWS_H

#ifndef QT_H
#include "qlist.h"
#include "qregion.h"
#endif // QT_H

class QWSRegionHeader;
class QWSRegionIndex;

class QWSRegionManager
{
public:
    QWSRegionManager( const QString &filename, bool c = TRUE );
    ~QWSRegionManager();

    // for clients
    const int *revision( int idx ) const;
    QRegion region( int idx );

    int find( int id );

    // for server
    int add( int id, QRegion region );
    void set( int idx, QRegion region );
    void remove( int idx );
    void markUpdated( int idx );
    void commit();

private:
    QRect *rects( int offset );
    bool attach( const QString &filename );
    void detach();

private:
    bool client;
    QList<QRegion*> regions;
    QWSRegionHeader *regHdr;
    QWSRegionIndex *regIdx;
    unsigned char *data;
    int shmId;
};

#endif // QWSREGIONMANAGER_QWS_H
