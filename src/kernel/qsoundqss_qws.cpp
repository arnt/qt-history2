/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_qws.cpp#8 $
**
** Implementation of Qt Sound System
**
** Created : 001017
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsoundqss_qws.h"

#ifndef QT_NO_SOUND

#include <qlist.h>
#include <qsocketnotifier.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#define QT_QWS_SOUND_16BIT 1 // or 0, or undefined for always 0
#define QT_QWS_SOUND_STEREO 1 // or 0, or undefined for always 0

static int sound_speed = 44100;
#ifndef QT_NO_NETWORK
static int sound_port = 4992;
#endif


struct QRiffChunk {
    char id[4];
    Q_UINT32 size;
    char data[4/*size*/];
};

static const int sound_fragment_size = 8;
static const int sound_buffer_size=1<<sound_fragment_size;

#ifdef QT_QWS_SOUND_STEREO
static int sound_stereo=QT_QWS_SOUND_STEREO;
#else
static const int sound_stereo=0;
#endif
#ifdef QT_QWS_SOUND_16BIT
static bool sound_16bit=QT_QWS_SOUND_16BIT;
#else
static const bool sound_16bit=FALSE;
#endif


#ifndef QT_NO_NETWORK
QWSSoundServerClient::QWSSoundServerClient(int s, QObject* parent) :
    QSocket(parent)
{
    setSocket(s);
    connect(this,SIGNAL(readyRead()),
	this,SLOT(tryReadCommand()));
    connect(this,SIGNAL(connectionClosed()),
	this,SLOT(destruct()));
}

QWSSoundServerClient::~QWSSoundServerClient()
{
}

void QWSSoundServerClient::destruct()
{
    delete this;
}

void QWSSoundServerClient::tryReadCommand()
{
    while ( canReadLine() ) {
	QString l = readLine();
	l.truncate(l.length()-1); // chomp
	QStringList token = QStringList::split(" ",l);
	if ( token[0] == "PLAY" )
	    emit play(token[1]);
    }
}
#endif


class QWSSoundServerBucket {
public:
    QWSSoundServerBucket(QIODevice* d)
    {
	dev = d;
	max = out = sound_buffer_size;
	wavedata_remaining = 0;
	samples_due = 0;
    }
    ~QWSSoundServerBucket()
    {
	delete dev;
    }
    int add(int* mixl, int* mixr, int count)
    {
	while ( count && dev ) {
	    int l,r;
	    getSample(l,r);
	    samples_due += sound_speed;
	    while ( count && samples_due > chunkdata.samplesPerSec ) {
		if ( mixl) *mixl++ += l;
		if ( sound_stereo && mixr) *mixr++ += r;
		samples_due -= chunkdata.samplesPerSec;
		count--;
	    }
	}
	return count;
    }
    bool finished() const
    {
	return wavedata_remaining < 0 || !max;
    }
private:
    void getSample(int& l, int& r)
    {
	l = r = 0;
	if ( wavedata_remaining < 0 || !max )
	    return; // in error state, return silence
	while ( 1 ) {
	    if ( wavedata_remaining > 0 ) {
		if ( out >= max ) {
		    max = dev->readBlock((char*)data,
			(uint)QMIN(sound_buffer_size,wavedata_remaining));
		    wavedata_remaining -= max;
		    out = 0;
		    if ( max <= 0 ) {
			max = 0;
			return;
		    }
		}
		if ( chunkdata.wBitsPerSample == 8 ) {
		    l = (data[out++] - 128) * 128;
		} else {
		    l = ((short*)data)[out/2];
		    out += 2;
		}
		if ( sound_stereo ) {
		    if ( chunkdata.channels == 1 ) {
			r = l;
		    } else {
			if ( chunkdata.wBitsPerSample == 8 ) {
			    r = (data[out++] - 128) * 128;
			} else {
			    r = ((short*)data)[out/2];
			    out += 2;
			}
		    }
		} else {
		    if ( chunkdata.channels == 2 ) {
			if ( chunkdata.wBitsPerSample == 8 ) {
			    r = (data[out++] - 128) * 128;
			} else {
			    r = ((short*)data)[out/2];
			    out += 2;
			}
			l = l + r;
		    }
		}
		return;
	    } else {
		wavedata_remaining = -1;
		// Keep reading chunks...
		const int n = sizeof(chunk)-sizeof(chunk.data);
		if ( dev->readBlock((char*)&chunk,n) != n )
		    return;
		if ( qstrncmp(chunk.id,"data",4) == 0 ) {
		    wavedata_remaining = chunk.size;
		} else if ( qstrncmp(chunk.id,"RIFF",4) == 0 ) {
		    char d[4];
		    if ( dev->readBlock(d,4) != 4 )
			return;
		    if ( qstrncmp(d,"WAVE",4) != 0 ) {
			// skip
			if ( chunk.size > 1000000000 || !dev->at(dev->at()+chunk.size-4) )
			    return;
		    }
		} else if ( qstrncmp(chunk.id,"fmt ",4) == 0 ) {
		    if ( dev->readBlock((char*)&chunkdata,sizeof(chunkdata)) != sizeof(chunkdata) )
			return;
#define WAVE_FORMAT_PCM 1
		    if ( chunkdata.formatTag != WAVE_FORMAT_PCM ) {
			//qDebug("WAV file: UNSUPPORTED FORMAT %d",chunkdata.formatTag);
			return;
		    }
		} else {
		    // ignored chunk
		    if ( chunk.size > 1000000000 || !dev->at(dev->at()+chunk.size) )
			return;
		}
	    }
	}
    }
    struct {
	Q_INT16 formatTag;
	Q_INT16 channels;
	Q_INT32 samplesPerSec;
	Q_INT32 avgBytesPerSec;
	Q_INT16 blockAlign;
	Q_INT16 wBitsPerSample;
    } chunkdata;
    QRiffChunk chunk;
    int wavedata_remaining;

