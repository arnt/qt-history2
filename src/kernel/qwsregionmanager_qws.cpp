
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "qwsdisplay_qws.h"
#include "qwsregionmanager_qws.h"

#define QT_MAX_REGIONS      400
#define QT_RECTS_PER_REGION 4

/*

Format:

| QWSRegionHeader | QWSRegionIndex * N | Rectanges * M |

*/

class QWSRegionHeader
{
public:
    int maxRegions;
    int maxRects;
};

class QWSRegionIndex
{
public:
    int id;
    int revision;
    int numRects;
    int data;
};

/*
 Create a QWSRegionManger.  if c == TRUE the QWSRegionManager is for
 clients, else it is for use by a server.
*/
QWSRegionManager::QWSRegionManager( const QString &filename, bool c) :
    client( c )
{
    if ( client ) {
	if ( !attach( filename ) )
	    qFatal( "Cannot attach region manager" );
	regHdr = (QWSRegionHeader *)(data);
	regIdx = (QWSRegionIndex *)(data + sizeof(QWSRegionHeader));
    } else {
	// the server needs a local copy of the data
	regions.setAutoDelete(TRUE);
	regHdr = new QWSRegionHeader;
	regHdr->maxRegions = 0;
	regHdr->maxRects = QT_MAX_REGIONS * QT_RECTS_PER_REGION;
	regIdx = new QWSRegionIndex[ QT_MAX_REGIONS ];
	if ( !attach( filename ) )
	    qFatal( "Cannot attach region manager" );
	commit();
    }
}

/*
 Detach from shared memory and cleanup.
*/
QWSRegionManager::~QWSRegionManager()
{
    detach();

    if ( !client ) {
	delete regHdr;
	delete [] regIdx;
    }
}

/*
 Get the revision of a region.  Since it is expensive to get a region with
 region() you should normally store a copy of your region plus its revision.
 Before using the region, check whether a different revision is currently
 in shared memory, and if so retrieve it using region().
 It is safe to store the pointer returned for later use.
*/
int *QWSRegionManager::revision( int idx )
{
    return &regIdx[idx].revision;
}

/*
 Get the region at an index.  Use find(int) to get the index from an id.
*/
QRegion QWSRegionManager::region( int idx )
{
    QRegion r;
    if ( regIdx[idx].numRects )
	r.setRects( rects(regIdx[idx].data), regIdx[idx].numRects );
    return r;
}

/*
 Search through the region index for a particular region id.
 The index is not sorted, but it does not change the placement of
 indexes either, so it is safe to store the index for later use.
*/
int QWSRegionManager::find( int id )
{
    for ( int idx = 0; idx < regHdr->maxRegions; idx++ ) {
	if ( regIdx[idx].id == id ) {
	    return idx;
	}
    }

    return -1;
}

/*
 Add a new region.  Only the local copy is modified.
 Call commit() to commit the current region table to shared memory.
*/
int QWSRegionManager::add( int id, QRegion region )
{
    if ( client ) return -1;

    int idx = 0;

    for ( idx = 0; idx < regHdr->maxRegions; idx++ ) {
	if ( regIdx[idx].id == -1 )
	    break;
    }

    if ( idx == regHdr->maxRegions ) {
	regHdr->maxRegions++;
	if ( regHdr->maxRegions > QT_MAX_REGIONS ) {
	    // What do we do?  Resize the shared memory?
	    qFatal( "Too many client regions." );
	}
	regions.resize( regHdr->maxRegions );
    }

    regIdx[idx].id = id;
    regIdx[idx].revision = 0;
    if ( !regions[idx] )
	regions.insert( idx, new QRegion );
    set( idx, region );

    return idx;
}

/*
 Set an existing region.  Only the local copy is modified.
 Call commit() to commit the current region table to shared memory.
*/
void QWSRegionManager::set( int idx, QRegion region )
{
    if ( client ) return;

    regIdx[idx].revision++;
    regIdx[idx].numRects = region.rects().size();
    *regions[idx] = region;
}

/*
 Remove an existing region.  Only the local copy is modified.
 Call commit() to commit the current region table to shared memory.
*/
void QWSRegionManager::remove( int idx )
{
    if ( client ) return;

    regIdx[idx].id = -1;
}

/*
 Commit the region table to shared memory.
 The region table is not copied to shared memory every time it is
 modified as this would lead to a great deal of overhead when the server
 is calculating new regions.  Once all regions have been modified, commit()
 should be called to set the new region table in shared memory.
*/
void QWSRegionManager::commit()
{
    if ( client ) return;

    QWSDisplay::grab( TRUE );

    // copy region rects first
    int numRects;
    int offset = 0;
    QRect *r = rects(0);
    for ( int idx = 0; idx < regHdr->maxRegions; idx++ ) {
	if ( regIdx[idx].id != -1 ) {
	    numRects = regIdx[idx].numRects;
	    if ( numRects ) {
		if ( offset + numRects > regHdr->maxRects ) {
		    // What do we do?  Resize the shared memory?
		    qFatal( "Too many client rects" );
		}
		regIdx[idx].data = offset;
		memcpy( r, regions[idx]->rects().data(),
			numRects * sizeof(QRect) );
		r += numRects;
		offset += numRects;
	    }
	}
    }

    // now copy region header and index
    memcpy( data, regHdr, sizeof(QWSRegionHeader) );
    memcpy( data + sizeof(QWSRegionHeader),
	    (unsigned char *)regIdx,
	    regHdr->maxRegions * sizeof(QWSRegionIndex) );

    QWSDisplay::ungrab();
}

/*
 return a pointer to the QRects at an offset
*/
QRect *QWSRegionManager::rects( int offset )
{
    QRect *start = (QRect *)(data + sizeof(QWSRegionHeader) + sizeof(QWSRegionIndex) * QT_MAX_REGIONS);
    return start + offset;
}

/*
 Attach to shared memory.  The server attaches with read/write priveliges.
 Clients attach with read privileges only.
*/
bool QWSRegionManager::attach( const QString &filename )
{
    int shmId;
    key_t key = ftok( filename.latin1(), 'r' );
    if ( !client ) {
	int dataSize = sizeof(QWSRegionHeader)                // header
		    + sizeof(QWSRegionIndex) * QT_MAX_REGIONS // + index
		    + sizeof(QRect) * regHdr->maxRects;       // + rects

	shmId = shmget( key, dataSize, IPC_CREAT|0666);
	if ( shmId != -1 )
	    data = (unsigned char *)shmat( shmId, 0, 0 );
    } else {
	shmId = shmget( key, 0, 0 );
	if ( shmId != -1 )
	    data = (unsigned char *)shmat( shmId, 0, SHM_RDONLY );
    }

    return ( shmId != -1 && (int)data != -1 );
}

/*
 Detach shared memory.
*/
void QWSRegionManager::detach()
{
    shmdt( data );
}

