/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.cpp#7 $
**
** Implementation of QImage class
**
** Author  : Haavard Nord
** Created : 950207
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qimage.h"
#include "qregexp.h"
#include "qfile.h"
#include "qdstream.h"
#include "qbitmap.h"				// TO BE REMOVED !!!
#include <stdlib.h>
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qimage.cpp#7 $";
#endif


#undef MIN
#undef MAX
#define MIN(x,y)  ((x)<(y) ? (x) : (y))
#define MAX(x,y)  ((x)>(y) ? (x) : (y))


static void read_qt_image( QImageIO * );	// standard image io handlers
static void write_qt_image( QImageIO * );	//   (defined below)
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


QImageData::QImageData()
{
    width = height = depth = ncols = -1;
    ctbl  = 0;
    bits  = 0;
    bigend = FALSE;	/* TODO!!! How to handle this??? */
}

QImageData::~QImageData()
{
    if ( ctbl )
	delete ctbl;
    if ( bits )
	freeBits();
}


long QImageData::nBytes() const			// number of bytes image data
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


void QImageData::allocBits()
{
    if ( bits )
	freeBits();
    if ( width <= 0 || height <= 0 )
	return;
    long nbytes = nBytes();			// bytes total image bits
    long bpl    = nbytes/height;		// bytes per line
    int  ptrtbl = height*sizeof(uchar*);	// size of pointer table
    uchar *p = (uchar *)calloc( nbytes+ptrtbl+sizeof(int), 1 );
    CHECK_PTR( p );
    *((int *)p) = height;			// encode height as first int
    p += sizeof(int);
    bits = (uchar**)p;				// set image pointer
    uchar **h = bits;				// pointer table
    uchar *d = p + ptrtbl;			// start of image data
    for ( int i=0; i<height; i++ ) {
	bits[i] = d;
	d += bpl;
    }
}

void QImageData::freeBits()
{
    if ( bits ) {
	uchar *p = (uchar *)bits;
	p -= sizeof(int);			// real start of pointer
	free( p );
	bits = 0;
    }
}


QImageIO::QImageIO()
{
    status = 0;
    iodev  = 0;
    length = -1;
}

QImageIO::~QImageIO()
{
}

// --------------------------------------------------------------------------
// Image handler
//

struct QImageHandler {
    char	       *format;			// image format
    char	       *header;			// image header pattern
    image_io_handler	read_image;		// image read function
    image_io_handler	write_image;		// image write function
};

const		     max_image_handlers = 24;
static QImageHandler image_handlers[max_image_handlers];
static int	     num_image_handlers = 0;
static bool	     is_installing_standards = FALSE;

void install_standard_handlers()		// install standard handlers
{
    if ( num_image_handlers == 0 && !is_installing_standards ) {
	is_installing_standards = TRUE;		// avoid total recursion
	QImage::defineIOHandler( "QT", "^QIMG",
				 read_qt_image, write_qt_image );
	QImage::defineIOHandler( "GIF", "^GIF[0-9][0-9][a-z]",
				 read_gif_image, write_gif_image );
	QImage::defineIOHandler( "BMP", "^BM",
				 read_bmp_image, write_bmp_image );
	QImage::defineIOHandler( "PBM", "^P1",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "PBMRAW", "^P4",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "PGM", "^P2",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "PGMRAW", "^P5",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "PPM", "^P3",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "PPMRAW", "^P6",
				 read_pbm_image, write_pbm_image );
	QImage::defineIOHandler( "XBM", "^#define",
				 read_xbm_image, write_xbm_image );
	QImage::defineIOHandler( "XPM", "/\\*.XPM.\\*/",
				 read_xpm_image, write_xpm_image );
	is_installing_standards = FALSE;
    }
}

QImageHandler *get_image_handler( const char *format )
{						// get pointer to handler
    if ( num_image_handlers == 0 )
	install_standard_handlers();
    register QImageHandler *p = &image_handlers[0];
    for ( int i=0; i<num_image_handlers; i++ ) {
	if ( strcmp(p->format,format) == 0 )
	    return p;
	p++;
    }
    return 0;					// no such handler
}

void QImage::defineIOHandler( const char *format,
			      const char *header,
			      image_io_handler read_image,
			      image_io_handler write_image )
{
    if ( num_image_handlers == 0 )
	install_standard_handlers();
    if ( num_image_handlers >= max_image_handlers ) {
#if defined(CHECK_RANGE)
	warning( "QImage::defineIOHandler: Too many handlers" );
#endif
	return;
    }
    else if ( get_image_handler(format) != 0 ) {
#if defined(CHECK_RANGE)
	warning( "QImage::defineIOHandler: Handler for %s already defined",
		 format );
#endif
	return;
    }
    register QImageHandler *p = &image_handlers[num_image_handlers++];
    p->format = (char *)format;
    p->header = (char *)header;
    p->read_image = read_image;
    p->write_image = write_image;
}


