/****************************************************************************
** $Id: //depot/qt/main/util/makemovie/makemovie.cpp#2 $
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
#include <qsortedlist.h>
#include <qlist.h>

#include "QwCluster.h"

#include <stdio.h>
#include <stdlib.h>

#define FRAME_RATE 60
bool demo_mode = false;

QwClusterizer clusterizer( 16 );

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
    img2 = img2.convertDepth(32,Qt::ColorOnly);

    QPixmap pix;
    pix.convertFromImage( img2, Qt::ColorOnly | Qt::AvoidDither );
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
    img2 = img2.convertDepth(32,Qt::ColorOnly);

    clusterizer.clear();
    ASSERT( img1.depth() == 32 );
    ASSERT( img2.depth() == 32 );

    // Variables used for demo mode
    QPainter painter;
    QPen pen;
    QPixmap pix;
    if ( demo_mode )
    {
      // Convert the first image
      pix.convertFromImage( img1, Qt::ColorOnly | Qt::AvoidDither );

      // We want to paint the difference red
      painter.begin( &pix );
      pen.setColor( Qt::green );
      painter.setPen( pen );
    }

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
	for( int x = 0; x < 8; x++ )
	{
	  if ( line[w*8+x] != line2[w*8+x] )
	  {
	    if ( demo_mode )
	      painter.drawLine( w*8, h, w*8 + 7, h );

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
    if ( demo_mode )
    {
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
      QLabel* label2 = new QLabel( 0 );
      label2->setPixmap( pix );
      label2->show();
    }

    // Convert the new image to create the difference frames.
    QPixmap pix2;
    pix2.convertFromImage( img2, Qt::ColorOnly | Qt::AvoidDither );

    // Test wether it is not a better idea to merge all rectangles.
    QRect maxrect = clusterizer[0];
    int area = 0;
    for( int c = 0; c < clusterizer.clusters(); c++ )
    {
      QRect r = clusterizer[c];
      area += r.width() * r.height();
      maxrect = maxrect.unite( r );
    }
    // How much do we loose if we take just a simple frame ?
    if ( (float)area * 1.20 >= (float)( maxrect.width() * maxrect.height() ) )
    {
      QPixmap dpix;
      dpix.resize( maxrect.width(), maxrect.height() );
      bitBlt( &dpix, QPoint( 0, 0 ), &pix2, maxrect, Qt::CopyROP );
      QMovieFrame* f = new QMovieFrame( dpix, maxrect.left(), maxrect.top(), FRAME_RATE );
      frames.append( f );
      printf("....... No optimization possible .......Factor of saving was %f percent \n",
	     ((float)(maxrect.width() * maxrect.height())) / (float)area * 100.0 - 100.0 );
    }
    else
    {
      printf(".... The optimization is %f percent\n",
	     ((float)(maxrect.width() * maxrect.height())) / (float)area * 100.0 - 100.0 );
      // Sort the frames by their y-coordinate. This will
      // lead to a more smooth update.
      QSortedList<QMovieFrame> list;
      for( int c = 0; c < clusterizer.clusters(); c++ )
      {
	QRect rect = clusterizer[c];
	
	QPixmap dpix;
	dpix.resize( rect.width(), rect.height() );
	bitBlt( &dpix, QPoint(0, 0), &pix2, rect, Qt::CopyROP );
	QMovieFrame* f = new QMovieFrame( dpix, rect.left(), rect.top(), 0 );
	list.inSort( f );
      }
      if ( list.first() )
	list.first()->set( list.first()->pixmap(), list.first()->xOffset(),
			   list.first()->yOffset(), FRAME_RATE );
      // Copy the sorted frames to the movie
      QMovieFrame* f;
      for( f = list.first(); f != 0; f = list.next() )
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

    if ( argc == 1 )
    {
      printf("moviemaker: Syntax:\n");
      printf("   [-demo] files\n");
      printf("   -load qtmovie\n");
      exit(0);
    }

    if ( argc >= 2 && strcmp( argv[1], "-demo" ) == 0 )
      demo_mode = true;

    QList<QMovieFrame> frames;

    bool load_mode = false;
    if ( argc >= 2 && strcmp( argv[1], "-load" ) == 0 )
    {
      load_mode = true;
      if ( argc != 3 )
      {
	printf("moviemaker: Syntax:\n");
	printf("   [-demo] files\n");
	printf("   -load qtmovie\n");
	exit(0);
      }

      QFile file( argv[2] );
      if ( !file.open( IO_ReadOnly ) )
      {
	printf("Could not open %s\n", argv[1] );
	exit(0);
      }

      {
	QDataStream str( &file );
	str >> frames;
      }
      file.close();

      printf("Loaded %i QMovieFrames\n", frames.count() );
    }
    else
    {
      int offset = ( demo_mode ? 1 : 0 );
      unsigned int count = (unsigned int)argc - 1 - offset;
      QStrList files;
      for ( unsigned int i=0; i<count; i++)
	files.append( argv[i+1+offset] );

      frames = optimize( files, count );
    }

    if (!demo_mode)
    {
      if ( !load_mode )
      {
	QFile file( "movie.out" );
	file.open( IO_WriteOnly );
        {
	  QDataStream str( &file );
	  str << frames;
	}
	file.close();
      }

      QMovie mov( frames );
      QLabel* l = new QLabel( 0L );
      l->setMovie( mov );
      l->setGeometry( 0,0,640,480);
      l->show();
    }

    app.exec();

    return 0;
}
