/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#3 $
**
** Implementation of QPixmap class
**
** Author  : Haavard Nord
** Created : 950301
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qcache.h"
#include "qregexp.h"
#include "qfile.h"
#include "qdstream.h"
#include "qlist.h"
#include "qintdict.h"
#include <stdlib.h>
#include <ctype.h>


#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#3 $";
#endif

// --------------------------------------------------------------------------
// QPixmap member functions
//

#undef MIN
#define MIN(x,y)  ((x)<(y) ? (x) : (y))

void QPixmap::resize( int w, int h )
{
    if ( !data->virgin ) {				// has existing pixmap
	QPixmap pm( w, h, depth() );
	bitBlt( &pm, 0, 0, this, 0, 0,	// copy old pixmap
		MIN(width(), w),
		MIN(height(),h) );
	*this = pm;
    }
    else					// create new pixmap
	*this = QPixmap( w, h, isBitMap() ? 1 : -1 );
}

const char *QPixmap::imageType( const char *fileName )
{						// determine image format
    return QImageIO::imageFormat(fileName);
}

bool QPixmap::load( const char *fileName, const char *format )
{						// load image
    QImageIO io;
    io.fileName = fileName;
    io.format   = format;
    if ( io.read() ) {
	setImageData( &io );
	return TRUE;
    }
    return FALSE;
}

bool QPixmap::save( const char *fileName, const char *format ) const
{						// save image
    if ( isNull() )
	return FALSE;				// nothing to save
    QImageIO io;
    io.fileName = fileName;
    io.format   = format;
    getImageData( &io );
    return io.write();
}

bool QPixmap::isBitMap() const			// reimplemented in QBitmap
{
    return FALSE;
}


// --------------------------------------------------------------------------
// pixmap cache declarations and functions
//

typedef declare(QCacheM,QPixmap) QPixmapCache;
static QPixmapCache *pmcache = 0;		// global pixmap cache
const long pmcache_maxcost   = 1024*1024;	// maximum cache cost
const int  pmcache_size      = 61;		// size of internal hash array


QPixmap *QPixmap::find( const char *key )	// find pixmap in cache
{
    return pmcache ? pmcache->find(key) : 0;
}

bool QPixmap::insert( const char *key, QPixmap *pm )
{
    if ( !pmcache ) {				// create pixmap cache
	pmcache = new QPixmapCache( pmcache_maxcost, pmcache_size );
	pmcache->setAutoDelete( TRUE );
	CHECK_PTR( pmcache );
    }
    return pmcache->insert( key, pm, pm->width()*pm->height()*pm->depth()/8 );
}

void QPixmap::setCacheSize( long maxCost )	// change the maxcost parameter
{
    if ( pmcache )
	pmcache->setMaxCost( maxCost );
    else {
	pmcache = new QPixmapCache( maxCost, pmcache_size );
	pmcache->setAutoDelete( TRUE );
	CHECK_PTR( pmcache );
    }
}

void QPixmap::cleanup()				// cleanup cache
{
    delete pmcache;
}

// --------------------------------------------------------------------------
// standard image io handlers (defined below)
//

static void read_qt_image( QImageIO * );
static void write_qt_image( QImageIO * );
static void read_gif_image( QImageIO * );
static void write_gif_image( QImageIO * );
static void read_bmp_image( QImageIO * );
static void write_bmp_image( QImageIO * );
static void read_pbm_image( QImageIO * );
static void write_pbm_image( QImageIO * );
static void read_xbm_image( QImageIO * );
static void write_xbm_image( QImageIO * );
static void read_xpm_image( QImageIO * );
static void write_xpm_image( QImageIO * );


// --------------------------------------------------------------------------
// Misc. utility functions
//

static QString fbname( const char *fileName )	// get file basename (sort of)
{
    QString s = fileName;
    if ( !s.isEmpty() ) {
	int i;
	if ( (i=s.findRev('/')) >= 0 )
	    s = &s[i];
	if ( (i=s.findRev('\\')) >= 0 )
	    s = &s[i];
	QRegExp r( "[a-zA-Z][a-zA-Z0-9_]*" );
	int p = r.match( s, 0, &i );
	if ( p >= 0 )
	    s = &s[p];
	s.truncate(i);
    }
    if ( s.isEmpty() )
	s = "dummy";
    return s;
}


// --------------------------------------------------------------------------
// Image handler functions
//

struct QImageHandler {
    QString	      format;			// image format
    QString	      header;			// image header pattern
    bool	      text_mode;		// image I/O mode
    image_io_handler  read_image;		// image read function
    image_io_handler  write_image;		// image write function
};

typedef declare(QListM,QImageHandler) QIHList;	// list of image handlers
static QIHList *imageHandlers = 0;

static void cleanup_image_handlers()		// cleanup image handler list
{
    delete imageHandlers;
    imageHandlers = 0;
}

static void init_image_handlers()		// initialize image handlers
{
    if ( imageHandlers == 0 ) {
	imageHandlers = new QIHList;
	CHECK_PTR( imageHandlers );
	imageHandlers->setAutoDelete( TRUE );
	qAddPostRoutine( cleanup_image_handlers );
	QPixmap::defineIOHandler( "QT", "^QIMG", 0,
				 read_qt_image, write_qt_image );
//	QPixmap::defineIOHandler( "GIF", "^GIF[0-9][0-9][a-z]", 0,
//				 read_gif_image, write_gif_image );
	QPixmap::defineIOHandler( "BMP", "^BM", 0,
				 read_bmp_image, write_bmp_image );
	QPixmap::defineIOHandler( "PBM", "^P1", "T",
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PBMRAW", "^P4", 0,
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PGM", "^P2", "T",
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PGMRAW", "^P5", 0,
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PPM", "^P3", "T",
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PPMRAW", "^P6", 0,
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "PNM", "^P1", 0,
				 read_pbm_image, write_pbm_image );
	QPixmap::defineIOHandler( "XBM", "^#define", "T",
				 read_xbm_image, write_xbm_image );
//	QPixmap::defineIOHandler( "XPM", "/\\*.XPM.\\*/", "T",
//				 read_xpm_image, write_xpm_image );
    }
}

static QImageHandler *get_image_handler( const char *format )
{						// get pointer to handler
    if ( imageHandlers == 0 )
	init_image_handlers();
    register QImageHandler *p = imageHandlers->first();
    while ( p ) {				// traverse list
	if ( p->format == format )
	    return p;
	p = imageHandlers->next();
    }
    return 0;					// no such handler
}

void QPixmap::defineIOHandler( const char *format,
			      const char *header,
			      const char *flags,
			      image_io_handler read_image,
			      image_io_handler write_image )
{
    if ( imageHandlers == 0 )
	init_image_handlers();
    register QImageHandler *p = new QImageHandler;
    CHECK_PTR( p );
    p->format = format;
    p->header = header;
    p->text_mode = flags && *flags == 'T';
    p->read_image = read_image;
    p->write_image = write_image;
    imageHandlers->insert( p );
}


// --------------------------------------------------------------------------
// QImageData and QImageIO functions
//

static bool bitflip_init = FALSE;
static char bitflip[256];			// table to flip bits