// --------------------------------------------------------------------------
// QImage member functions
//

QImage::QImage()
{
    install_standard_handlers();
    data = new QImagePix;
    CHECK_PTR( data );
    data->pm = 0;
}

QImage::QImage( int w, int h, int depth )
{
    install_standard_handlers();
    data = new QImagePix;
    CHECK_PTR( data );
    data->pm = new QPixMap( w, h, depth );
}

QImage::QImage( const QPixMap &pixmap )
{
    install_standard_handlers();
    data = new QImagePix;
    CHECK_PTR( data );
    data->pm = (QPixMap*)&pixmap;
}

QImage::QImage( const QImage &image )
{
    install_standard_handlers();
    data = image.data;
    data->ref();
}

QImage::QImage( const QImageData *image )
{
    install_standard_handlers();
    data = new QImagePix;
    CHECK_PTR( data );
    data->pm = 0;
    createImage( image );
}

QImage::~QImage()
{
    if ( data->deref() ) {
	delete data->pm;
	delete data;
    }
}

QImage &QImage::operator=( const QPixMap &pixmap )
{
    if ( data->deref() )
	delete data;
    data = new QImagePix;
    CHECK_PTR( data );
    data->pm = (QPixMap*)&pixmap;
    return *this;
}

QImage &QImage::operator=( const QImage &image )
{
    image.data->ref();
    if ( data->deref() )
	delete data;
    data = image.data;
    return *this;
}


bool QImage::operator==( const QImage &image ) const
{
    return data == image.data;
}


QImage QImage::copy() const
{
    if ( isNull() ) {
	QImage nullImage;
	return nullImage;
    }
    else {					// copy image
	int w=data->pm->width(), h=data->pm->height();
	int d=data->pm->depth();
	QPixMap *pm = new QPixMap( w, h, d );	// create new pixmap
	CHECK_PTR( pm );
	data->pm->bitBlt( 0, 0, w, h,		// copy from this pixmap
			  pm, 0, 0 );		//   into new pixmap
	QImage image( *pm );
	return image;
    }
}


int QImage::width() const
{
    return data->pm ? data->pm->width() : -1;
}

int QImage::height() const
{
    return data->pm ? data->pm->height() : -1;
}

QSize QImage::size() const
{
    return data->pm ? data->pm->size() : QSize(-1,-1);
}

QRect QImage::rect() const
{
    return data->pm ? data->pm->rect() : QRect(0,0,0,0);
}


int QImage::depth() const
{
    return data->pm ? data->pm->depth() : -1;
}

int QImage::numColors() const
{
    return data->pm ? (1 << data->pm->depth()) : -1;
}


QImage::operator QPixMap &() const
{
    static QPixMap *dummy = 0;
    if ( !data->pm && !dummy ) {		// !!!NOTE: Use pixmap cache
	dummy = new QPixMap( 8, 8 );
	dummy->fill( white );
    }
    return data->pm ? *data->pm : *dummy;
}


void QImage::resize( int w, int h )
{
    if ( data->pm ) {				// has pixmap
	QPixMap *pm = new QPixMap( w, h );	// create new pixmap
	CHECK_PTR( pm );
	data->pm->bitBlt( 0, 0,			// copy old pixmap
			  MIN(pm->width(), data->pm->width()),
			  MIN(pm->height(),data->pm->height()),
			  pm, 0, 0 );
	delete data->pm;			// delete old pixmap
	data->pm = pm;
    }
    else					// create new pixmap
	data->pm = new QPixMap( w, h );
}

void QImage::fill( const QColor &color )
{
    if ( data->pm )				// only if existing pixmap
	data->pm->fill( color );
}


void QImage::createImage( const QImageData *image )
{						// create image from data
    if ( data->pm )
	data->pm->createPixMap( image );
    else
	data->pm = new QPixMap( image );
}


static bool matchBytes( const char *hdr, const char *pattern )
{
    QRegExp r( pattern );
    return r.match(hdr) == 0;
}

