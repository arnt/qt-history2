/****************************************************************************
**
** Definition of QIODevice class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIODEVICE_H
#define QIODEVICE_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


// IO device access types

#define IO_Direct		0x0100		// direct access device
#define IO_Sequential		0x0200		// sequential access device
#define IO_Combined		0x0300		// combined direct/sequential
#define IO_TypeMask		0x0f00

// IO handling modes

#define IO_Raw			0x0040		// raw access (not buffered)
#define IO_Async		0x0080		// asynchronous mode

// IO device open modes

#define IO_ReadOnly		0x0001		// readable device
#define IO_WriteOnly		0x0002		// writable device
#define IO_ReadWrite		0x0003		// read+write device
#define IO_Append		0x0004		// append
#define IO_Truncate		0x0008		// truncate device
#define IO_Translate		0x0010		// translate CR+LF
#define IO_ModeMask		0x00ff

// IO device state

#define IO_Open			0x1000		// device is open
#define IO_StateMask		0xf000

// IO device status

#define IO_Ok			0
#define IO_ReadError		1		// read error
#define IO_WriteError		2		// write error
#define IO_FatalError		3		// fatal unrecoverable error
#define IO_ResourceError	4		// resource limitation
#define IO_OpenError		5		// cannot open device
#define IO_ConnectError		5		// cannot connect to device
#define IO_AbortError		6		// abort error
#define IO_TimeOutError		7		// time out
#define IO_UnspecifiedError	8		// unspecified error

class QByteArray;


class Q_CORE_EXPORT QIODevice
{
public:
    typedef Q_LLONG Offset;

    QIODevice();
    virtual ~QIODevice();

    int		 flags()  const { return ioMode; }
    int		 mode()	  const { return ioMode & IO_ModeMask; }
    int		 state()  const { return ioMode & IO_StateMask; }

    bool	 isDirectAccess()     const { return ((ioMode & IO_Direct)     == IO_Direct); }
    bool	 isSequentialAccess() const { return ((ioMode & IO_Sequential) == IO_Sequential); }
    bool	 isCombinedAccess()   const { return ((ioMode & IO_Combined)   == IO_Combined); }
    bool	 isBuffered()	      const { return ((ioMode & IO_Raw)        != IO_Raw); }
    bool	 isRaw()	      const { return ((ioMode & IO_Raw)        == IO_Raw); }
    bool	 isSynchronous()      const { return ((ioMode & IO_Async)      != IO_Async); }
    bool	 isAsynchronous()     const { return ((ioMode & IO_Async)      == IO_Async); }
    bool	 isTranslated()	      const { return ((ioMode & IO_Translate)  == IO_Translate); }
    bool	 isReadable()	      const { return ((ioMode & IO_ReadOnly)   == IO_ReadOnly); }
    bool	 isWritable()	      const { return ((ioMode & IO_WriteOnly)  == IO_WriteOnly); }
    bool	 isReadWrite()	      const { return ((ioMode & IO_ReadWrite)  == IO_ReadWrite); }
    bool	 isInactive()	      const { return state() == 0; }
    bool	 isOpen()	      const { return state() == IO_Open; }

    int		 status() const { return ioSt; }
    void	 resetStatus()	{ ioSt = IO_Ok; }

    virtual bool open( int mode ) = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

    virtual Offset size()  const = 0;
    virtual Offset at()  const;
    virtual bool at( Offset );
    virtual bool atEnd()  const;
    bool	 reset() { return at(0); }

    virtual Q_LONG readBlock( char *data, Q_ULONG maxlen ) = 0;
    virtual Q_LONG writeBlock( const char *data, Q_ULONG len ) = 0;
    virtual Q_LONG readLine( char *data, Q_ULONG maxlen );
    Q_LONG writeBlock( const QByteArray& data );
    virtual QByteArray readAll();

    virtual int	 getch() = 0;
    virtual int	 putch( int ) = 0;
    virtual int	 ungetch( int ) = 0;

protected:
    void	 setFlags( int f ) { ioMode = f; }
    void	 setType( int );
    void	 setMode( int );
    void	 setState( int );
    void	 setStatus( int );
    Offset	 ioIndex;

private:
    int		 ioMode;
    int		 ioSt;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QIODevice( const QIODevice & );
    QIODevice &operator=( const QIODevice & );
#endif
};


#endif // QIODEVICE_H