static void setup_bitflip()			// create bitflip table
{
    if ( !bitflip_init ) {
	for ( int i=0; i<256; i++ )
	    bitflip[i] = ((i >> 7) & 0x01) | ((i >> 5) & 0x02) |
			 ((i >> 3) & 0x04) | ((i >> 1) & 0x08) |
			 ((i << 7) & 0x80) | ((i << 5) & 0x40) |
			 ((i << 3) & 0x20) | ((i << 1) & 0x10);
	bitflip_init = TRUE;
    }
}

char *qt_get_bitflip_array()			// called from QPixmap code
{
    setup_bitflip();
    return bitflip;
}


QImageData::QImageData()
{
    width = height = depth = ncols = 0;
    ctbl  = 0;
    bits  = 0;
    bitOrder = IgnoreEndian;
}

QImageData::~QImageData()
{
    if ( ctbl )
	delete ctbl;
    if ( bits )
	freeBits();
}


void QImageData::clear()			// dealloc all data
{
    freeBits();
    delete ctbl;
    ctbl = 0;
    width = height = depth = ncols = 0;
    bitOrder = IgnoreEndian;
}


int QImageData::systemByteOrder()		// determine system byte order
{
    static int sbo = IgnoreEndian;
    if ( sbo == IgnoreEndian ) {		// initialize
	int ws, be;
	qSysInfo( &ws, &be );
	sbo = be ? BigEndian : LittleEndian;
    }
    return sbo;
}


#if defined(_WS_X11_)
#define	 GC GC_QQQ
#include <X11/Xlib.h>				// needed for systemBitOrder
#include <X11/Xutil.h>
#include <X11/Xos.h>
#endif

int QImageData::systemBitOrder()		// determine hardware bit order
{
#if defined(_WS_X11_)
    return BitmapBitOrder(qt_xdisplay()) == MSBFirst ? BigEndian :LittleEndian;
#else
    return BigEndian;
#endif
}



long QImageData::numBytes() const		// number of bytes image data
{
    long n;
    switch ( depth ) {
	case 1:
	    n = (width+7)/8*height;
	    break;
        case 8:
	    n = width*height;
	    break;
	case 24:
	    n = width*height*3;
	    break;
	default:
	    n = 0;
    }
    return n;
}


//
// On 32-bit systems, the image data is always allocated as one block
// (contigous data).
// On Windows 3.1 (16 bit) the image data is allocated in smaller chunks,
// (one block per scanline) when the image data occupies more than 64k.
// The image data structure ('bits' member of QImageData) consists of a
// table of pointers to each scanline.
// We also store the image height just before the image data.
//

void QImageData::allocBits()			// allocate the image bits
{
    if ( bits )					// deallocate old data
	freeBits();
    if ( width <= 0 || height <= 0 )		// invalid width or height
	return;
    long nbytes = numBytes();			// bytes total image bits
    long bpl    = nbytes/height;		// bytes per line
    long ptrtbl = height*sizeof(uchar*);	// size of pointer table
    long totsize= nbytes + ptrtbl + sizeof(int);// size of data block
    uchar *p = 0;
#if defined(_WS_WIN16_)
    if ( totsize < 64000L )			// try to alloc just one block
	p = (uchar *)calloc( (uint)totsize, 1 );
    if ( !p ) {					// one block per scanline
	p = (uchar *)malloc( ptrtbl + sizeof(int) );
	CHECK_PTR( p );
	*((int*)p) = height | 0x8000;
	bits = (uchar **)(p + sizeof(int));
	for ( int i=0; i<height; i++ ) {
	    bits[i] = (uchar *)calloc( bpl, 1 );
	    CHECK_PTR( bits[i] );
	}
	return;
    }
#else
    p = (uchar *)calloc( totsize, 1 );		// alloc a single block
    CHECK_PTR( p );
#endif
    *((int *)p) = height;			// encode height as first int
    p += sizeof(int);
    bits = (uchar**)p;				// set image pointer
    uchar *d = p + ptrtbl;			// start of image data
    for ( int i=0; i<height; i++ ) {
	bits[i] = d;
	d += bpl;
    }
}

void QImageData::freeBits()
{
    if ( bits ) {
	int *p = (int *)bits;
	p--;					// real start of pointer
#if defined(_WS_WIN16_)
	if ( (*p & 0x8000) == 0x8000 ) {	// one block per scanline
	    int h = *p & 0x7fff;
	    for ( int i=0; i<h; i++ )
		free( bits[i] );
	}
#endif
	free( p );
	bits = 0;
    }
}


bool QImageData::contiguousBits() const
{
#if defined(_WS_WIN16_)
    int *p = (int*)bits;
    return p && (*(p-1) & 0x8000) == 0;
#else
    return TRUE;
#endif
}


//
// convert_24_to_8:  Converts a 24 bits depth (true color) to an 8 bit
// image with a colormap.  If the 24 bit image has more than 256 colors,
// we convert the red,green and blue bytes into a single byte encoded
// as RRRGGGBB.
//

declare(QIntDictM,char);
declare(QIntDictIteratorM,char);

static bool convert_24_to_8( const QImageData *src, QImageData *dst )
{
    register uchar *p;
    int	    ncols;
    ulong  *c;
    uchar  *b, *end;
    bool    do_quant = FALSE;

    dst->width  = src->width;			// set dst image params
    dst->height = src->height;
    dst->depth  = 8;
    dst->allocBits();				// allocate data
    if ( !dst->bits )				// could not allocate data
	return FALSE;

    QIntDictM(char) cdict( 2111 );		// dict of 24-bit RGB colors
    p = src->bits[0]-1;
    b = dst->bits[0];
    end = p + src->numBytes();
    char *pixel=0, *pix;			// hack: use ptr as int value
    while ( p < end ) {				// check if <= 256 colors
	ulong rgb = (uchar)*++p + ((ushort)*++p << 8) + ((ulong)*++p <<16);
	if ( !(pix=cdict.find(rgb)) ) {		// new RGB color?
	    cdict.insert( rgb, (pix=++pixel) );	// yes -> keep it
	    if ( cdict.count() > 256 ) {	// oops: too many colors
		do_quant = TRUE;
		break;
	    }
	}
	*b++ = (int)pix - 1;			// map RGB color to pixel
    }
    ncols = do_quant ? 256 : cdict.count();
    c = new ulong[ncols];			// allocate color table
    if ( !c )
	return FALSE;
    if ( do_quant ) {				// quantization needed
	for ( int i=0; i<ncols; i++ )		// build 3+3+2 color table
	    c[i] = QRGB( ((i & 0xe0)*255 + 0x70) / 0xe0,
			 (((i << 3) & 0xe0)*255 + 0x70) / 0xe0,
			 (((i << 6) & 0xc0)*255 + 0x60) / 0xc0 );
	p = src->bits[0]-1;
	b = dst->bits[0];
	end = p + src->numBytes();
	while ( p < end )			// perform fast quantization
	    *b++ = (*++p & 0xe0) | ((*++p >> 3) & 0x1c) | ((*++p >> 6) & 0x03);
    }
    else {					// number of colors <= 256
	QIntDictIteratorM(char) cdictit( cdict );
	while ( cdictit.current() ) {		// build color table
	    c[(int)(cdictit.current())-1] = cdictit.currentKey();
	    ++cdictit;
	}
    }
    dst->ncols = ncols;
    dst->ctbl  = c;
    return TRUE;
}


