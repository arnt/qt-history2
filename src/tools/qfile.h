/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#5 $
**
** Definition of QFile class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFILE_H
#define QFILE_H

#include "qiodev.h"
#include "qstring.h"
#include <stdio.h>


class QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const char *fileName );
   ~QFile();

    const char *fileName() const;		// get/set file name
    bool	setFileName( const char *fileName );

    static bool exists( const char *fileName ); // test if file exists
    bool	exists()	const;
    bool	isRegular()	const;		// is it a regular file?
    bool	isDirectory()	const;		// is it a directory?
    bool	isSymLink()	const;		// is it a symlink?

    bool	remove( const char *fileName=0);// remove file

    bool	open( int );			// open file
    bool	open( int, FILE * );		// open file, using file handle
    bool	open( int, int );		// open file, using file descr
    void	close();			// close file
    void	flush();			// flush file

    long	size()	const;			// get file size
    long	at()	const;			// get file pointer
    bool	at( long );			// set file pointer

    int		readBlock( char *data, uint len );
    int		writeBlock( const char *data, uint len );
    int		readLine( char *data, uint maxlen );

    int		getch();			// get next char
    int		putch( int );			// put char
    int		ungetch( int ) ;		// put back char

protected:
    QString	fn;				// file name
    FILE       *fh;				// file handle (buffered)
    int		fd;				// file descriptor (raw)
    long	length;				// file length

private:
    void	init();
    long	get_stat( bool=FALSE ) const;
};


inline const char *QFile::fileName() const
{
    return fn;
}


#endif // QFILE_H
