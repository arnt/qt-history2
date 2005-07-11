/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_SOUND

#include "qsound.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qsound_p.h"

#include "qsoundqss_qws.h"

#include "qhash.h"
#include "qfileinfo.h"

class QAuServerQWS;

class QAuBucketQWS : public QAuBucket
{
public:
    QAuBucketQWS( QAuServerQWS*, QSound* );
    
    ~QAuBucketQWS();
    
    int id() const { return id_; }
    
    QSound* sound() const { return sound_; }
    
private:
    int id_;
    QSound *sound_;
    QAuServerQWS *server_;
    
    static int next;
};

int QAuBucketQWS::next = 0;

class QAuServerQWS : public QAuServer
{
    Q_OBJECT
public:
    QAuServerQWS( QObject* parent );
    
    void init( QSound* s )
    {
        QAuBucketQWS *bucket = new QAuBucketQWS( this, s );
        setBucket( s, bucket );
    }
    
    // Register bucket
    void insert( QAuBucketQWS *bucket )
    {
        buckets.insert( bucket->id(), bucket );
    }
    
    // Remove bucket from register
    void remove( QAuBucketQWS *bucket )
    {
        buckets.remove( bucket->id() );
    }

    void play( QSound* s )
    {
        QString filepath = QFileInfo( s->fileName() ).absoluteFilePath();
#ifdef QT_NO_QWS_SOUNDSERVER
        server->playFile( bucket( s )->id(), filepath );
#else
        client->play( bucket( s )->id(), filepath );
#endif
    }
    
    void stop( QSound* s )
    {
#ifdef QT_NO_QWS_SOUNDSERVER
        server->stopFile( bucket( s )->id() );
#else
        client->stop( bucket( s )->id() );
#endif
    }

    bool okay() { return true; }
    
private slots:
    // Continue playing sound if loops remain
    void complete( int id )
    {
        QAuBucketQWS *bucket = find( id );
        if( bucket ) {
            QSound *sound = bucket->sound();
            if( decLoop( sound ) ) {
                play( sound );
            }
        }
    }
    
protected:
    QAuBucketQWS* bucket( QSound *s )
    {
        return (QAuBucketQWS*)QAuServer::bucket( s );
    }
    
private:
    // Find registered bucket with given id, return null if none found
    QAuBucketQWS* find( int id )
    {
        QHash<int, QAuBucketQWS*>::Iterator it = buckets.find( id );
        if( it != buckets.end() ) {
            return it.value();
        }
        
        return 0;
    }
    
    QHash<int, QAuBucketQWS*> buckets; // ### possible problem with overlapping keys
    
#ifdef QT_NO_QWS_SOUNDSERVER
    QWSSoundServer *server;
#else
    QWSSoundClient *client;
#endif
};

QAuServerQWS::QAuServerQWS(QObject* parent) :
    QAuServer(parent)
{
    setObjectName( "qauserverqws" );
    
#ifdef QT_NO_QWS_SOUNDSERVER
    server = new QWSSoundServer( this ); // ### only suitable for single application
    
    connect( server, SIGNAL( soundCompleted( int ) ),
        this, SLOT( complete( int ) ) );
#else
    client = new QWSSoundClient( this ); // ### requires successful connection
    
    connect( client, SIGNAL( soundCompleted( int ) ),
        this, SLOT( complete( int ) ) );
#endif
}

QAuBucketQWS::QAuBucketQWS( QAuServerQWS *server, QSound *sound )
    : sound_( sound ), server_( server )
{
    id_ = next++;
    server_->insert( this );
}
    
QAuBucketQWS::~QAuBucketQWS()
{
    server_->remove( this );
}


QAuServer* qt_new_audio_server()
{
    return new QAuServerQWS(qApp);
}

#include "qsound_qws.moc"

#endif // QT_NO_SOUND