static bool convert_8_to_24( const QImageData *src, QImageData *dst )
{
    dst->width  = src->width;			// set dst image params
    dst->height = src->height;
    dst->depth  = 24;
    dst->ncols  = 0;
    delete dst->ctbl;				// no color table needed
    dst->ctbl   = 0;
    dst->allocBits();				// allocate data
    if ( !dst->bits )				// could not allocate data
	return FALSE;
    for ( int y=0; y<dst->height; y++ ) {	// for each scan line...
	register uchar *p = dst->bits[y];
	uchar *b = src->bits[y];
	uchar *end = p + dst->width;
	while ( p < end ) {
	    *p++ = QRED  ( src->ctbl[*b] );
	    *p++ = QGREEN( src->ctbl[*b] );
	    *p++ = QBLUE ( src->ctbl[*b++] );
	}
    }
    return TRUE;
}


static bool convert_1_to_24( const QImageData *src, QImageData *dst )
{
    dst->width  = src->width;			// set dst image params
    dst->height = src->height;
    dst->depth  = 24;
    dst->ncols  = 0;
    delete dst->ctbl;				// no color table needed
    dst->ctbl   = 0;
    dst->allocBits();				// allocate data
    if ( !dst->bits )				// could not allocate data
	return FALSE;
    bool big = src->bitOrder == QImageData::BigEndian;
    for ( int y=0; y<dst->height; y++ ) {	// for each scan line...
	register uchar *p = dst->bits[y];
	uchar *b = src->bits[y];
	int v;
	for ( int x=0; x<dst->width; x++ ) {
	    v = (big ? *b >> (7 - (x & 7)) : *b >> (x & 7)) & 1;
	    *p++ = QRED  ( src->ctbl[v] );
	    *p++ = QGREEN( src->ctbl[v] );
	    *p++ = QBLUE ( src->ctbl[v] );
	    if ( x & 7 == 7 )
		b++;
	}
    }
    return TRUE;
}


static bool convert_1_to_8( const QImageData *src, QImageData *dst )
{
    dst->width  = src->width;			// set dst image params
    dst->height = src->height;
    dst->depth  = 8;
    dst->ncols  = 2;
    delete dst->ctbl;				// no color table needed
    dst->ctbl   = new ulong[2];
    dst->allocBits();				// allocate data
    if ( !(dst->ctbl && dst->bits) )		// could not allocate data
	return FALSE;
    dst->ctbl[0] = src->ctbl[0];		// copy color table
    dst->ctbl[1] = src->ctbl[1];
    bool big = src->bitOrder == QImageData::BigEndian;
    for ( int y=0; y<dst->height; y++ ) {	// for each scan line...
	register uchar *p = dst->bits[y];
	uchar *b = src->bits[y];
	for ( int x=0; x<dst->width; x++ ) {
	    *p++ = (big ? *b >> (7 - (x & 7)) : *b >> (x & 7)) & 1;
	    if ( x & 7 == 7 )
		b++;
	}
    }
    return TRUE;
}


//
// dither_image:  Uses the Floyd-Steinberg error diffusion algorithm.
// Floyd-Steinberg dithering is a one-pass algorithm that spreads errors
// to neighbor pixels.
//

static bool dither_image( const QImageData *src, QImageData *dst )
{
    int w = src->width, h = src->height;
    dst->width  = w;				// set dst image params
    dst->height = h;
    dst->depth  = 1;
    dst->ncols  = 2;
    delete dst->ctbl;				// delete old color table
    dst->ctbl   = new ulong[2];
    dst->allocBits();				// allocate image data
    if ( !(dst->ctbl && dst->bits) )		// could not allocate data
	return FALSE;
    dst->bitOrder = QImageData::BigEndian;
    dst->ctbl[0] = QRGB( 255, 255, 255 );
    dst->ctbl[1] = QRGB(   0,   0,   0 );
    uchar gray[256];				// gray map for 8 bit images
    bool  use_gray = src->depth == 8;
    if ( use_gray ) {				// make gray map
	for ( int i=0; i<src->ncols; i++ )
	    gray[i] = QGRAY( src->ctbl[i] );
    }
    int *line1 = new int[w];
    int *line2 = new int[w];
    uchar *bmline = new uchar[w];
    int bmwidth = (w+7)/8;
    if ( !(line1 && line2 && bmline) )
	return FALSE;
    register uchar *p;
    int *b1, *b2, *end;
    p = src->bits[0] - 1;
    b2 = line2;
    end = b2 + w;
    if ( use_gray ) {				// 8 bit image
	while ( b2 < end )
	    *b2++ = gray[*++p];
    }
    else {					// 24 bit image
	while ( b2 < end )
	    *b2++ = QGRAY(*++p,*++p,*++p);
    }
    for ( int y=0; y<h; y++ ) {			// for each scan line...
	int *tmp = line1; line1 = line2; line2 = tmp;
	bool not_last_line = y < h - 1;
	if ( not_last_line ) {			// calc. grayvals for next line
	    p = src->bits[y+1] - 1;
	    b2 = line2;
	    end = b2 + w;
	    if ( use_gray ) {			// 8 bit image
		while ( b2 < end )
		    *b2++ = gray[*++p];
	    }
	    else {				// 24 bit image
		while ( b2 < end )
		    *b2++ = QGRAY(*++p,*++p,*++p);
	    }
	}
	int err;
	p = bmline;
	b1 = line1;  b2 = line2;
	end = b1 + w;
	while ( b1 < end ) {
	    if ( *b1 < 128 ) {			// black pixel
		err = *b1++;
		*p++ = 1;
	    }
	    else {				// white pixel
		err = *b1++ - 255;
		*p++ = 0;
	    }
	    if ( b1 != end )
		*b1 += (err*7)/16;		// spread error to right pixel
	    if ( not_last_line ) {
		b2[0] += (err*5)/16;		// pixel below
		if ( b1 != end )
		    b2[1] += err/16;		// pixel below right
		if ( b1 != line1 )
		    b2[-1] += (err*3)/16;	// pixel below left
	    }
	    b2++;
	}
	p = bmline;
	uchar *b = dst->bits[y];
	memset( b, 0, bmwidth );
	for ( int x=0; x<w; x++ ) {
	    if ( *p++ )
		*b |= 1 << (7 - (x & 7));
	    if ( (x & 7) == 7 )
		b++;
	}
    }
    delete line1;
    delete line2;
    delete bmline;
    return TRUE;
}


bool QImageData::copyData( QImageData *dst ) const
{						// copy image data
    dst->width    = width;
    dst->height   = height;
    dst->depth    = depth;
    dst->ncols    = ncols;
    dst->bitOrder = bitOrder;
    if ( dst->ctbl )
	delete dst->ctbl;
    dst->ctbl	= new ulong[ncols];
    if ( !dst->ctbl )
	return FALSE;
    memcpy( dst->ctbl, ctbl, ncols*sizeof(ulong) );
    dst->allocBits();
    if ( dst->bits != 0 ) {			// alloc ok
	memcpy( dst->bits[0], bits[0], numBytes() );
	return TRUE;
    }
    return FALSE;
}


