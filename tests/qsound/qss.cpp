#include "qss.h"
#include <qptrlist.h>
#include <qsocketnotifier.h>
#include <qfile.h>
#include <qserversocket.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#define QT_QWS_SOUND_16BIT 1 // or 0, or undefined for always 0
#define QT_QWS_SOUND_STEREO 1 // or 0, or undefined for always 0

static int sound_speed = 22050;
static int sound_port = 4992;


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
static bool sound_stereo=FALSE;
#endif


class QWSSoundServerClient : public QSocket {
    Q_OBJECT

public:
    QWSSoundServerClient(int s, QObject* parent) :
	QSocket(parent)
    {
	setSocket(s);
	connect(this,SIGNAL(readyRead()),
	    this,SLOT(tryReadCommand()));
	connect(this,SIGNAL(connectionClosed()),
	    this,SLOT(destruct()));
    }
    ~QWSSoundServerClient()
    {
    }

signals:
    void play(const QString&);

private slots:
    void destruct()
    {
	delete this;
    }

    void tryReadCommand()
    {
	while ( canReadLine() ) {
	    QString l = readLine();
	    l.truncate(l.length()-1); // chomp
	    QStringList token = QStringList::split(" ",l);
	    if ( token[0] == "PLAY" )
		emit play(token[1]);
	}
    }
};



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

class QWSSoundServerData : public QServerSocket {
    Q_OBJECT

public:
    QWSSoundServerData(QObject* parent=0, const char* name=0) :
	QServerSocket(sound_port, 0, parent, name)
    {
	active.setAutoDelete(TRUE);
	sn = 0;
    }

    void newConnection(int s)
    {
	QWSSoundServerClient* client = new QWSSoundServerClient(s,this);
	connect(client, SIGNAL(play(const QString&)),
	    this, SLOT(playFile(const QString&)));
    }

private slots:
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
	short d16[sound_buffer_size*2];
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
	int w = ::write(fd,(char*)d16,available*2*(sound_stereo+1));
#else
	signed char d8[sound_buffer_size*2];
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
	int w = ::write(fd,d8,available*(sound_stereo+1));
#endif
	if ( w < 0 )
	    return;

	QPtrListIterator<QWSSoundServerBucket> it(active);
	for (; (bucket = *it);) {
	    ++it;
	    if ( bucket->finished() )
		active.removeRef(bucket);
	}
    }

private:
    void openDevice()
    {
	if ( !sn ) {
	    int fd = ::open("/dev/dsp",O_WRONLY);

	    // Setup soundcard at 16 bit mono
	    int v;
	    v=0x00040000+sound_fragment_size; ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &v);
#ifdef QT_QWS_SOUND_16BIT
	    v=AFMT_S16_LE; ioctl(fd, SNDCTL_DSP_SETFMT, &v);
#else
	    v=AFMT_U8; ioctl(fd, SNDCTL_DSP_SETFMT, &v);
#endif
	    v=sound_stereo; ioctl(fd, SNDCTL_DSP_STEREO, &v);
#ifdef QT_QWS_SOUND_STEREO
	    sound_stereo=v;
#endif
	    ioctl(fd, SNDCTL_DSP_SPEED, &sound_speed);

	    sn = new QSocketNotifier(fd,QSocketNotifier::Write,this);
	    QObject::connect(sn,SIGNAL(activated(int)),this,SLOT(feedDevice(int)));
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

    QPtrList<QWSSoundServerBucket> active;
    QSocketNotifier* sn;
};

QWSSoundServer::QWSSoundServer(QObject* parent) :
    QObject(parent)
{
    d = new QWSSoundServerData(this);
}

QWSSoundServer::~QWSSoundServer()
{
}

QWSSoundClient::QWSSoundClient( QObject* parent ) :
    QSocket(parent)
{
    connectToHost("localhost", sound_port);
}

void QWSSoundClient::play( const QString& filename )
{
    QCString u = ("PLAY " + filename + "\n").utf8();
    writeBlock(u.data(), u.length());
}

#include "qss.moc"
