/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#21 $
**
** Definition of QFile class
**
** Created : 930831
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFILE_H
#define QFILE_H

#include "qiodev.h"
#include "qstring.h"
#include <stdio.h>

class QDir;


class QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const char *name );
   ~QFile();

    const char *name()	const;
    void	setName( const char *name );

    bool	exists()   const;
    static bool exists( const char *fileName );

    bool	remove();
    static bool remove( const char *fileName );

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
    int		readLine( char *data, uint maxlen );

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

private:
    void	init();

private:	// Disabled copy constructor and operator=
    QFile( const QFile & );
    QFile &operator=( const QFile & );
};


inline const char *QFile::name() const
{ return fn; }

inline int QFile::at() const
{ return index; }


#endif // QFILE_H