bool QImageData::convertDepth( int newDepth, QImageData *dst ) const
{
    if ( dst == this ) {
#if defined(CHECK_RANGE)
	warning( "QImageData::convertDepth: Source and destination must be "
		 "different image data structures" );
#endif
	return FALSE;
    }
    if ( depth == newDepth )			// !!! should copy
	return copyData( dst );
    else if ( (depth == 8 || depth == 24) && newDepth == 1 ) // dither
	return dither_image( this, dst );
    else if ( depth == 24 && newDepth == 8 )	// 24 -> 8
	return convert_24_to_8( this, dst );
    else if ( depth == 8 && newDepth == 24 )	// 8 -> 24
	return convert_8_to_24( this, dst );
    else if ( depth == 1 && newDepth == 8 )	// 1 -> 8
	return convert_1_to_8( this, dst );
    else if ( depth == 1 && newDepth == 24 )	// 1 -> 24
	return convert_1_to_24( this, dst );
    return FALSE;
}


void QImageData::convertBitOrder( int bo )
{
    if ( bo == bitOrder || bo == IgnoreEndian || bitOrder == IgnoreEndian ||
	 depth != 1 )				// cannot convert bit order
	return;
    setup_bitflip();
    bitOrder = bo;
    register uchar *p;
    uchar *end;
    if ( contiguousBits() ) {			// contiguous bits
	p = bits[0];
	end = p + numBytes();
	while ( p < end )
	    *p++ = bitflip[*p];
    }
    else {
	int w = (width+7)/8;
	for ( int i=0; i<height; i++ ) {	// for each scanline
	    p = bits[i];
	    end = p + w;
	    while ( p < end )
		*p++ = bitflip[*p];
	}
    }
}


static void swapPixel01( QImageData *d )	// 1-bit: swap 0 and 1 pixels
{
    if ( d->depth == 1 && d->ncols == 2 ) {
	register ulong *p = (ulong *)d->bits[0];
	long nbytes = d->numBytes();
	for ( int i=0; i<nbytes/4; i++ )
	    *p++ = ~*p;
	uchar *p2 = (uchar *)p;
	for ( i=0; i<(nbytes&3); i++ )
	    *p2++ = ~*p2;
	ulong t = d->ctbl[0];			// swap color 0 and 1
	d->ctbl[0] = d->ctbl[1];
	d->ctbl[1] = t;
    }
}


QImageIO::QImageIO()
{
    status = 0;
    ioDevice  = 0;
}

QImageIO::~QImageIO()
{
}

static bool matchBytes( const char *hdr, const char *pattern )
{
    QRegExp r( pattern );
    return r.match(hdr) == 0;
}


const char *QImageIO::imageFormat( const char *fileName )
{						// determine image format
    QFile file( fileName );
    if ( !file.open(IO_ReadOnly) )
	return 0;
    const char *format = imageFormat( &file );
    file.close();
    return format;
}

const char *QImageIO::imageFormat( QIODevice *d )
{						// determine image format
    const  buflen = 14;
    char   buf[buflen];
    if ( imageHandlers == 0 )
	init_image_handlers();
    long pos = d->at();				// save position
    int rdlen = d->readBlock( buf, buflen );	// read a few bytes
    const char *format = 0;
    if ( d->status() == IO_Ok && rdlen == buflen ) {
	QImageHandler *p = imageHandlers->first();
	while ( p ) {
	    if ( matchBytes(buf,p->header) ) {	// try match with headers
		format = p->format;
		break;
	    }
	    p = imageHandlers->next();
	}
    }
    d->at( pos );				// restore position
    return format;
}


bool QImageIO::read()				// read image data
{
    QFile 	   file;
    const char 	  *image_format;
    QImageHandler *h;

    if ( ioDevice ) {				// read from io device
	image_format = imageFormat( ioDevice );
	h = get_image_handler( image_format );
    }
    else if ( !fileName.isEmpty() ) {		// read from file
	int fmode = IO_ReadOnly;
	file.setFileName( fileName );
	image_format = imageFormat( fileName );
	h = get_image_handler( image_format );
	if ( image_format ) {
	    if ( h->text_mode )
		fmode |= IO_Translate;
	    if ( !file.open(fmode) )		// cannot open file
		return FALSE;
	    ioDevice = &file;
	}
    }
    else					// no file name or io device
	return FALSE;
    if ( !image_format )			// unknown image format
	return FALSE;
    if ( !format.isEmpty() && format != image_format )
	return FALSE;				// format doesn't match

    status = 1;
    (*h->read_image)( this );
    if ( file.isOpen() ) {			// image was read using file
	file.close();
	ioDevice = 0;
    }
    format = image_format;
    return status == 0;				// image successfully read?
}

bool QImageIO::write()
{
    const char *image_format = format.isEmpty() ? "QT" : format;
    QImageHandler *h = get_image_handler( image_format );
    if ( !h ) {
#if defined(CHECK_RANGE)
	warning( "QImageIO::write: No such image format handler: %s",
		 (char *)format );
#endif
	return FALSE;
    }
    QFile file;
    if ( ioDevice )
	;
    else if ( !fileName.isEmpty() ) {
	file.setFileName( fileName );
	int fmode = h->text_mode ? IO_WriteOnly|IO_Translate : IO_WriteOnly;
	if ( !file.open(fmode) )		// couldn't create file
	    return FALSE;
	ioDevice = &file;
    }
    status = 1;
    (*h->write_image)( this );
    if ( file.isOpen() ) {			// image was written using file
	file.close();
	ioDevice = 0;
    }
    return status == 0;				// image successfully written?
}


// --------------------------------------------------------------------------
// QPixmap stream functions
//

QDataStream &operator<<( QDataStream &s, const QPixmap &pixmap )
{
    QImageIO io;
    pixmap.getImageData( &io );
    io.ioDevice = s.device();
    io.format = "QT";
    io.write();
    return s;
}

QDataStream &operator>>( QDataStream &s, QPixmap &pixmap )
{
    QImageIO io;
    io.ioDevice  = s.device();
    io.format = "QT";
    io.read();
    pixmap.setImageData( &io );
    return s;
}


// --------------------------------------------------------------------------
// Qt image read/write functions
//

