/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#2 $
**
** Definition of QFile class
**
** Author  : Haavard Nord
** Created : 930831
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
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

    char *fileName() const { return fn.data(); }// get file name
    bool  setFileName( const char *fileName );	// set file name

    static bool exists( const char *fileName ); // test if file exists
    bool  exists()	  const;
    bool  isRegular()     const;
    bool  isDirectory()	  const;
    bool  isSymLink()	  const;

    bool  remove( const char *fileName=0 );	// remove file

    bool  open( int );				// open file
    bool  open( int, FILE * );			// open file, using file handle
    bool  open( int, int );			// open file, using file descr
    void  close();				// close file
    void  flush();				// flush file

    long  size() const;				// get file size
    long  at()   const;				// get file pointer
    bool  at( long );				// set file pointer

    int   readBlock( char *data, uint len );
    int   writeBlock( const char *data, uint len );

    int	  getch();				// get next char
    int	  putch( int );				// put char
    int	  ungetch( int ) ;			// put back char

protected:
    QString  fn;				// file name
    FILE    *fh;				// file handle
    int	     fd;				// file descriptor (raw)
    long     length;				// file length

private:
    void  init();   
    long  get_stat( bool=FALSE ) const;
};


#endif // QFILE_H
