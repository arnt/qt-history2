
#ifndef QT_H
#include "qvector.h"
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
    QVector<QRegion> regions;
    QWSRegionHeader *regHdr;
    QWSRegionIndex *regIdx;
    unsigned char *data;
    int shmId;
};