static void read_qt_image( QImageIO *image )	// read Qt image data
{
    QIODevice  *d = image->ioDevice;
    QDataStream s( d );
    bool	depth4;
    const	buflen = 24;
    char	buf[buflen];
    int		i;

    d->readBlock( buf, 4 );			// read signature
    if ( strncmp(buf,"QIMG",4) != 0 )
	return;
    INT16 minor, major;
    INT32 width, height, depth, ncols;
    s >> minor >> major;			// read image info
    s >> width >> height >> depth >> ncols;
    INT16 extra;
    s >> extra;
    if ( extra > 0 )				// skip extra data
	d->at( d->at() + extra );
    if ( d->atEnd() )				// end of file
	return;

    if ( (depth4 = (depth == 4)) )		// 4 bit encoded?
	depth = 8;
    ulong *c = 0;
    if ( ncols ) {				// read colormap
	c = new ulong[ncols];
	if ( !c )				// could not allocate colormap
	    return;
	UINT32 col;
	for ( i=0; i<ncols; i++ ) {
	    s >> col;  c[i] = col;
	}
    }
    image->width  = (int)width;
    image->height = (int)height;
    image->depth  = (int)depth;
    image->ncols  = (int)ncols;
    image->ctbl   = c;
    image->allocBits();
    if ( !image->bits )				// could not allocate bits
	return;
    register uchar *p;
    long nbytes = image->numBytes();
    int  bpl  	= nbytes/image->height;
    if ( image->contiguousBits() && !depth4 )
	d->readBlock( (char*)image->bits[0], nbytes );
    else {					// read each scanline
	if ( depth4 )
	    bpl = (bpl+1)/2;
	uchar *buf = new uchar[bpl];
	uchar *b, *end;
	for ( int y=0; y<image->height; y++ ) {
	    uchar *data = depth4 ? buf : image->bits[y];
	    if ( d->readBlock((char *)data,bpl) != bpl )
		break;
	    if ( depth4 ) {			// convert 4 -> 8 bit depth
		p = image->bits[y];
		b = buf;
		end = b + image->width/2;
		while ( b < end ) {
		    *p++ = *b >> 4;
		    *p++ = *b++ & 0x0f;
		}
		if ( image->width & 1 )
		    *p = *b >> 4;
	    }
	}
	delete buf;
    }
    if ( image->depth == 1 )
	image->bitOrder = QImageData::BigEndian;
    image->status = 0;				// image successfully read
}


static void write_qt_image( QImageIO *image )	// write Qt image data
{
    QIODevice  *d = image->ioDevice;
    QDataStream s( d );
    bool	depth4 = image->depth == 8  && image->ncols <= 16;
    int		i;

    d->writeBlock( "QIMG", 4 );			// write signature
    s << (INT16)1 << (INT16)0;			// version x.y
    s << (INT32)image->width			// write image info
      << (INT32)image->height;
    s << (INT32)(depth4 ? 4 : image->depth);
    s << (INT32)image->ncols;
    s << (INT16)0;				// size of extra info

    if ( image->ncols ) {			// write colormap
	for ( i=0; i<image->ncols; i++ )
	    s << (UINT32)image->ctbl[i];
    }

    QImageIO tmp_image;
    if ( image->depth == 1 && image->bitOrder != QImageData::BigEndian ) {
	image->copyData( &tmp_image );		// get the right bit order
	tmp_image.convertBitOrder( QImageData::BigEndian );
	image = &tmp_image;
    }
    register uchar *p;
    long nbytes = image->numBytes();
    int  bpl 	= nbytes/image->height;
    if ( image->contiguousBits() && !depth4 )
	d->writeBlock( (char*)image->bits[0], nbytes );
    else {					// write as many scanlines
	if ( depth4 )
	    bpl = (bpl+1)/2;
	uchar *buf = new uchar[bpl];
	uchar *b, *end;
	for ( int y=0; y<image->height; y++ ) {
	    uchar *data = image->bits[y];
	    if ( depth4 ) {			// convert 8 -> 4 bit depth
		p = data;
		b = buf;
		end = b + image->width/2;
		while ( b < end ) {
		    *b++ = (*p << 4) | (*(p+1) & 0x0f);
		    p += 2;
		}
		if ( image->width & 1 )
		    *b = *p << 4;
		data = buf;
	    }
	    d->writeBlock( (char *)data, bpl );
	}
	delete buf;
    }
    image->status = 0;				// image successfully written
}


// --------------------------------------------------------------------------
// GIF image read/write functions
//
// The Graphical Interchange Format (c) is the Copyright property of
// CompuServe Incorporated.  GIF (sm) is a Service Mark propery of
// CompuServe Incorporated.
//

static void read_gif_image( QImageIO *image )	// read GIF image data
{
    warning( "Qt: GIF not supported in this version" );
}


static void write_gif_image( QImageIO *image )	// write GIF image data
{
    warning( "Qt: GIF not supported in this version" );
}


// --------------------------------------------------------------------------
// BMP (DIB) image read/write functions
//

const  BMP_FILEHDR_SIZE = 14;			// size of BMP_FILEHDR data

struct BMP_FILEHDR {				// BMP file header
    char   bfType[2];				// "BM"
    INT32  bfSize;				// size of file
    INT16  bfReserved1;
    INT16  bfReserved2;
    INT32  bfOffBits;				// pointer to the pixmap bits
};

QDataStream &operator>>( QDataStream &s, BMP_FILEHDR &bf )
{						// read file header
    s.readRawBytes( bf.bfType, 2 );
    s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
    return s;
}

QDataStream &operator<<( QDataStream &s, const BMP_FILEHDR &bf )
{						// write file header
    s.writeRawBytes( bf.bfType, 2 );
    s << bf.bfSize << bf.bfReserved1 << bf.bfReserved2 << bf.bfOffBits;
    return s;
}


const  BMP_OLD = 12;				// old Windows/OS2 BMP size
const  BMP_WIN = 40;				// new Windows BMP size
const  BMP_OS2 = 64;				// new OS/2 BMP size

const  BMP_RGB  = 0;				// no compression
const  BMP_RLE8 = 1;				// run-length encoded, 8 bits
const  BMP_RLE4 = 2;				// run-length encoded, 4 bits

struct BMP_INFOHDR {				// BMP information header
    INT32  biSize;				// size of this struct
    INT32  biWidth;				// pixmap width
    INT32  biHeight;				// pixmap height
    INT16  biPlanes;				// should be 1
    INT16  biBitCount;				// number of bits per pixel
    INT32  biCompression;			// compression method
    INT32  biSizeImage;				// size of image
    INT32  biXPelsPerMeter;			// horizontal resolution
    INT32  biYPelsPerMeter;			// vertical resolution
    INT32  biClrUsed;				// number of colors used
    INT32  biClrImportant;			// number of important colors
};


