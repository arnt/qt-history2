/****************************************************************************
** $Id: //depot/qt/main/util/gifanim/gifanim.cpp#1 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

/*****************************************************************************
  The Graphical Interchange Format (c) is the Copyright property of
  CompuServe Incorporated.  GIF (sm) is a Service Mark propery of
  CompuServe Incorporated.
 *****************************************************************************/

#include <qimage.h>
#include <qfile.h>
#include <qdstream.h>

#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/util/gifanim/gifanim.cpp#1 $");

/*
	QImageIO::defineIOHandler( "GIF", "^GIF[0-9][0-9][a-z]", 0,
				   read_gif_image, write_gif_image );
*/



/*
Block Name                  Required   Label       Ext.   Vers.
Application Extension       Opt. (*)   0xFF (255)  yes    89a
Comment Extension           Opt. (*)   0xFE (254)  yes    89a
Global Color Table          Opt. (1)   none        no     87a
Graphic Control Extension   Opt. (*)   0xF9 (249)  yes    89a
Header                      Req. (1)   none        no     N/A
Image Descriptor            Opt. (*)   0x2C (044)  no     87a (89a)
Local Color Table           Opt. (*)   none        no     87a
Logical Screen Descriptor   Req. (1)   none        no     87a (89a)
Plain Text Extension        Opt. (*)   0x01 (001)  yes    89a
Trailer                     Req. (1)   0x3B (059)  no     87a

Unlabeled Blocks
Header                      Req. (1)   none        no     N/A
Logical Screen Descriptor   Req. (1)   none        no     87a (89a)
Global Color Table          Opt. (1)   none        no     87a
Local Color Table           Opt. (*)   none        no     87a

Graphic-Rendering Blocks
Plain Text Extension        Opt. (*)   0x01 (001)  yes    89a
Image Descriptor            Opt. (*)   0x2C (044)  no     87a (89a)

Control Blocks
Graphic Control Extension   Opt. (*)   0xF9 (249)  yes    89a

Special Purpose Blocks
Trailer                     Req. (1)   0x3B (059)  no     87a
Comment Extension           Opt. (*)   0xFE (254)  yes    89a
Application Extension       Opt. (*)   0xFF (255)  yes    89a
*/


static const int trans = 216;
static inline int c216( QRgb rgb )
{
    return (qRed(rgb) * 5 / 255)
	  + (qGreen(rgb) * 5 / 255) * 6
	  + (qBlue(rgb) * 5 / 255) * 36;
}

static inline int gif_get_pixel( const QImage& image, const QImage* previmage,
				 int& x, int& y, const QRect& sub )
{
    static uchar * p;
    static uchar * pp;
    int color;

    if ( x == 0 ) {
	p = image.scanLine( sub.y() + y ) + sub.x();
    }
    color = c216(image.color(*p));

    if ( previmage ) {
	if ( x == 0 ) {
	    pp = previmage->scanLine( sub.y() + y ) + sub.x();
	}
	int pcolor = c216(previmage->color(*pp));
	if ( pcolor == color )
	    color = trans;
    }

    p++;
    pp++;
    x++;

    return color;
}


static void gif_write_content( Q_UINT8 packet[], int& packetPointer,
			       Q_UINT32 & accumulator,
			       unsigned int & shift,
			       QDataStream * s )
{
    while ( shift > 7 ) {
	packet[packetPointer++] = (accumulator & 255);
	accumulator = accumulator >> 8;
	shift -= 8;
	if ( packetPointer == 256 ) {
	    s->device()->writeBlock( (char *)packet, 256 );
	    packetPointer = 1;
	}
    }
}


