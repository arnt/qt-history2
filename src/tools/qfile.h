/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#38 $
**
** Definition of QFile class
**
** Created : 930831
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFILE_H
#define QFILE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QDir;


class Q_EXPORT QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const QString &name );
   ~QFile();

    QString	name()	const;
    void	setName( const QString &name );

    typedef QCString (*EncoderFn)( const QString &fileName );
    typedef QString (*DecoderFn)( const QCString &localfileName );
    static QCString encodeName( const QString &fileName );
    static QString decodeName( const QCString &localFileName );
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

    uint	size()	const;
    int		at()	const;
    bool	at( int );
    bool	atEnd() const;

    int		readBlock( char *data, uint len );
    int		writeBlock( const char *data, uint len );
    int		writeBlock( const QByteArray& data )
		    { return QIODevice::writeBlock(data); }
    int		readLine( char *data, uint maxlen );
    int		readLine( QString &, uint maxlen );

    int		getch();
    int		putch( int );
    int		ungetch( int );

    int		handle() const;

protected:
    QString	fn;
    FILE       *fh;
    int		fd;
    int		length;
    bool	ext_f;
    void * 	d;

private:
    void	init();
    QCString ungetchBuffer;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFile( const QFile & );
    QFile &operator=( const QFile & );
#endif
};


inline QString QFile::name() const
{ return fn; }

inline int QFile::at() const
{ return ioIndex; }


#endif // QFILE_H