QDataStream &operator>>( QDataStream &s, BMP_INFOHDR &bi )
{
    s >> bi.biSize;
    if ( bi.biSize == BMP_WIN || bi.biSize == BMP_OS2 ) {
	s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
	s >> bi.biCompression >> bi.biSizeImage;
	s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
	s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {					// probably old Windows format
	INT16 w, h;
	s >> w >> h >> bi.biPlanes >> bi.biBitCount;
	bi.biWidth  = w;
	bi.biHeight = h;
	bi.biCompression = BMP_RGB;		// no compression
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
	bi.biClrUsed = bi.biClrImportant = 0;
    }
    return s;
}

QDataStream &operator<<( QDataStream &s, const BMP_INFOHDR &bi )
{
    s << bi.biSize;
    s << bi.biWidth << bi.biHeight;
    s << bi.biPlanes;
    s << bi.biBitCount;
    s << bi.biCompression;
    s << bi.biSizeImage;
    s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
    s << bi.biClrUsed << bi.biClrImportant;
    return s;
}


static void read_bmp_image( QImageIO *image )	// read BMP image data
{
    QIODevice  *d = image->ioDevice;
    QDataStream s( d );
    BMP_FILEHDR	bf;
    BMP_INFOHDR	bi;
    int		startpos = d->at();

    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    s >> bf;					// read BMP file header
    if ( strncmp(bf.bfType,"BM",2) != 0 )	// not a BMP image
	return;
    s >> bi;					// read BMP info header
    if ( d->atEnd() )				// end of stream/file
	return;
#if 0
    debug( "biSize...........%d", bi.biSize );
    debug( "biWidth..........%d", bi.biWidth );
    debug( "biHeight.........%d", bi.biHeight );
    debug( "biPlanes.........%d", bi.biPlanes );
    debug( "biBitCount.......%d", bi.biBitCount );
    debug( "biCompression....%d", bi.biCompression );
    debug( "biSizeImage......%d", bi.biSizeImage );
    debug( "biXPelsPerMeter..%d", bi.biXPelsPerMeter );
    debug( "biYPelsPerMeter..%d", bi.biYPelsPerMeter );
    debug( "biClrUsed........%d", bi.biClrUsed );
    debug( "biClrImportant...%d", bi.biClrImportant );
#endif
    int w = bi.biWidth,  h = bi.biHeight,  nbits = bi.biBitCount;
    int t = bi.biSize,   comp = bi.biCompression;
    if ( !(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 24) ||
	 bi.biPlanes != 1 || comp > BMP_RLE4 )
	return;					// weird BMP image
    if ( comp != BMP_RGB && (nbits == 4 && comp != BMP_RLE4) &&
	 (nbits == 8 && comp != BMP_RLE8) )
	 return;				// weird compression type
    d->at( startpos + BMP_FILEHDR_SIZE + bi.biSize ); // goto start of colormap
    int    ncols;
    ulong *c;
    if ( nbits == 24 ) {			// there's no colormap
	ncols = 0;
	c = 0;
    }
    else {					// read colormap
	ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;
	c = new ulong[ ncols ];
	CHECK_PTR( c );
	uchar rgb[4];
	int   rgb_len = t == BMP_OLD ? 3 : 4;
	for ( int i=0; i<ncols; i++ ) {
	    d->readBlock( (char *)rgb, rgb_len );
	    c[i] = QRGB(rgb[2],rgb[1],rgb[0]);
	    if ( d->atEnd() ) {			// truncated file
		delete c;
		return;
	    }
	}
    }
    image->ncols  = ncols;
    image->ctbl   = c;
    image->width  = w;
    image->height = h;
    image->depth  = nbits == 4 ? 8 : nbits;	// depth can be 1,8,24
    image->allocBits();

    d->at( startpos + bf.bfOffBits );		// start of image data

    int  padlen;
    char padbuf[8];

    if ( nbits == 1 ) {				// 1 bit BMP image
	image->bitOrder = QImageData::BigEndian;
	w = (w+7)/8;
	padlen = ((w+3)/4)*4 - w;
	while ( --h >= 0 ) {
	    if ( d->readBlock((char*)image->bits[h],w) != w )
		break;
	    if ( padlen )
		d->readBlock( padbuf, padlen );
	}
	if ( ncols == 2 && QGRAY(c[0]) < QGRAY(c[1]) )
	    swapPixel01( image );
    }

    else if ( nbits == 4 ) {			// 4 bit BMP image
	int    buflen = ((w+7)/8)*4;
	uchar *buf = new uchar[buflen];
	CHECK_PTR( buf );
	if ( comp == BMP_RLE4 ) {		// run length compression
	    int x=0, y=0, b, c, i;
	    register uchar *p = image->bits[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = image->bits[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = image->bits[h-y-1] + x;
			    break;
			default:		// absolute mode
			    i = (c = b)/2;
			    while ( i-- ) {
				b = d->getch();
				*p++ = b >> 4;
				*p++ = b & 0x0f;
			    }
			    if ( c & 1 )
				*p++ = d->getch() >> 4;
			    if ( (((c & 3) + 1) & 2) == 2 )
				d->getch();	// align on word boundary
			    x += c;
		    }
		}
		else {				// encoded mode
		    i = (c = b)/2;
		    b = d->getch();		// 2 pixels to be repeated
		    while ( i-- ) {
			*p++ = b >> 4;
			*p++ = b & 0x0f;
		    }
		    if ( c & 1 )
			*p++ = b >> 4;
		    x += c;
		}
	    }
	}
	else if ( comp == BMP_RGB ) {		// no compression
	    while ( --h >= 0 ) {
		if ( d->readBlock((char*)buf,buflen) != buflen )
		    break;
		register uchar *p = image->bits[h];
		uchar *b = buf;
		for ( int i=0; i<w/2; i++ ) {	// convert nibbles to bytes
		    *p++ = *b >> 4;
		    *p++ = *b++ & 0x0f;
		}
		if ( w & 1 )			// the last nibble
		    *p = *b >> 4;
	    }
	}
	delete buf;
    }

    else if ( nbits == 8 ) {			// 8 bit BMP image
	if ( comp == BMP_RLE8 ) {		// run length compression
	    int x=0, y=0, b;
	    register uchar *p = image->bits[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = image->bits[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = image->bits[h-y-1] + x;
			    break;
			default:		// absolute mode
			    d->readBlock( (char *)p, b );
			    if ( (b & 1) == 1 )
				d->getch();	// align on word boundary
			    x += b;
			    p += b;
		    }
		}
		else {				// encoded mode
		    memset( p, d->getch(), b );	// repeat pixel
		    x += b;
		    p += b;
		}
	    }
	}
	else if ( comp == BMP_RGB ) {		// uncompressed
	    padlen = ((w+3)/4)*4 - w;
	    while ( --h >= 0 ) {
		if ( d->readBlock((char *)image->bits[h],w) != w )
		    break;
		if ( padlen )
		    d->readBlock( padbuf, padlen );
	    }
	}
    }

    else if ( nbits == 24 ) {			// 24 bit BMP image
	padlen = (4 - ((w*3)&3)) & 3;
	while ( --h >= 0 ) {
	    register uchar *p = image->bits[h];
	    if ( d->readBlock( (char *)p,w*3) != w*3 )
		break;
	    for ( int i=0; i<w; i++ ) {		// swap r and b
		uchar t = p[0];
		p[0] = p[2];
		p[2] = t;
		p += 3;
	    }
	    if ( padlen )
		d->readBlock( padbuf, padlen );
	}
    }
    image->status = 0;				// image ok
}