    QIODevice* dev;
    uchar data[sound_buffer_size+4]; // +4 to handle badly aligned input data
    int out,max;
    int samples_due;
};

#ifndef QT_NO_NETWORK
QWSSoundServerSocket::QWSSoundServerSocket(QObject* parent, const char* name) :
    QServerSocket(sound_port, 0, parent, name)
{
}


void QWSSoundServerSocket::newConnection(int s)
{
    QWSSoundServerClient* client = new QWSSoundServerClient(s,this);
    connect(client, SIGNAL(play(const QString&)),
	this, SIGNAL(playFile(const QString&)));
}
#endif

class QWSSoundServerData : public QObject {
    Q_OBJECT

public:
    QWSSoundServerData(QObject* parent=0, const char* name=0) :
	QObject(parent, name)
    {
#ifndef QT_NO_NETWORK
	server = new QWSSoundServerSocket(this);
	connect(server, SIGNAL(playFile(const QString&)),
		this, SLOT(playFile(const QString&)));
#endif
	sn = 0;
	active.setAutoDelete(TRUE);
	unwritten = 0;
    }

public slots:
    void playFile(const QString& filename)
    {
	QFile* f = new QFile(filename);
	if ( f->open(IO_ReadOnly) ) {
	    active.append(new QWSSoundServerBucket(f));
	    openDevice();
	} else {
	    qDebug("Failed opening \"%s\"",filename.latin1());
	}
    }

