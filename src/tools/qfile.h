/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfile.h#1 $
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

#include "qstream.h"
#include "qstring.h"
#include <stdio.h>


class QFile : public QStream			// file class
{
public:
    QFile();
    QFile( const char *fileName );
   ~QFile();

    static bool exists( const char *fileName ); // test if file exists

    bool  file( const char *fileName );		// set file name
    bool  remove( const char *fileName=0 );	// remove file

    bool  open( int );				// open file
    bool  open( int, FILE * );			// open file, using file handle
    bool  close();				// close file
    bool  flush();				// flush file

    long  size();				// get file size
    long  at();					// get file pointer
    bool  at( long );				// set file pointer

    QStream& _read( char *p, uint len );	// read data from file
    QStream& _write( const char *p, uint len ); // write data to file

    int	  getch();				// get next char
    int	  putch( int );				// put char
    int	  ungetch( int ) ;			// put back char

protected:
    QString  fn;				// file name
    FILE    *fh;				// file handle
    long     length;				// file length
};


#endif // QFILE_H