static void write_bmp_image( QImageIO *image )	// write BMP image data
{
    QIODevice  *d = image->ioDevice;
    QDataStream s( d );
    BMP_FILEHDR	bf;
    BMP_INFOHDR	bi;
    long	bpl = image->numBytes()/image->height;
    bool	depth4 = image->depth == 8 && image->ncols <= 16;
    long	bpl_bmp;

    if ( depth4 )
	bpl_bmp = (((bpl+1)/2+3)/4)*4;
    else
	bpl_bmp = ((bpl+3)/4)*4;

    image->status = 0;
    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    strncpy( bf.bfType, "BM", 2 );		// build file header
    bf.bfReserved1 = bf.bfReserved2 = 0;	// reserved, should be zero
    bf.bfOffBits   = BMP_FILEHDR_SIZE + BMP_WIN + image->ncols*4;
    bf.bfSize      = bf.bfOffBits + bpl_bmp*image->height;
    s << bf;					// write file header

    bi.biSize          = BMP_WIN;		// build info header
    bi.biWidth         = image->width;
    bi.biHeight        = image->height;
    bi.biPlanes        = 1;
    bi.biBitCount      = depth4 ? 4 : image->depth;
    bi.biCompression   = BMP_RGB;
    bi.biSizeImage     = bpl_bmp*image->height;
    bi.biXPelsPerMeter = 2834;			// 72 dpi
    bi.biYPelsPerMeter = 2834;
    bi.biClrUsed       = image->ncols;
    bi.biClrImportant  = image->ncols;
    s << bi;					// write info header

    if ( image->depth != 24 ) {			// image has color table
	uchar rgb[4];
	ulong *c = image->ctbl;
	rgb[3] = 0;
	for ( int i=0; i<image->ncols; i++ ) {	// write color table
	    rgb[0] = QBLUE ( c[i] );
	    rgb[1] = QGREEN( c[i] );
	    rgb[2] = QRED  ( c[i] );
	    d->writeBlock( (char *)rgb, 4 );
	}
    }

    QImageIO tmp_image;
    if ( image->depth == 1 && image->bitOrder != QImageData::BigEndian ) {
	image->copyData( &tmp_image );		// get the right bit order
	tmp_image.convertBitOrder( QImageData::BigEndian );
	image = &tmp_image;
    }
    bool   flip = image->depth == 24;
    uchar *buf  = new uchar[bpl_bmp];
    uchar *b, *end;
    register uchar *p;

    memset( buf, 0, bpl_bmp );
    for ( int y=image->height-1; y>=0; y-- ) {	// write the image bits
	if ( depth4 ) {				// convert 8 -> 4 bits
	    p = image->bits[y];
	    b = buf;
	    end = b + image->width/2;
	    while ( b < end ) {
		*b++ = (*p << 4) | (*(p+1) & 0x0f);
		p += 2;
	    }
	    if ( image->width & 1 )
		*b = *p << 4;
	}
	else if ( flip ) {			// RGB -> BGR
	    b = image->bits[y];
	    p = buf;
	    end = p + bpl;
	    while ( p < end ) {
		*p++ = b[2];
		*p++ = b[1];
		*p++ = b[0];
		b += 3;
	    }
	}
	else
	    memcpy( buf, image->bits[y], bpl );
	d->writeBlock( (char*)buf, bpl_bmp );
    }
    delete buf;
}


// --------------------------------------------------------------------------
// PBM/PGM/PPM (ASCII and RAW) image read/write functions
//

static int read_pbm_int( QIODevice *d )		// read int, skip comments
{
    int   c;
    int   val = -1;
    bool  digit;
    const buflen = 100;
    char  buf[buflen];
    while ( TRUE ) {
	if ( (c=d->getch()) == EOF )		// end of file
	    break;
	digit = isdigit(c);
	if ( val != -1 ) {
	    if ( digit ) {
		val = 10*val + c - '0';
		continue;
	    }
	    else {
		if ( c == '#' )			// comment
		    d->readLine( buf, buflen );
		break;
	    }
	}
	if ( digit )				// first digit
	    val = c - '0';
	else if ( isspace(c) )
	    continue;
	else if ( c == '#' )
	    d->readLine( buf, buflen );
	else
	    break;
    }
    return val;
}

static void read_pbm_image( QImageIO *image )	// read PBM image data
{
    const       buflen = 300;
    char        buf[buflen];
    QRegExp     r1, r2;
    QIODevice  *d = image->ioDevice;
    int	      	w, h, nbits, mcc;
    char	type;
    bool	raw;

    d->readBlock( buf, 3 );			// read P[1-6]<white-space>
    if ( !(buf[0] == 'P' && isdigit(buf[1]) && isspace(buf[2])) )
	return;
    switch ( (type=buf[1]) ) {
	case '1':				// ascii PBM
	case '4':				// raw PBM
	    nbits = 1;
	    image->bitOrder = QImageData::BigEndian;
	    break;
	case '2':				// ascii PGM
	case '5':				// raw PGM
	    nbits = 8;
	    break;
	case '3':				// ascii PPM
	case '6':				// raw PPM
	    nbits = 24;
	    break;
        default:
	    return;
    }
    raw = type >= '4';
    w = read_pbm_int( d );			// get image width
    h = read_pbm_int( d );			// get image height
    if ( nbits == 1 )
	mcc = 0;				// ignore max color component
    else
	mcc = read_pbm_int( d );		// get max color component
    if ( w <= 0 || w > 32767 || h <= 0 || h > 32767 || mcc < 0 || mcc > 32767 )
	return;					// weird P.M image

    ASSERT( mcc <= 255 );
    image->width  = w;				// set image data
    image->height = h;
    image->depth  = nbits;
    image->allocBits();

    if ( raw ) {				// read raw data
	if ( image->contiguousBits() )		// read everything at once
	    d->readBlock( (char *)image->bits[0], image->numBytes() );
	else {
	    long bpl = image->numBytes()/h;
	    for ( int i=0; i<h; i++ ) {		// read each scanline
		if ( d->readBlock((char *)image->bits[i],bpl) != bpl )
		    break;
	    }
	}
    }
    else {					// read ascii data
	register uchar *p = image->bits[0];
	long n = image->numBytes();
	if ( nbits == 1 ) {
	    int b;
	    while ( n-- ) {
		b = 0;
		for ( int i=0; i<8; i++ )
		    b = (b << 1) | (read_pbm_int(d) & 1);
		*p++ = b;
	    }
	}
	else {
	    while ( n-- ) {
		*p++ = read_pbm_int( d );
	    }
	}
    }
    ulong *c;
    int   nc;
    if ( nbits == 1 ) {				// bitmap
	c = new ulong[nc=2];
	CHECK_PTR( c );
	c[0] = QRGB(255,255,255);		// white
	c[1] = QRGB(0,0,0);			// black
    }
    else if ( nbits == 8 ) {			// graymap
	c = new ulong[nc=mcc];			// !!! not correct
	CHECK_PTR( c );
	for ( int i=0; i<mcc; i++ )
	    c[i] = QRGB(i*255/mcc,i*255/mcc,i*255/mcc);
    }
    else {					// 24 bit pixmap
	nc = 0;
	c = 0;
    }
    image->ncols = nc;
    image->ctbl  = c;

    image->status = 0;				// image ok
}


static void write_pbm_image( QImageIO *image )	// write PBM image data
{
    warning( "Qt: %s output not supported in this version",
	     (char *)image->format );
}


// --------------------------------------------------------------------------
// X bitmap image read/write functions
//

