/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#6 $
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

class QDir;

class QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const char *fullPathFileName );
    QFile( const QDir &, const char *fileName );
   ~QFile();

    const char *fileName() const;		// get/set file name
    void	setFileName( const char *fullPathFileName );
    void	setFileName( const QDir &, const char *fileName );

    bool	exists()	const;
    bool	isFile()	const;		// is it a regular file?
    bool	isDir()		const;		// is it a directory?
    bool	isSymLink()	const;		// is it a symlink?

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

    bool        remove();
    static bool remove( const QDir &, const char *fileName );
    static bool remove( const char *fullPathFileName );

    static bool exists( const char *fullPathFileName ); // test if file exists
    static bool exists( const QDir &, const char *fileName );
protected:
    QString	fn;				// file name
    FILE       *fh;				// file handle (buffered)
    int		fd;				// file descriptor (raw)
    long	length;				// file length

private:
    void	init();
    long	get_stat( bool=FALSE ) const;

    friend class QFileInfo;
};


inline const char *QFile::fileName() const
{
    return fn;
}


#endif // QFILE_H