const char *QImage::imageType( const char *fileName )
{						// determine image format
    const  buflen = 14;
    char   buf[buflen];
    if ( num_image_handlers == 0 )
	install_standard_handlers();
    QFile file( fileName );
    file.open( IO_ReadOnly );			// read file header
    int   fileSize = file.size();
    if ( fileSize < buflen ) {			// short file
	file.close();
	return 0;
    }
    memset( buf, buflen, 0 );			// reset buffer
    file.readBlock( buf, buflen );
    const char *format = 0;
    if ( file.status() == IO_Ok ) {		// file read ok
	register QImageHandler *p = &image_handlers[0];
	for ( int i=0; i<num_image_handlers; i++ ) {
	    if ( matchBytes(buf,p->header) ) {	// try match with headers
		format = p->format;
		break;
	    }
	    p++;
	}
    }
    file.close();
    return format;
}


bool QImage::load( const char *fileName, const char *format )
{
    if ( format == 0 ) {			// auto-detech format
	format = imageType( fileName );
	if ( !format )
	    return FALSE;
    }
    else {
	if ( strcmp(format,imageType(fileName)) != 0 )
	    return FALSE;			// format does not match
    }
    QImageHandler *h = get_image_handler( format );
    if ( !h ) {
#if defined(CHECK_RANGE)
	warning( "QImage::load: No such image format handler: %s", format );
#endif
	return FALSE;
    }
    QFile file( fileName );
    int fmode = IO_ReadOnly;
    if ( strcmp(format,"XBM")==0 )
	fmode |= IO_Translate;
    if ( !file.open(fmode) )			// cannot open file
	return FALSE;
    QImageIO io;
    io.status = 1;
    io.format = format;
    io.iodev  = &file;
    io.length = file.size();
    io.fname  = fileName;
    (*h->read_image)( &io );
    file.close();
    if ( io.status == 0 )			// image successfully read
	createImage( &io );
    return io.status == 0;
}


bool QImage::save( const char *fileName, const char *format ) const
{
    if ( format == 0 )				// set default format
	format = "QT";
    QImageHandler *h = get_image_handler( format );
    if ( !h ) {
#if defined(CHECK_RANGE)
	warning( "QImage::save: No such image format handler: %s", format );
#endif
	return FALSE;
    }
    if ( isNull() )				// null image/nothing to save
	return FALSE;
    QFile file( fileName );
    int fmode = IO_WriteOnly;
    if ( strcmp(format,"XBM")==0 )
	fmode |= IO_Translate;
    if ( !file.open(fmode) )
	return FALSE;				// could not create file
    QImageIO io;
    data->pm->getPixMap( &io );
    io.status = 1;
    io.format = format;
    io.iodev  = &file;
    io.fname  = fileName;
    (*h->write_image)( &io );
    file.close();
    return io.status == 0;			// image successfully written?
}


// --------------------------------------------------------------------------
// QImage stream functions
//

QDataStream &operator<<( QDataStream &s, const QImage &image )
{
    QImageIO io;
    if ( image.data->pm )
	image.data->pm->getPixMap( &io );
    else					// no data
	io.bits = 0;
    io.iodev  = s.device();
    io.fname  = 0;
    io.length = -1;
    io.params = 0;
    io.descr  = 0;
    write_qt_image( &io );
    return s;
}

QDataStream &operator>>( QDataStream &s, QImage &image )
{
    QImageIO io;
    io.status = 0;				// initialize image IO
    io.width  = -1;
    io.height = -1;
    io.depth  = -1;
    io.ctbl   = 0;
    io.bits   = 0;
    io.iodev  = s.device();
    io.fname  = 0;
    read_qt_image( &io );
    return s;
}


// --------------------------------------------------------------------------
// Misc. utility functions
//

static QString fbname( const char *fileName )	// get file basename
{
    QString s = fileName;
    if ( s.isNull() )
	s = "dummy";
    else {
	int i;
	if ( (i=s.findRev('/')) >= 0 )
	    s = &s[i];
	if ( (i=s.findRev('\\')) >= 0 )
	    s = &s[i];
	if ( (i=s.find('.')) >= 0 )
	    s.truncate( i );
    }
    return s;
}


// --------------------------------------------------------------------------
// Qt image read/write functions
//

static void read_qt_image( QImageIO * )		// read Qt image data
{
    debug( "READING QT" );
}


static void write_qt_image( QImageIO * )	// write Qt image data
{
    debug( "WRITING QT" );
}


// --------------------------------------------------------------------------
// GIF image read/write functions
//

static void read_gif_image( QImageIO *image )	// read GIF image data
{
    debug( "READING GIF" );
}


static void write_gif_image( QImageIO *image )	// write GIF image data
{
    debug( "WRITING GIF" );
}


