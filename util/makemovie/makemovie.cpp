/****************************************************************************
** $Id: //depot/qt/main/util/makemovie/makemovie.cpp#1 $
**
** C++ file
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qimage.h>
#include <qdstream.h>
#include <qlabel.h>
#include <qpen.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qapp.h>
#include <qmovie.h>
#include <qfile.h>
#include <qstrlist.h>

#include "QwCluster.h"

#include <stdio.h>
#include <stdlib.h>

QwClusterizer clusterizer( 8 );

QList<QMovieFrame> optimize( QStrList& files, unsigned int count )
{
  QImage img1;
  QImage img2;

  QList<QMovieFrame> frames;
  if ( count == 0 )
    return frames;

  // The first image is the first frame
  {
    if (!img2.load( files.first() ))
    {
      fprintf( stderr, "Error reading image from %s\n", files.first() );
      exit(1);
    }
    img2 = img2.convertDepth(32,ColorOnly);

    QPixmap pix;
    pix.convertFromImage( img2, ColorOnly );
    QMovieFrame *f = new QMovieFrame( pix, 0, 0, 0 );
    frames.append( f );
    if ( count < 2 )
      return frames;
  }

  const char* nextfile = files.first();
  for( unsigned int i = 0; i < count - 1; i++ )
  {
    img1 = img2;
    // Load the next image
    nextfile = files.next();
    if (!img2.load( nextfile ))
    {
      fprintf( stderr, "Error reading image from %s\n", nextfile );
      exit(1);
    }
    img2 = img2.convertDepth(32,ColorOnly);

    clusterizer.clear();
    ASSERT( img1.depth() == 32 );
    ASSERT( img2.depth() == 32 );

#ifdef DEMO_MODE
    // Convert the first image
    QPixmap pix;
    pix.convertFromImage( img1, ColorOnly );

    // We want to paint the difference red
    QPainter painter;
    painter.begin( &pix );
    QPen pen;
    pen.setColor( Qt::red );
    painter.setPen( pen );
#endif

    // Almost random walk thru the lines
    int h = 0;
    do {
      uchar *uline = img1.scanLine(h);
      QRgb* line = (QRgb*)uline;
      uchar *uline2 = img2.scanLine(h);
      QRgb* line2 = (QRgb*)uline2;
      // Pseudo random walk along a line.
      int w = 0;
      do {
	for( int x = 0; x < 7; x++ )
	{
	  if ( line[w*8+x] != line2[w*8+x] )
	  {
#ifdef DEMO_MODE
	    painter.drawLine( w*8, h, w*8 + 7, h );
#endif
	    clusterizer.add( QRect( w*8, h, 8, 1 ) );
	    goto ende;
	  }
	}
      ende:
	w = (w + 293) % 80;
      } while( w );
      h = (h + 199) % 480;
    } while( h );

    printf("------------ Image %i clusters=%i-------------\n",i,clusterizer.clusters());
#ifdef DEMO_MODE
    // Paint the blue rectangles
    pen.setColor( Qt::blue );
    painter.setPen( pen );
    for( int c = 0; c < clusterizer.clusters(); c++ )
    {
      QRect rect = clusterizer[c];
      // printf("Rect (%i|%i) (%i|%i)\n",rect.left(),rect.top(),rect.right(),rect.bottom());
      painter.drawRect( rect );
    }
    painter.end();
#endif

    /*QLabel* label = new QLabel( 0L );
    label->setPixmap( pix );
    label->show(); */

    // Convert the new image to create the difference frames.
    QPixmap pix2;
    pix2.convertFromImage( img2, ColorOnly );

    for( int c = 0; c < clusterizer.clusters(); c++ )
    {
      QRect rect = clusterizer[c];

      QPixmap dpix;
      dpix.resize( rect.width(), rect.height() );
      bitBlt( &dpix, QPoint(0, 0), &pix2, rect, Qt::CopyROP );
      QMovieFrame* f = new QMovieFrame( dpix, rect.left(), rect.top(),
					( c == 0 ? 200 : 0 ) );
      frames.append( f );
    }

    printf("Done one\n");
  }

  printf("Done all\n");
  return frames;
}

main(int argc, char** argv)
{
    QApplication app( argc, argv );

    unsigned int count = (unsigned int)argc - 1;
    QStrList files;
    for ( unsigned int i=0; i<count; i++)
      files.append( argv[i+1] );

    QList<QMovieFrame> frames = optimize( files, count );

    QFile file( "movie" );
    file.open( IO_WriteOnly );
    {
      QDataStream str( &file );
      str << frames;
    }
    file.close();

    QMovie mov( frames );
    QLabel* l = new QLabel( 0L );
    l->setMovie( mov );
    l->show();

    app.exec();

    return 0;
}
