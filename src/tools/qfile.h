/****************************************************************************
**
** Definition of QFile class.
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

#ifndef QFILE_H
#define QFILE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QDir;
class QFilePrivate;

class Q_KERNEL_EXPORT QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const QString &name );
   ~QFile();

    QString	name()	const;
    void	setName( const QString &name );

    typedef QByteArray (*EncoderFn)( const QString &fileName );
    typedef QString (*DecoderFn)( const QByteArray &localfileName );
    static QByteArray encodeName( const QString &fileName );
    static QString decodeName( const QByteArray &localFileName );
    static QString decodeName( const char *localFileName );
    static void setEncodingFunction( EncoderFn );
    static void setDecodingFunction( DecoderFn );

    bool	exists()   const;
    static bool exists( const QString &fileName );

    bool	remove();
    static bool remove( const QString &fileName );

    bool	open( int );
    bool	open( int, FILE * );
    bool	open( int, int );
    void	close();
    void	flush();

    Offset	size() const;
    Offset	at() const;
    bool	at( Offset );
    bool	atEnd() const;

    Q_LONG	readBlock( char *data, Q_ULONG len );
    Q_LONG	writeBlock( const char *data, Q_ULONG len );
    Q_LONG	writeBlock( const QByteArray& data )
		      { return QIODevice::writeBlock(data); }
    Q_LONG	readLine( char *data, Q_ULONG maxlen );
    Q_LONG	readLine( QString &, Q_ULONG maxlen );

    int		getch();
    int		putch( int );
    int		ungetch( int );

    int		handle() const;

    QString	errorString() const; // ### Qt 4: move into QIODevice

protected:
    void	setErrorString( const QString& ); // ### Qt 4: move into QIODevice
    QString	fn;
    FILE       *fh;
    int		fd;
    Offset	length;
    bool	ext_f;
    QFilePrivate *d; // ### Qt 4: make private

private:
    void	init();
    void	setErrorStringErrno( int );
    QByteArray	ungetchBuffer;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFile( const QFile & );
    QFile &operator=( const QFile & );
#endif
};


inline QString QFile::name() const
{ return fn; }

inline QIODevice::Offset QFile::at() const
{ return ioIndex; }


#endif // QFILE_H
