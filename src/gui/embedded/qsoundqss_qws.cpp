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

#include "qsoundqss_qws.h"

#ifndef QT_NO_SOUND
#include <qbytearray.h>
#include <qlist.h>
#include <qsocketnotifier.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qevent.h>
#include <qalgorithms.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <qdebug.h>

#define QT_QWS_SOUND_16BIT 1 // or 0, or undefined for always 0
#define QT_QWS_SOUND_STEREO 1 // or 0, or undefined for always 0

static int sound_speed = 44100;
#ifndef QT_NO_QWS_SOUNDSERVER
static int sound_port = 4992;
#endif


struct QRiffChunk {
    char id[4];
    Q_UINT32 size;
    char data[4/*size*/];
};

static const int sound_fragment_size = 13;
static const int sound_buffer_size=1<<sound_fragment_size;

#ifdef QT_QWS_SOUND_STEREO
static int sound_stereo=QT_QWS_SOUND_STEREO;
#else
static const int sound_stereo=0;
#endif
#ifdef QT_QWS_SOUND_16BIT
static bool sound_16bit=QT_QWS_SOUND_16BIT;
#else
static const bool sound_16bit=false;
#endif

#ifndef QT_NO_QWS_SOUNDSERVER
QWSSoundServerClient::QWSSoundServerClient(int s, QObject* parent) :
    QTcpSocket(parent)
{
    setSocketDescriptor(s);
    connect(this,SIGNAL(readyRead()),
        this,SLOT(tryReadCommand()));
    connect(this,SIGNAL(closed()),
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
    while (canReadLine()) {
        QString l = readLine();
        l.chop(1);
        QStringList token = l.split(" ");
        if (token[0] == "PLAY")
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
        wavedata_remaining = -1;
        samples_due = 0;
    }
    ~QWSSoundServerBucket()
    {
        delete dev;
    }
    int add(int* mixl, int* mixr, int count)
    {
        while (count && dev) {
            int l,r;
            getSample(l,r);
            samples_due += sound_speed;
            while (count && samples_due > chunkdata.samplesPerSec) {
                if (mixl) *mixl++ += l;
                if (sound_stereo && mixr) *mixr++ += r;
                samples_due -= chunkdata.samplesPerSec;
                count--;
            }
        }
        return count;
    }
    bool finished() const
    {
        return !max;
    }
private:
    void getSample(int& l, int& r)
    {
        l = r = 0;
        while (wavedata_remaining < 0) {
            max = 0;
            wavedata_remaining = -1;
            // Keep reading chunks...
            const int n = sizeof(chunk)-sizeof(chunk.data);
            if (dev->read((char*)&chunk,n) != n)
                return;
            if (qstrncmp(chunk.id,"data",4) == 0) {
                wavedata_remaining = chunk.size;
                out = max = sound_buffer_size;
            } else if (qstrncmp(chunk.id,"RIFF",4) == 0) {
                char d[4];
                if (dev->read(d,4) != 4)
                    return;
                if (qstrncmp(d,"WAVE",4) != 0) {
                    // skip
                    if (chunk.size > 1000000000 || !dev->seek(dev->pos()+chunk.size-4))
                        return;
                }
            } else if (qstrncmp(chunk.id,"fmt ",4) == 0) {
                if (dev->read((char*)&chunkdata,sizeof(chunkdata)) != sizeof(chunkdata))
                    return;
#define WAVE_FORMAT_PCM 1
                if (chunkdata.formatTag != WAVE_FORMAT_PCM) {
                    //qDebug("WAV file: UNSUPPORTED FORMAT %d",chunkdata.formatTag);
                    return;
                }
            } else {
                // ignored chunk
                if (chunk.size > 1000000000 || !dev->seek(dev->pos()+chunk.size))
                    return;
            }
        }
        if (wavedata_remaining >= 0) {
            if (out >= max) {
                max = dev->read((char*)data,
                    (uint)qMin(sound_buffer_size,wavedata_remaining));
                wavedata_remaining -= max;
                out = 0;
                if (max <= 0) {
                    max = 0;
                    return;
                }
            }
            if (chunkdata.wBitsPerSample == 8) {
                l = (data[out++] - 128) * 128;
            } else {
                l = ((short*)data)[out/2];
                out += 2;
            }
            if (sound_stereo) {
                if (chunkdata.channels == 1) {
                    r = l;
                } else {
                    if (chunkdata.wBitsPerSample == 8) {
                        r = (data[out++] - 128) * 128;
                    } else {
                        r = ((short*)data)[out/2];
                        out += 2;
                    }
                }
            } else {
                if (chunkdata.channels == 2) {
                    if (chunkdata.wBitsPerSample == 8) {
                        r = (data[out++] - 128) * 128;
                    } else {
                        r = ((short*)data)[out/2];
                        out += 2;
                    }
                    l = l + r;
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

#ifndef QT_NO_QWS_SOUNDSERVER
QWSSoundServerSocket::QWSSoundServerSocket(QObject* parent, const char* name) :
    QTcpServer(parent)
{
    if (name)
        setObjectName(name);
    listen(sound_port);
}


void QWSSoundServerSocket::incomingConnection(int s)
{
    QWSSoundServerClient* client = new QWSSoundServerClient(s,this);
    connect(client, SIGNAL(play(QString)),
        this, SIGNAL(playFile(QString)));
}
#endif

class QWSSoundServerPrivate : public QObject {
    Q_OBJECT

public:
    QWSSoundServerPrivate(QObject* parent=0, const char* name=0) :
        QObject(parent)
    {
        setObjectName(name);
#ifndef QT_NO_QWS_SOUNDSERVER
        server = new QWSSoundServerSocket(this);
        connect(server, SIGNAL(playFile(QString)),
                this, SLOT(playFile(QString)));
#endif
        timerId = 0;
        fd = -1;
        unwritten = 0;
        can_GETOSPACE = true;
    }
    ~QWSSoundServerPrivate()
    {
        qDeleteAll(active);
    }
public slots:
    void playFile(const QString& filename);
    void feedDevice(int fd);


protected:
    void timerEvent(QTimerEvent* event);

private:
    bool openDevice();
    void closeDevice()
    {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    QList<QWSSoundServerBucket*> active;
    int fd;
    int unwritten;
    int timerId;
    char* cursor;
    short d16[sound_buffer_size*2];
    signed char d8[sound_buffer_size*2];
    bool can_GETOSPACE;
#ifndef QT_NO_QWS_SOUNDSERVER
    QWSSoundServerSocket *server;
#endif
};

void QWSSoundServerPrivate::timerEvent(QTimerEvent* event)
{
    if (fd >= 0)
        feedDevice(fd);

    if (fd < 0 && event->timerId() == timerId) {
        killTimer(timerId);
        timerId = 0;
    }
}

void QWSSoundServerPrivate::playFile(const QString& filename)
{
        QFile* f = new QFile(filename);
        if (f->open(QIODevice::ReadOnly)) {
            if (openDevice())
                active.append(new QWSSoundServerBucket(f));
            else
                delete f;
        } else {
            qDebug() << "Failed opening:" << filename;
        }
}

bool QWSSoundServerPrivate::openDevice()
{
        if (fd < 0) {
            //
            // Don't block open right away.
            //
            if ((fd = ::open("/dev/dsp", O_WRONLY|O_NONBLOCK)) != -1) {
                int flags = fcntl(fd, F_GETFL);
                flags &= ~O_NONBLOCK;
                if (fcntl(fd, F_SETFL, flags) != 0) {
                    return false;
                }
            } else {
                return false;
            }

            // Setup soundcard at 16 bit mono
            int v;
            v=0x00010000+sound_fragment_size;
            if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &v))
                qWarning("Could not set fragments to %08x",v);
#ifdef QT_QWS_SOUND_16BIT
            v=AFMT_S16_LE; if (ioctl(fd, SNDCTL_DSP_SETFMT, &v))
                qWarning("Could not set format %d",v);
            if (AFMT_S16_LE != v)
                qDebug("Want format %d got %d", AFMT_S16_LE, v);
#else
            v=AFMT_U8; if (ioctl(fd, SNDCTL_DSP_SETFMT, &v))
                qWarning("Could not set format %d",v);
            if (AFMT_U8 != v)
                qDebug("Want format %d got %d", AFMT_U8, v);
#endif
            v=sound_stereo; if (ioctl(fd, SNDCTL_DSP_STEREO, &v))
                qWarning("Could not set stereo %d",v);
            if (sound_stereo != v)
                qDebug("Want stereo %d got %d", sound_stereo, v);
#ifdef QT_QWS_SOUND_STEREO
            sound_stereo=v;
#endif
            v=sound_speed; if (ioctl(fd, SNDCTL_DSP_SPEED, &sound_speed))
                qWarning("Could not set speed %d",v);
            if (v != sound_speed)
                qDebug("Want speed %d got %d", v, sound_speed);

            int delay = 1000*(sound_buffer_size>>(sound_stereo+sound_16bit))
                                    /sound_speed/2;
            timerId = startTimer(delay);
        }
        return true;
}

void  QWSSoundServerPrivate::feedDevice(int fd)
{
        if (!unwritten && active.count() == 0) {
            closeDevice();
            return;
        }

        audio_buf_info info;
        if (can_GETOSPACE && ioctl(fd,SNDCTL_DSP_GETOSPACE,&info)) {
            can_GETOSPACE = false;
            fcntl(fd, F_SETFL, O_NONBLOCK);
        }
        if (!can_GETOSPACE)
            info.fragments = 4; // #### configurable?
        if (info.fragments > 0) {
            if (!unwritten) {
                QWSSoundServerBucket* bucket;
                int blank = sound_buffer_size;
                int left[sound_buffer_size];
                memset(left,0,sound_buffer_size*sizeof(int));
                int right[sound_buffer_size];
                if (sound_stereo)
                    memset(right,0,sound_buffer_size*sizeof(int));
                for (int i = 0; i < active.size(); ++i) {
                    bucket = active.at(i);
                    int unused = bucket->add(left,right,sound_buffer_size);
                    if (unused < blank)
                        blank = unused;
                }
                int available = sound_buffer_size - blank;
                if (sound_16bit) {
                    short *d = d16;
                    for (int i=0; i<available; i++) {
                        int l = left[i];
                        if (l > 32767) l = 32767;
                        if (l < -32768) l = -32768;
                        *d++ = (short)l;
                        if (sound_stereo) {
                            int r = right[i];
                            if (r > 32767) r = 32767;
                            if (r < -32768) r = -32768;
                            *d++ = (short)r;
                        }
                    }
                    unwritten = available*2*(sound_stereo+1);
                    cursor = (char*)d16;
                } else {
                    signed char *d = d8;
                    for (int i=0; i<available; i++) {
                        int l = left[i] / 256;
                        if (l > 127) l = 127;
                        if (l < -128) l = -128;
                        *d++ = (signed char)l+128;
                        if (sound_stereo) {
                            int r = right[i] / 256;
                            if (r > 127) r = 127;
                            if (r < -128) r = -128;
                            *d++ = (signed char)r+128;
                        }
                    }
                    unwritten = available*(sound_stereo+1);
                    cursor = (char*)d8;
                }
                for (int i = 0; i < active.size(); ++i) {
                    bucket = active.at(i);
                    if (bucket->finished()) {
                        active.removeAll(bucket);
                        delete bucket;
                    }
                }
            }

            int w = ::write(fd,cursor,unwritten);

            if (w < 0)
                if (!can_GETOSPACE)
                    w = 0;
                else
                    return;

            cursor += w;
            unwritten -= w;

            if (!unwritten && active.count() == 0) {
                if (timerId)
                    killTimer(timerId);
                int delay = 1000*(w>>(sound_stereo+sound_16bit))/sound_speed;
                timerId = startTimer(delay);
            }
        }
}


QWSSoundServer::QWSSoundServer(QObject* parent) :
    QObject(parent)
{
    d = new QWSSoundServerPrivate(this);
}

void QWSSoundServer::playFile(const QString& filename)
{
    d->playFile(filename);
}

QWSSoundServer::~QWSSoundServer()
{
}

#ifndef QT_NO_QWS_SOUNDSERVER
QWSSoundClient::QWSSoundClient(QObject* parent) :
    QTcpSocket(parent)
{
    connectToHost("localhost", sound_port);
}

void QWSSoundClient::play(const QString& filename)
{
    QFileInfo fi(filename);
#ifndef QT_NO_TEXTCODEC
    QByteArray u = ("PLAY " + fi.absoluteFilePath() + "\n").toUtf8();
#else
    QByteArray u = ("PLAY " + fi.absoluteFilePath() + "\n").toLatin1();
#endif
    write(u.data(), u.length());
}
#endif

#include "qsoundqss_qws.moc"

#endif        // QT_NO_SOUND