static inline int hex2byte( register char *p )
{
    return (isdigit(*p)     ? *p     - '0' : toupper(*p)     - 'A' + 10) << 4 |
	   (isdigit(*(p+1)) ? *(p+1) - '0' : toupper(*(p+1)) - 'A' + 10);
}

static void read_xbm_image( QImageIO *image )	// read X bitmap image data
{
    const       buflen = 300;
    char        buf[buflen];
    QRegExp     r1, r2;
    QIODevice  *d = image->ioDevice;
    int	        i;
    int	      	w=-1, h=-1;

    r1 = "^#define[\x20\t]+[a-zA-Z0-9_]+[\x20\t]+";
    r2 = "[0-9]+";
    d->readLine( buf, buflen );			// "#define .._width <num>"
    if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i )
	w = atoi( &buf[i] );
    d->readLine( buf, buflen );			// "#define .._height <num>"
    if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i )
	h = atoi( &buf[i] );
    if ( w <= 0 || w > 32767 || h <= 0 || h > 32767 )
	return;					// format error

    while ( TRUE ) {				// scan for data
	if ( d->readLine(buf, buflen) == 0 )	// end of file
	    return;
	if ( strstr(buf,"0x") != 0 )		// does line contain data?
	    break;
    }

    ulong *c = new ulong[2];			// create colormap table
    if ( !c )
	return;
    c[0] = QRGB(255,255,255);			// white
    c[1] = QRGB(0,0,0);				// black
    image->width  = w;				// set image data
    image->height = h;
    image->depth  = 1;
    image->ncols  = 2;
    image->ctbl   = c;
    image->allocBits();
    image->bitOrder =  QImageData::LittleEndian;
    if ( !image->bits )
	return;

    int	   x = 0, y = 0;
    uchar *b = image->bits[0];
    char  *p = strstr( buf, "0x" );
    w = (w+7)/8;				// byte width

    while ( y < h ) {				// for all encoded bytes...
	if ( p ) {				// p = "0x.."
	    *b++ = hex2byte(p+2);
	    p += 2;
	    if ( ++x == w ) {
		b = image->bits[++y];
		x = 0;
	    }
	    p = strstr( p, "0x" );
	}
	else {					// read another line
	    if ( !d->readLine(buf,buflen) )	// EOF ==> truncated image
		break;
	    p = strstr( buf, "0x" );
	}
    }

    image->status = 0;				// image ok
}


static void write_xbm_image( QImageIO *image )	// write X bitmap image data
{
    QIODevice *d = image->ioDevice;
    int        w = image->width;
    int	       h = image->height;
    int	       i;
    QString    s = fbname(image->fileName);	// get file base name
    char       buf[100];

    sprintf( buf, "#define %s_width %d\n",  (char *)s, w );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "#define %s_height %d\n", (char *)s, h );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "static char %s_bits[] = {\n ", (char *)s );
    d->writeBlock( buf, strlen(buf) );

    image->status = 0;

    QImageIO tmp_image;
    if ( image->depth != 1 || image->bitOrder != QImageData::LittleEndian ) {
	if ( image->depth > 1 )			// convert to monochrome
	    image->convertDepth( 1, &tmp_image );
	else
	    image->copyData( &tmp_image );
	image = &tmp_image;
	image->convertBitOrder( QImageData::LittleEndian );
    }

    bool invert = QGRAY(image->ctbl[0]) < QGRAY(image->ctbl[1]);
    char hexrep[16];
    for ( i=0; i<10; i++ )
	hexrep[i] = '0' + i;
    for ( i=10; i<16; i++ )
	hexrep[i] = 'a' -10 + i;
    if ( invert ) {
	char t;
	for ( i=0; i<8; i++ ) {
	    t = hexrep[15-i];
	    hexrep[15-i] = hexrep[i];
	    hexrep[i] = t;
	}
    }
    int bcnt = 0;
    register char *p = buf;
    uchar *b = image->bits[0];
    int  x=0, y=0;
    long nbytes = image->numBytes();
    w = (w+7)/8;
    while ( nbytes-- ) {			// write all bytes
	*p++ = '0';  *p++ = 'x';
	*p++ = hexrep[*b >> 4];
	*p++ = hexrep[*b++ & 0xf];
	if ( ++x == w ) {
	    b = image->bits[++y];
	    x = 0;
	}
	if ( nbytes > 0 ) {
	    *p++ = ',';
	    if ( ++bcnt > 14 ) {
		*p++ = '\n';
		*p++ = ' ';
		*p++ = '\0';
		d->writeBlock( buf, strlen(buf) );
		p = buf;
		bcnt = 0;
	    }
	}
    }
    strcpy( p, " };\n" );
    d->writeBlock( buf, strlen(buf) );
}


// --------------------------------------------------------------------------
// XPM image read/write functions
//

static int read_xpm_char( QIODevice *d )
{
    static bool inside_quotes = FALSE;
    static int  save_char = 256;
    int c;
    if ( save_char != 256 ) {
	c = save_char;
	save_char = 256;
	return c;
    }
    if ( (c=d->getch()) == EOF )
	return EOF;
    if ( c == '"' )				// start or end of quotes
	inside_quotes = !inside_quotes;
    else {
	if ( !inside_quotes && c == '/' ) {	// C /* .. */ comment?
	    if ( (c=d->getch()) == EOF )
		return c;
	    if ( c == '*' ) {			// yes, it is a comment
		while ( TRUE ) {
		    if ( (c=d->getch()) == EOF )
			return c;
		    else if ( c == '*' ) {
			if ( (c=d->getch()) == EOF )
			    return c;
			if ( c == '/' ) {
			    c = d->getch();
			    break;
			}
		    }
		}
	    }
	    else
		save_char = c;
	}
    }
    return c;
}


static void read_xpm_image( QImageIO *image )	// read XPM image data
{
#if 0
    const       buflen = 200;
    char        buf[buflen];
    char       *p = buf;
    QRegExp     r = "/\\*.XPM.\\*/";
    QIODevice  *d = image->iodev;
    int	      	c, cpp, ncols, w, h;

    d->readLine( buf, buflen );			// "/* XPM */"
    if ( r.match(buf) < 0 )
	return;
    while ( (c=read_xpm_char(d)) != EOF && c != '"' )
	;
    while ( (c=read_xpm_char(d)) != EOF && c != '"' )
	*p++ = c;
    if ( c == EOF )
	return;
    *p = '\0';
    sscanf( buf, "%d %d %d %d", &w, &h, &ncols, &cpp );
    debug( "%d %d %d %d", w, h, ncols, cpp );
    if ( ncols > 256 )				// 24 bit colors
	image->depth = 24;
    else if ( ncols > 2 )			// standard 8 bit color
	image->depth = 8;
    else					// monochrome
	image->depth = 2;
    image->ncols = ncols;
    image->width = w;
    image->height = h;
#endif
}


static void write_xpm_image( QImageIO *image )	// write XPM image data
{
#if 0
    QIODevice *d = image->iodev;
    int w = image->width, h = image->height, depth = image->depth, i;
    QString s = fbname(image->fname);		// get file base name
    char buf[100];
#endif
}

