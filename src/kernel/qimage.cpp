/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.cpp#3 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qimage.cpp#3 $";
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

QImageData::QImageData()
{
    spec  = Any;
    width = height = depth = ncols = -1;
    carr  = 0;
    bits  = 0;
    bigend = FALSE;	/* TODO!!! How to handle this??? */
}

QImageData::~QImageData()
{
    delete carr;
    delete bits;
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
	QImage::defineIOHandler( "GIF", "^GIF",
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
    if ( data->deref() )
	delete data;
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
	QPixMap *pm = new QPixMap( w, h );	// create new pixmap
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

bool QImage::trueColor() const
{
    debug( "NOT SUPPORTED YET" );
    return FALSE;
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
    io.carr   = 0;
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

static void read_bmp_image( QImageIO *image )	// read BMP image data
{
    debug( "READING BMP" );
}


static void write_bmp_image( QImageIO *image )	// write BMP image data
{
    debug( "WRITING BMP" );
}


// --------------------------------------------------------------------------
// PBM/PGM/PPM image read/write functions
//

static void read_pbm_image( QImageIO *image )	// read PBM image data
{
    debug( "READING BMP" );
}


static void write_pbm_image( QImageIO *image )	// write PBM image data
{
    debug( "WRITING BMP" );
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
    const       buflen = 260;
    char        buf[buflen];
    QRegExp     r1, r2;
    QIODevice  *d = image->iodev;
    int	        i;
    int	      	w=-1, h=-1;

    image->status = 1;				// assume format error
    r1 = "^#define[\x20\t]+[a-zA-Z0-9_]+[\x20\t]+";
    r2 = "[0-9]+";
    d->readLine( buf, buflen );			// "#define .._width <num>" ?
    if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i ) {
	w = atoi( &buf[i] );
	d->readLine( buf, buflen );		// "#define .._height <num>" ?
	if ( r1.match(buf,0,&i)==0 && r2.match(buf,i)==i )
	    h = atoi( &buf[i] );
    }
    if ( w < 0 || h < 0 )			// invalid data
	return;

    while ( TRUE ) {
	if ( d->readLine(buf, buflen) == 0 )	// end of file
	    return;
	if ( strstr(buf,"0x") != 0 )		// does line contain data?
	    break;
    }

    int	bitflip[256];				// create bitflip array
    for ( i=0; i<256; i++ )
#if 0
        bitflip[i] = ((i >> 7) & 0x01) | ((i >> 5) & 0x02) |
	    	     ((i >> 3) & 0x04) | ((i >> 1) & 0x08) |
	    	     ((i << 7) & 0x80) | ((i << 5) & 0x40) |
	    	     ((i << 3) & 0x20) | ((i << 1) & 0x10);
#else
        bitflip[i] = i;
#endif
    int    nbytes = ((w+7)/8)*h;
    uchar *bits = new uchar[nbytes];
    uchar *b = bits;
    char  *p = buf;
    CHECK_PTR( bits );
    memset( bits, nbytes, 0 );    

    while ( nbytes-- ) {			// for all encoded bytes...
	p = strstr( p, "0x" );
	while ( !p ) {				// read another line	
	    if ( !d->readLine(buf, buflen) ) {
		delete bits;
		return;				// end of file
	    }
	    p = strstr( buf, "0x" ); 
	}
	p++;
	p++;
	*b++ = bitflip[hex2byte(p)];
	p++;
	p++;
    }

    ulong *c = new ulong[2];			// create colormap array
    CHECK_PTR( c );
    c[0] = QImage::setRGB(255,255,255);		// white
    c[1] = QImage::setRGB(0,0,0);		// black
    image->width  = w;				// set image data
    image->height = h;
    image->depth  = 1;
    image->ncols  = 2;
    image->carr   = c;
    image->bits   = bits;
    image->bigend = TRUE;			// does not matter, really
    image->status = 0;
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
    uchar *b = image->bits;
    while ( nbytes-- ) {		// write all bytes
	*p++ = '0';  *p++ = 'x';
	*p++ = hexrep[*b >> 4];
	*p++ = hexrep[*b++ & 0xf];
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