// --------------------------------------------------------------------------
// BMP (DIB) image read/write functions
//

struct BMP_FILE {				// BMP file header
    char   bfType[2];				// "BM"
    INT32  bfSize;				// size of file
    INT16  bfReserved1;
    INT16  bfReserved2;
    INT32  bfOffBits;				// pointer to the pixmap bits
};

QDataStream &operator>>( QDataStream &s, BMP_FILE &bf )
{						// read file header
    s.readRawBytes( bf.bfType, 2 );
    s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
}


const  BMP_OLD = 12;				// old Windows/OS2 BMP size
const  BMP_WIN = 40;				// new Windows BMP size
const  BMP_OS2 = 64;				// new OS/2 BMP size

const  BMP_RGB  = 0;				// no compression
const  BMP_RLE8 = 1;				// run-length encoded, 8 bits
const  BMP_RLE4 = 2;				// run-length encoded, 4 bits

struct BMP_INFO {				// BMP information header
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


QDataStream &operator>>( QDataStream &s, BMP_INFO &bi )
{
    s >> bi.biSize;
    if ( bi.biSize == BMP_WIN || bi.biSize == BMP_OS2 ) {
	s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
	s >> bi.biCompression >> bi.biSizeImage;
	s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
	s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {
	INT16 w, h;
	s >> w >> h >> bi.biPlanes >> bi.biBitCount;
	bi.biWidth  = w;
	bi.biHeight = h;
	bi.biCompression = BMP_RGB;		// no compression
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
	bi.biClrUsed = bi.biClrImportant = 0;
    }
}


static void read_bmp_image( QImageIO *image )	// read BMP image data
{
    QIODevice  *d = image->iodev;
    QDataStream s( d );
    BMP_FILE	bf;
    BMP_INFO	bi;
    
    image->status = 1;				// assume format error
    s.setByteOrder( QDataStream::LittleEndian );// Intel
    s >> bf;					// read BMP header
    if ( strncmp(bf.bfType,"BM",2) != 0 )	// not a BMP image
	return;

    s >> bi;					// read BMP info
    if ( s.eos() )				// end of stream/file
	return;
#if 1
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
	return;					// invalid BMP image

    if ( t != BMP_OLD )				// jump to start of colormap
	d->at( bi.biSize + 14 );

    int    ncols;
    ulong *c;
    if ( nbits == 24 ) {			// there is no colormap
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
	    c[i] = QImageData::setRGB(rgb[2],rgb[1],rgb[0]);
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
    image->depth  = nbits == 24 ? 24 : 8;
    image->allocBits();

    d->at( bf.bfOffBits );			// start of image data
    ASSERT( comp == BMP_RGB );

    if ( nbits == 1 ) {				// 1 bit BMP image
	debug( "BMP: 1 bit images not yet supported" );
    }
    else if ( nbits == 4 ) {			// 4 bit BMP image
	int padw = ((w+7)/8)*8;
	for ( int i=h-1; i>=0; i-- ) {
	    uchar *p = image->bits[i];
	    int nybnum;
	    uchar x;
	    for ( int j=nybnum=0; j<padw; j++,nybnum++ ) {
		if ( (nybnum & 1) == 0 ) {	// read next byte
		    s >> x;
		nybnum = 0;
		}
		if ( j < w ) {
		    *p++ = x >> 4;
		    x <<= 4;
		}
	    }
	}
    }
    else if ( nbits == 8 ) {			// 8 bit BMP image
	int  padlen = ((w+3)/4)*4 - w;
	char padbuf[8];
	while ( --h >= 0 ) {
	    if ( d->readBlock((char *)image->bits[h],w) != w )
		break;
	    if ( padlen )
		d->readBlock( padbuf, padlen );
	}
    }
    else if ( nbits == 24 ) {			// 24 bit BMP image
	int  padlen = (4 - ((w*3) % 4)) & 0x03;
	char padbuf[8];
	while ( --h >= 0 ) {
	    uchar *p = image->bits[h];
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
    debug( "BMP IMAGE OK!!!" );
}


static void write_bmp_image( QImageIO *image )	// write BMP image data
{
    debug( "WRITING BMP" );
}


// --------------------------------------------------------------------------
// PBM/PGM/PPM (ASCII and RAW) image read/write functions
//

static int read_pbm_int( QIODevice *d )		// read int, skip comments
{
    int   c;
    int   val = -1;
    bool  skip = FALSE;				// skip comments
    while ( TRUE ) {
	if ( (c=d->getch()) == EOF )		// end of file
	    break;
	if ( val != -1 && !isdigit(c) )
	    break;
	if ( skip && c != '\n' )		// skip to end of line
	    continue;
	if ( c == '\n' )			// newline
	    skip = FALSE;
	else if ( c == '#' )			// start of comment
	    skip = TRUE;
	else if ( isdigit(c) )			// 0..9
	    val = val == -1 ? (c - '0') : (10*val + c - '0');
	else if ( !isspace(c) )
	    break;
    }
    return val;
}


static void read_pbm_image( QImageIO *image )	// read PBM image data
{
    const       buflen = 300;
    char        buf[buflen];
    QRegExp     r1, r2;
    QIODevice  *d = image->iodev;
    int	      	w, h, nbits, mcc;
    char	type;
    bool	raw;

    image->status = 1;				// assume format error
    d->readBlock( buf, 3 );			// read P[1-6].
    if ( !(buf[0] == 'P' && isdigit(buf[1]) && isspace(buf[2])) )
	return;
    switch ( (type=buf[1]) ) {
	case '1':				// ascii PBM
	case '4':				// raw PBM
	    setup_bitflip();	//!!!		// we will have to flip bits
	    nbits = 1;
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
	return;					// format error

    ASSERT( mcc <= 255 );
    image->width  = w;				// set image data
    image->height = h;
    image->depth  = nbits;
    image->allocBits();

  // NOTE!!! The following code assumes contigous memory!!!

    if ( raw ) {				// read raw data
	d->readBlock( (char *)image->bits[0], image->nBytes() );
    }
    else {					// read ascii data
	register uchar *p = image->bits[0];
	long n = image->nBytes();
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
	register uchar *p = image->bits[0];
	long n = image->nBytes();
	while ( n-- ) {				// to be removed
	    *p = bitflip[*p];
	    p++;
	}
	c = new ulong[nc=2];
	CHECK_PTR( c );
	c[0] = QImageData::setRGB(255,255,255);	// white
	c[1] = QImageData::setRGB(0,0,0);	// black
    }
    else if ( nbits == 8 ) {			// graymap
	c = new ulong[nc=mcc];			// !!! not correct
	CHECK_PTR( c );
	for ( int i=0; i<mcc; i++ )
	    c[i] = QImageData::setRGB(i*255/mcc,i*255/mcc,i*255/mcc);
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
    debug( "WRITING PBM" );
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
    QIODevice  *d = image->iodev;
    int	        i;
    int	      	w=-1, h=-1;

    image->status = 1;				// assume format error
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
    CHECK_PTR( c );
    c[0] = QImageData::setRGB(255,255,255);	// white
    c[1] = QImageData::setRGB(0,0,0);		// black
    image->width  = w;				// set image data
    image->height = h;
    image->depth  = 1;
    image->ncols  = 2;
    image->ctbl   = c;
    image->allocBits();
//!!!    image->bitOrder = BigEndian;
    image->bigend = TRUE;			// does not matter, really

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
    QIODevice *d = image->iodev;
    int w = image->width, h = image->height, depth = image->depth, i;
    QString s = fbname(image->fname);		// get file base name
    char buf[100];
    sprintf( buf, "#define %s_width %d\n",  (char *)s, w );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "#define %s_height %d\n", (char *)s, h );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "static char %s_bits[] = {\n ", (char *)s );
    d->writeBlock( buf, strlen(buf) );
    int nbytes = ((w+7)/8)*h;			// number of bytes
    if ( depth == 1 ) {
    }
    else if ( depth == 8 ) {
    }
    else {
    }
    char hexrep[16];
    for ( i=0; i<10; i++ )
	hexrep[i] = '0' + i;
    for ( i=10; i<16; i++ )
	hexrep[i] = 'a' -10 + i;
    int bcnt = 0;
    register char *p = buf;
    uchar *b = image->bits[0];
    int x=0, y=0;
    while ( nbytes-- ) {		// write all bytes
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
    image->status = 0;
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
    const       buflen = 200;
    char        buf[buflen];
    char       *p = buf;
    QRegExp     r = "/\\*.XPM.\\*/";    
    QIODevice  *d = image->iodev;
    int	        c, i;
    int	      	cpp, ncols, w, h;

    image->status = 1;				// assume format error
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
}


static void write_xpm_image( QImageIO *image )	// write XPM image data
{
    QIODevice *d = image->iodev;
    int w = image->width, h = image->height, depth = image->depth, i;
    QString s = fbname(image->fname);		// get file base name
    char buf[100];
}