bool write_gif_animation( QIODevice* iod, QImage *images, unsigned int count )
{
    // Best with 24-bit images, as we insist on 216 colour cube,
    // and convertDepth() will not dither from 8 bit currently.

    // We always have a transparent index, because it can help a lot
    // when compressing multiple images.

    int w = images[0].width(), h = images[0].height();
    int x, y;

    const int delay_time = 30;
    const int dithermode = PreferDither|ThresholdDither;
    //const int dithermode = PreferDither|OrderedDither;
    const int loop_count = 1;  // Infinite

    unsigned int i;
    int palette[217];
    int ncols = 0;
    for( i=0; i<217; i++ )
	palette[i] = 0;

    for (i=0; i<count && ncols < 216; i++) {
	QImage image = images[i].convertDepth( 8, dithermode );
	uchar** jt8 = (uchar**)image.jumpTable();
	QRgb* cmap = image.colorTable();

	for( y=0; y<h && ncols < 216; y++ ) {
	    for( x=0; x<w; x++ ) {
		QRgb color = cmap[jt8[y][x]];
		int color216 = c216(color);
		if ( !palette[color216] ) {
		    palette[color216] = 1;
		    ncols++;
		}
	    }
	}
    }

    ncols++; // for transparency

    int colorTableSize = 2;
    int colorTableCode = 1;
    while ( colorTableSize < ncols ) {
	colorTableSize *= 2;
	colorTableCode++;
    }

    // write the file header
    QDataStream * s = new QDataStream( iod );
    if ( s->device()->writeBlock( "GIF89a", 6 ) < 6 )
	return FALSE;

    s->setByteOrder( QDataStream::LittleEndian );
    *s << (Q_UINT16)w
       << (Q_UINT16)h
       << (Q_UINT8) (128 + 17*(colorTableCode-1))
       << (Q_UINT8) 0 // some random color is "background"
       << (Q_UINT8) 0; // no aspect ratio information

    // build and write the global color table
    char * globalColorTable = new char[3*colorTableSize];

    int p = 0;

    palette[trans] = p/3;
    globalColorTable[p++] = 0xcc;
    globalColorTable[p++] = 0xcc;
    globalColorTable[p++] = 0xcc;

    for( i=0; i<216; i++ )
	if ( palette[i] ) {
	    palette[i] = p/3;
	    globalColorTable[p++] = (unsigned char)( (i%6) * 0x33 );
	    globalColorTable[p++] = (unsigned char)( ( (i/6)%6 ) * 0x33 );
	    globalColorTable[p++] = (unsigned char)( ( i/36 ) * 0x33 );
	}

    // Be tidy - fill the rest with 0
    memset( globalColorTable + p, 0, 3*colorTableSize - p );

    if ( s->device()->writeBlock( globalColorTable, 3*colorTableSize ) <
	 3*colorTableSize )
	return FALSE;

    delete[]globalColorTable;
    globalColorTable = 0;

    // Add looping extension
    *s << '!' // extension label
       << (Q_UINT8) 0xff // application extension
       << (Q_UINT8) 11 // size
       << 'N' << 'E' << 'T' << 'S' << 'C' << 'A' << 'P' << 'E'
       << '2' << '.' << '0'
       << (Q_UINT8) 3 // size
       << (Q_UINT16) loop_count
       << (Q_UINT8) 0
       << (Q_UINT8) 0
       ;

    QImage* previmage = 0;

    for (unsigned int im=0; im<count; im++) {
	QImage image = images[im].convertDepth( 8, dithermode );

	// write transparent color index
	*s << (Q_UINT8) 0x21 // extension block
	   << (Q_UINT8) 0xf9 // graphic control extension
	   << (Q_UINT8) 4 // size
	   << (Q_UINT8) 1 // no not dispose, transparent color follows
	   << (Q_UINT16) delay_time
	   //<< (Q_UINT16) (im == 0 ? 4000 : delay_time)
	   << (Q_UINT8) palette[trans]
	   << (Q_UINT8) 0; // terminate the extension block

	QRect sub;

	if ( previmage ) {
	    // Compare with previous image to find smallest changed rectangle
	    int minx, maxx, miny, maxy;

	    uchar** jt = image.jumpTable();
	    uchar** pjt = previmage->jumpTable();

	    bool done;

	    // Find left edge of change
	    done = FALSE;
	    for (minx = 0; minx < w && !done; minx++) {
		for (int ty = 0; ty < h; ty++) {
		    if ( jt[ty][minx] != pjt[ty][minx] ) {
			done = TRUE;
			break;
		    }
		}
	    }
	    minx--;

	    // Find right edge of change
	    done = FALSE;
	    for (maxx = w-1; maxx >= 0 && !done; maxx--) {
		for (int ty = 0; ty < h; ty++) {
		    if ( jt[ty][maxx] != pjt[ty][maxx] ) {
			done = TRUE;
			break;
		    }
		}
	    }
	    maxx++;

	    // Find top edge of change
	    done = FALSE;
	    for (miny = 0; miny < h && !done; miny++) {
		for (int tx = 0; tx < w; tx++) {
		    if ( jt[miny][tx] != pjt[miny][tx] ) {
			done = TRUE;
			break;
		    }
		}
	    }
	    miny--;

	    // Find right edge of change
	    done = FALSE;
	    for (maxy = h-1; maxy >= 0 && !done; maxy--) {
		for (int tx = 0; tx < w; tx++) {
		    if ( jt[maxy][tx] != pjt[maxy][tx] ) {
			done = TRUE;
			break;
		    }
		}
	    }
	    maxy++;

	    if ( minx > maxx ) minx=maxx=0;
	    if ( miny > maxy ) miny=maxy=0;

	    sub.setCoords(minx, miny, maxx, maxy);
	} else {
	    // First image - the whole frame
	    sub.setRect(0,0,w,h);
	}

	*s << (Q_UINT8) 0x2C // image separator, constant
	   << (Q_UINT16) sub.x()
	   << (Q_UINT16) sub.y()
	   << (Q_UINT16) sub.width()
	   << (Q_UINT16) sub.height()
	   << (Q_UINT8) 0; // non-interlaces, no local cmap

	// write pixels

	unsigned int codeSize = colorTableCode;
	if ( codeSize < 2 )
	    codeSize = 2;

	*s << (Q_UINT8) codeSize;

	// stuff for the compressor...

	struct {
	    unsigned int child : 12;
	    unsigned int sibling : 12;
	    unsigned int pixel : 8;
	} codeTable[4096];

	Q_UINT8 packet[257]; // data sub-block and one byte for a hack below
	int packetPointer = 1;
	packet[0] = (Q_UINT8) 255;

	unsigned int clear = 1 << codeSize++; // gif says
	unsigned int endOfInformation = clear+1; // gif says
	unsigned int ncodes = clear+1; // highest-numbered code in use - mine

	unsigned int entry = clear; // clear, here, means Do Not Add
	codeTable[clear].child = clear;

	for( i=0; i<clear; i++ ) {
	    codeTable[i].child = clear; // meaning: invalid
	    codeTable[i].pixel = i;
	}

	UINT32 accumulator = 0;
	unsigned int shift = 0;

	x = 0;
	y = 0;
	unsigned int pixel, child;

	while( y < sub.height() ) {
	    // output to make space, if necessary
	    if ( shift + codeSize > 30 )
		gif_write_content( packet, packetPointer, accumulator, shift, s );

	    pixel = palette[gif_get_pixel( image, previmage, x, y, sub )];
	    if ( x >= sub.width() ) {
		y++;
		x = 0;
	    }

	    // see whether entry+child exists in the dictionary
	    child = codeTable[entry].child;
	    while ( child != clear && codeTable[child].pixel != pixel )
		child = codeTable[child].sibling;
	    if ( child != clear ) {
		// yes, use that entry
		entry = child;
	    } else if ( ncodes < 4095 && entry != clear ) {
		// no, and entry is not 'clear', and there is space to add
		// entry+child to the dictionary
		child = ++ncodes;
		codeTable[child].child = clear;
		codeTable[child].pixel = pixel;
		codeTable[child].sibling = codeTable[entry].child;
		codeTable[entry].child = child;
		accumulator = accumulator | ( entry << shift );
		shift += codeSize;
		// may need to increase the code size
		if ( codeSize < 12 && ncodes >= (unsigned int)(1 << codeSize ) )
		    codeSize++;
		entry = pixel;
	    } else {
		// dictionary is full, so send the entry and then a clear
		// (unless the entry is the initial clear)
		accumulator = accumulator | ( entry << shift );
		shift += codeSize;
		// may need to make space in the accumulator
		if ( shift + codeSize > 30 )
		    gif_write_content( packet, packetPointer,
				       accumulator, shift, s );
		if ( entry != clear ) {
		    for( i=0; i<clear; i++ ) {
			codeTable[i].child = clear; // meaning: invalid
			codeTable[i].pixel = i;
		    }
		    accumulator = accumulator | ( clear << shift );
		    shift += codeSize;
		}
		ncodes = clear+1;
		codeSize = colorTableCode+1;
		if ( codeSize < 2 )
		    codeSize = 2;
		entry = pixel;
	    }
	}

	// write the last entry
	if ( shift + codeSize > 30 )
	    gif_write_content( packet, packetPointer, accumulator, shift, s );
	accumulator = accumulator | ( entry << shift );
	shift += codeSize;

	// add the end of information marker
	if ( shift + codeSize > 30 )
	    gif_write_content( packet, packetPointer, accumulator, shift, s );
	accumulator = accumulator | ( endOfInformation << shift );

	shift = shift + codeSize + 7; // to make sure the final byte is sent off
	gif_write_content( packet, packetPointer, accumulator, shift, s );

	// add the zero-length sub-block
	packet[0] = (Q_UINT8)(packetPointer-1);
	if ( packetPointer > 1 )
	    packet[packetPointer++] = (Q_UINT8) 0; // zero length sub-block
	if ( s->device()->writeBlock( (char *)packet, packetPointer )
				< packetPointer )
	{
	    return FALSE; // assumption: if this succeeds, all earlier writes succeeded
	}
	packetPointer = 0;

	delete previmage;
	previmage = new QImage(image);
    }
    Q_UINT8 byte = 0x3B; // gif trailer
    if ( s->device()->writeBlock( (char *)&byte, 1 ) < 1 )
	return FALSE; // assumption: if this succeeds, all earlier writes succeeded

    // Done, and successful.
    return TRUE;
}




main(int argc, char** argv)
{
    int count = argc - 1;
    QImage image[count];
    for (int i=0; i<count; i++)
	if (!image[i].load( argv[i+1] )) {
	    fprintf( stderr, "Error reading image from %s\n", argv[i+1] );
	    exit(1);
	}
    QFile out("gif.out");
    out.open( IO_WriteOnly );
    if ( !write_gif_animation( &out, image, count ) ) {
	fprintf( stderr, "Error writing image\n" );
	exit(1);
    }
    return 0;
}
