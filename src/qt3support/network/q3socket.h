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

#ifndef Q3SOCKET_H
#define Q3SOCKET_H

#include <QtCore/qiodevice.h>
#include <QtNetwork/qhostaddress.h> // int->QHostAddress conversion

QT_MODULE(Qt3Support)

class Q3SocketPrivate;
class Q3SocketDevice;

class Q_COMPAT_EXPORT Q3Socket : public QIODevice
{
    Q_OBJECT
public:
    enum Error {
	ErrConnectionRefused,
	ErrHostNotFound,
	ErrSocketRead
    };

    Q3Socket( QObject *parent=0, const char *name=0 );
    virtual ~Q3Socket();

    enum State { Idle, HostLookup, Connecting,
		 Connected, Closing,
		 Connection=Connected };
    State	 state() const;

    int		 socket() const;
    virtual void setSocket( int );

    Q3SocketDevice *socketDevice();
    virtual void setSocketDevice( Q3SocketDevice * );

#ifndef QT_NO_DNS
    virtual void connectToHost( const QString &host, Q_UINT16 port );
#endif
    QString	 peerName() const;

    // Implementation of QIODevice abstract virtual functions
    bool	 open( int mode );
    void	 close();
    bool	 flush();
    Offset	 size() const;
    Offset	 at() const;
    bool	 at( Offset );
    bool	 atEnd() const;

    qint64	 bytesAvailable() const;
    Q_ULONG	 waitForMore( int msecs, bool *timeout  ) const;
    Q_ULONG	 waitForMore( int msecs ) const; // ### Qt 4.0: merge the two overloads
    qint64	 bytesToWrite() const;
    void	 clearPendingData();

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    bool	 canReadLine() const;

    Q_UINT16	 port() const;
    Q_UINT16	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    void	 setReadBufferSize( Q_ULONG );
    Q_ULONG	 readBufferSize() const;

    inline bool  isSequential() const { return !isOpen(); }

Q_SIGNALS:
    void	 hostFound();
    void	 connected();
    void	 connectionClosed();
    void	 delayedCloseFinished();
    void	 readyRead();
    void	 bytesWritten( int nbytes );
    void	 error( int );

protected Q_SLOTS:
    virtual void sn_read( bool force=false );
    virtual void sn_write();

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private Q_SLOTS:
    void	tryConnecting();
    void	emitErrorConnectionRefused();

private:
    Q3SocketPrivate *d;

    bool	 consumeWriteBuf( Q_ULONG nbytes );
    void	 tryConnection();
    void         setSocketIntern( int socket );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3Socket( const Q3Socket & );
    Q3Socket &operator=( const Q3Socket & );
#endif
};

#endif // Q3SOCKET_H