    void feedDevice(int fd)
    {
	if ( !unwritten ) {
	    QWSSoundServerBucket* bucket;
	    int blank = sound_buffer_size;
	    int left[sound_buffer_size];
	    memset(left,0,sound_buffer_size*sizeof(int));
	    int right[sound_buffer_size];
	    if ( sound_stereo )
		memset(right,0,sound_buffer_size*sizeof(int));
	    for (bucket = active.first(); bucket; bucket = active.next()) {
		int unused = bucket->add(left,right,sound_buffer_size);
		if ( unused < blank )
		    blank = unused;
	    }
	    int available = sound_buffer_size - blank;
#ifdef QT_QWS_SOUND_16BIT
	    short *d = d16;
	    for (int i=0; i<available; i++) {
		int l = left[i];
		if ( l > 32767 ) l = 32767;
		if ( l < -32768 ) l = -32768;
		*d++ = (short)l;
		if ( sound_stereo ) {
		    int r = right[i];
		    if ( r > 32767 ) r = 32767;
		    if ( r < -32768 ) r = -32768;
		    *d++ = (short)r;
		}
	    }
	    unwritten = available*2*(sound_stereo+1);
	    cursor = (char*)d16;
#else
	    signed char *d = d8;
	    for (int i=0; i<available; i++) {
		int l = left[i] / 256;
		if ( l > 127 ) l = 127;
		if ( l < -128 ) l = -128;
		*d++ = (signed char)l+128;
		if ( sound_stereo ) {
		    int r = right[i] / 256;
		    if ( r > 127 ) r = 127;
		    if ( r < -128 ) r = -128;
		    *d++ = (signed char)r+128;
		}
	    }
	    unwritten = available*(sound_stereo+1);
	    cursor = (char*)d8;
#endif
	    QListIterator<QWSSoundServerBucket> it(active);
	    for (; (bucket = *it);) {
		++it;
		if ( bucket->finished() )
		    active.removeRef(bucket);
	    }
	}

	int w = ::write(fd,cursor,unwritten);

	if ( w < 0 )
	    return;

	cursor += w;
	unwritten -= w;

	if ( !unwritten && active.count() == 0 )
	    closeDevice();
    }

protected:
    void timerEvent(QTimerEvent* event)
    {
	if ( sn )
	    feedDevice(sn->socket());
	if ( !sn )
	    killTimer(event->timerId());
    }

private:
    void openDevice()
    {
	if ( !sn ) {
	    int fd = ::open("/dev/dsp",O_WRONLY|O_NONBLOCK);
	    if ( fd < 0 ) {
		// For debugging purposes - defined QT_NO_SOUND if you
		// don't have sound hardware!
		fd = ::open("/tmp/dsp",O_WRONLY);
	    }

	    // Setup soundcard at 16 bit mono
	    int v;
	    v=0x00040000+sound_fragment_size; ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &v);
#ifdef QT_QWS_SOUND_16BIT
	    v=AFMT_S16_LE; if ( ioctl(fd, SNDCTL_DSP_SETFMT, &v) )
		qWarning("Could not set format %d",v);
	    qDebug("Want format %d got %d", AFMT_S16_LE, v);
#else
	    v=AFMT_U8; if ( ioctl(fd, SNDCTL_DSP_SETFMT, &v) )
		qWarning("Could not set format %d",v);
	    qDebug("Want format %d got %d", AFMT_U8, v);
#endif
	    v=sound_stereo; if ( ioctl(fd, SNDCTL_DSP_STEREO, &v) )
		qWarning("Could not set stereo %d",v);
	    qDebug("Want stereo %d got %d", sound_stereo, v);
#ifdef QT_QWS_SOUND_STEREO
	    sound_stereo=v;
#endif
	    v=sound_speed; if ( ioctl(fd, SNDCTL_DSP_SPEED, &sound_speed) )
		qWarning("Could not set speed %d",v);
	    qDebug("Want speed %d got %d", v, sound_speed);

	    sn = new QSocketNotifier(fd,QSocketNotifier::Write,this);
	    QObject::connect(sn,SIGNAL(activated(int)),this,SLOT(feedDevice(int)));
#ifdef QT_QWS_IPAQ
	    startTimer(0);
#endif
	}
    }

    void closeDevice()
    {
	if ( sn ) {
	    ::close(sn->socket());
	    delete sn;
	    sn = 0;
	}
    }

    QList<QWSSoundServerBucket> active;
    QSocketNotifier* sn;
    int unwritten;
    char* cursor;
#ifdef QT_QWS_SOUND_16BIT
    short d16[sound_buffer_size*2];
#else
    signed char d8[sound_buffer_size*2];
#endif
#ifndef QT_NO_NETWORK
    QWSSoundServerSocket *server;
#endif
};

QWSSoundServer::QWSSoundServer(QObject* parent) :
    QObject(parent)
{
    d = new QWSSoundServerData(this);
}

void QWSSoundServer::playFile( const QString& filename )
{
    d->playFile(filename);
}

QWSSoundServer::~QWSSoundServer()
{
}

#ifndef QT_NO_NETWORK
QWSSoundClient::QWSSoundClient( QObject* parent ) :
    QSocket(parent)
{
    connectToHost("localhost", sound_port);
}

void QWSSoundClient::play( const QString& filename )
{
    QFileInfo fi(filename);
    QCString u = ("PLAY " + fi.absFilePath() + "\n").utf8();
    writeBlock(u.data(), u.length());
}
#endif

#include "qsoundqss_qws.moc"

#endif	// QT_NO_SOUND
