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

#ifndef QSOUNDQSS_QWS_H
#define QSOUNDQSS_QWS_H

#include "QtNetwork/qtcpserver.h"
#include "QtNetwork/qtcpsocket.h"
#include "QtGui/qwssocket_qws.h"

#if defined(QT_NO_NETWORK) || defined(QT_NO_DNS)
#define QT_NO_QWS_SOUNDSERVER
#endif

QT_MODULE(Gui)

#ifndef QT_NO_SOUND

#ifndef Q_OS_MAC

class QWSSoundServerPrivate;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    explicit QWSSoundServer(QObject *parent=0);
    ~QWSSoundServer();
    void playFile( int id, const QString& filename );
    void stopFile( int id );
    void pauseFile( int id );
    void resumeFile( int id );
    
signals:
    void soundCompleted( int );
    
private slots:
    void translateSoundCompleted( int, int );

private:
    QWSSoundServerPrivate* d;
};

#ifndef QT_NO_QWS_SOUNDSERVER
class QWSSoundClient : public QWSSocket {
    Q_OBJECT
public:

    enum SoundFlags {
	Priority = 0x01,
	Streaming = 0x02  // currently ignored, but but could set up so both Raw and non raw can be done streaming or not.
    };
    enum DeviceErrors {
	ErrOpeningAudioDevice = 0x01,
	ErrOpeningFile = 0x02,
	ErrReadingFile = 0x04
    };
    explicit QWSSoundClient(QObject* parent=0);
    ~QWSSoundClient( );
    void reconnect();
    void play( int id, const QString& filename );
    void play( int id, const QString& filename, int volume, int flags = 0 );
    void playRaw( int id, const QString&, int, int, int, int flags = 0 );

    void pause( int id );
    void stop( int id );
    void resume( int id );
    void setVolume( int id, int left, int right );
    void setMute( int id, bool m );
    
    // to be used by server only, to protect phone conversation/rings.
    void playPriorityOnly(bool);
    
signals:
    void soundCompleted(int);
    void deviceReady(int id);
    void deviceError(int id, QWSSoundClient::DeviceErrors);

private slots:
    void tryReadCommand();
    void emitConnectionRefused();
    
private:
    void sendServerMessage(QString msg);
};

class QWSSoundServerSocket : public QWSServerSocket {
    Q_OBJECT

public:
    explicit QWSSoundServerSocket(QObject *parent=0);
public slots:    
    void newConnection();

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QWSSoundServerSocket(QObject *parent, const char *name);
#endif

signals:
    void playFile(int, int, const QString&);
    void playFile(int, int, const QString&, int, int);
    void playRawFile(int, int, const QString&, int, int, int, int);
    void pauseFile(int, int);
    void stopFile(int, int);
    void resumeFile(int, int);
    void setVolume(int, int, int, int);
    void setMute(int, int, bool);

    void stopAll(int);

    void playPriorityOnly(bool);

    void soundFileCompleted(int, int);
    void deviceReady(int, int);
    void deviceError(int, int, int);
};
#endif

#endif // Q_OS_MAC
#endif // QT_NO_SOUND

#endif // QSOUNDQSS_QWS_H
