/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.cpp#67 $
**
** Implementation of QImage and QImageIO classes
**
** Author  : Haavard Nord
** Created : 950207
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 QIMAGE_C
#include "qimage.h"
#include "qregexp.h"
#include "qfile.h"
#include "qdstream.h"
#include "qlist.h"
#include "qintdict.h"
#include <stdlib.h>
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qimage.cpp#67 $");


/*----------------------------------------------------------------------------
  \class QImage qimage.h
  \brief The QImage class provides a hardware-independent pixmap representation
  with direct access to the pixel data.

  The direct pixel access functionality of QImage makes it very suitable
  for image processing and for pixmap archiving.

  An image has the parameters \link width() width\endlink, \link height()
  height\endlink and \link depth() depth\endlink (bits per pixel, bpp), a
  color table and the actual \link bits() pixels\endlink.  QImage supports
  1-bpp, 8-bpp and 32-bpp image data.  1-bpp and 8-bpp images use a color
  lookup table; the pixel value is a color table index.

  32-bpp images encode an RGB value in 24 bits and ignore the color table.
  The most significant byte is reserved for alpha channel support in a
  future version of Qt.

  An entry in the color table is an RGB triplet encoded as \c uint.  Use
  the qRed, qGreen and qBlue functions (qcolor.h) to access the
  components, and qRgb to make an RGB triplet (see the QColor class
  documentation).

  1-bpp (monochrome) images have a color table with maximum 2 colors.
  There are two different formats; big endian (MSB first) or little endian
  (LSB first) bit order. To access a single bit, you will have to do some
  bitshifts:

  \code
    QImage image;
      // sets bit at (x,y) to 1
    if ( image.bitOrder() == QImage::LittleEndian )
	*(image.scanLine(y) + (x >> 3)) |= 1 << (x & 7);
    else
	*(image.scanLine(y) + (x >> 3)) |= 1 << (7 -(x & 7));
  \endcode

  If this looks complicated, it might be a good idea to convert the 1-bpp
  image to an 8-bpp image using convertDepth().

  8-bpp images are much easier to work with than 1-bpp images because they
  have a single byte per pixel:

  \code
    QImage image;
      // set entry 19 in the color table to yellow
    image.setColor( 19, qRgb(255,255,0) );
      // set 8 bit pixel at (x,y) to value yellow (in color table)
    *(image.scanLine(y) + x) = 19;
  \endcode

  32-bpp images ignore the color table, instead each pixel contains the
  RGB triplet. 24 bits contain the RGB value and the most significant
  byte is reserved for alpha channel.

  \code
    QImage image;
      // sets 32 bit pixel at (x,y) to yellow.
    uint *p = (uint *)image.scanLine(y) + x;
    *p = qRgb(255,255,0);
  \endcode

  The scanlines are 32-bit aligned for all depths.

  The QImage class uses explicit \link shclass.html sharing\endlink,
  similar to that of QArray and QString.

  \sa QImageIO, QPixmap, \link shclass.html Shared Classes\endlink
 ----------------------------------------------------------------------------*/


extern bool qt_image_native_bmp();

#if defined(_CC_DEC_) && defined(__alpha) && (__DECCXX_VER >= 50190001)
#pragma message disable narrowptr
#endif


/*****************************************************************************
  QImage member functions
 *****************************************************************************/

static bool  bitflip_init = FALSE;
static uchar bitflip[256];			// table to flip bits

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

uchar *qt_get_bitflip_array()			// called from QPixmap code
{
    setup_bitflip();
    return bitflip;
}


/*----------------------------------------------------------------------------
  Constructs a null image.
  \sa isNull()
 ----------------------------------------------------------------------------*/

QImage::QImage()
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
}

/*----------------------------------------------------------------------------
  Constructs an image with \e w width, \e h height, \e depth bits per
  pixel, \e numColors colors and bit order \e bitOrder.

  Using this constructor is the same as first constructing a null image and
  then calling the create() function.

  \sa create()
 ----------------------------------------------------------------------------*/

QImage::QImage( int w, int h, int depth, int numColors,	Endian bitOrder )
{
    data = new QImageData;
    CHECK_PTR( data );
    init();
    create( w, h, depth, numColors, bitOrder );
}

/*----------------------------------------------------------------------------
  Constructs a shallow copy of \e image.
 ----------------------------------------------------------------------------*/

QImage::QImage( const QImage &image )
{
    data = image.data;
    data->ref();
}

/*----------------------------------------------------------------------------
  Destroys the image and cleans up.
 ----------------------------------------------------------------------------*/

QImage::~QImage()
{
    if ( data && data->deref() ) {
	reset();
	delete data;
    }
}


/*----------------------------------------------------------------------------
  Assigns a shallow copy of \e image to this image and returns a reference
  to this image.

  \sa copy()
 ----------------------------------------------------------------------------*/

QImage &QImage::operator=( const QImage &image )
{
    image.data->ref();				// avoid 'x = x'
    if ( data->deref() ) {
	reset();
	delete data;
    }
    data = image.data;
    return *this;
}

/*----------------------------------------------------------------------------
  Sets the image bits to the \e pixmap contents and returns a reference to
  the image.

  If the image shares data with other images, it will first dereference
  the shared data.

  Makes a call to QPixmap::convertToImage().
 ----------------------------------------------------------------------------*/

QImage &QImage::operator=( const QPixmap &pixmap )
{
    *this = pixmap.convertToImage();
    return *this;
}

/*----------------------------------------------------------------------------
  Detaches from shared image data and makes sure that this image is the
  only one referring the data.

  If multiple images share common data, this image makes a copy of the
  data and detaches itself from the sharing mechanism.	Nothing is
  done if there is just a single reference.
 ----------------------------------------------------------------------------*/

void QImage::detach()
{
    if ( data->count != 1 )
	*this = copy();
}

/*----------------------------------------------------------------------------
  Returns a deep copy of the image.
 ----------------------------------------------------------------------------*/

QImage QImage::copy() const
{
    QImage image;
    if ( !isNull() ) {
	image.create( width(), height(), depth(), numColors(), bitOrder() );
	memcpy( image.bits(), bits(), numBytes() );
	memcpy( image.colorTable(), colorTable(), numColors()*sizeof(QRgb) );
    }
    return image;
}


/*----------------------------------------------------------------------------
  \fn bool QImage::isNull() const
  Returns TRUE if it is a null image.

  A null image has all parameters set to zero and no allocated data.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn int QImage::width() const
  Returns the width of the image.
  \sa heigth(), size(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImage::height() const
  Returns the height of the image.
  \sa width(), size(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QSize QImage::size() const
  Returns the size of the image.
  \sa width(), height(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QRect QImage::rect() const
  Returns the enclosing rectangle (0,0,width(),height()) of the image.
  \sa width(), height(), size()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImage::depth() const
  Returns the depth of the image.

  The image depth is the number of bits used to encode a single pixel, also
  called bits per pixel (bpp) or bit planes of an image.

  The supported depths are 1, 8 and 32.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImage::numColors() const
  Returns the size of the color table for the image.

  Notice that numColors() returns 0 for 32-bpp images, since these images
  do not use color tables, but instead encode pixel values as RGB triplets.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QImage::Endian QImage::bitOrder() const
  Returns the bit order for the image.

  If it is a 1-bit image, this function returns either QImage::BigEndian or
  QImage::LittleEndian.

  If it is not a 1-bit image, this function returns QImage::IgnoreEndian.

  \sa depth()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn uchar **QImage::jumpTable() const
  Returns a pointer to the scanline pointer table.

  This is the beginning of the data block for the image.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QRgb *QImage::colorTable() const
  Returns a pointer to the color table.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImage::numBytes() const
  Returns the number of bytes occupied by the image data.
  \sa bytesPerLine()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImage::bytesPerLine() const
  Returns the number of bytes per image scanline.
  This is equivalent to numBytes()/height().
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Returns the color in the color table at index \e i.

  A color value is an RGB triplet. Use the QRED, QGREEN and QBLUE functions
  (defined in qcolor.h) to get the color value components.

  \sa setColor(), QColor
 ----------------------------------------------------------------------------*/

QRgb QImage::color( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warning( "QImage::color: Index %d out of range", i );
#endif
    return data->ctbl ? data->ctbl[i] : (QRgb)-1;
}

/*----------------------------------------------------------------------------
  Sets a color in the color table at index \e i to \e c.

  A color value is an RGB triplet.  Use the qRgb function (defined in qcolor.h)
  to make RGB triplets.

  \sa color()
 ----------------------------------------------------------------------------*/

void QImage::setColor( int i, QRgb c )
{
#if defined(CHECK_RANGE)
    if ( i >= data->ncols )
	warning( "QImage::setColor: Index %d out of range", i );
#endif
    if ( data->ctbl )
	data->ctbl[i] = c;
}

/*----------------------------------------------------------------------------
  Returns a pointer to the pixel data at the \e i'th scanline.

  The scanline data is aligned on a 32-bit boundary.

  \warning If you are accessing 32-bpp image data, cast the returned
  pointer to \c uint* and use it to read/write the pixel value. You cannot
  use the \c uchar* pointer directly, because the pixel format depends on
  the byte order on the underlying platform. Hint: use \link ::qRgb()
  qRgb()\endlink and friends (qcolor.h) to access the pixels.

  \sa bits()
 ----------------------------------------------------------------------------*/

uchar *QImage::scanLine( int i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= data->h )
	warning( "QImage::scanLine: Index %d out of range", i );
#endif
    return data->bits ? data->bits[i] : 0;
}

/*----------------------------------------------------------------------------
  \fn uchar *QImage::bits() const
  Returns a pointer to the first pixel data. Equivalent to scanLine(0).
  \sa scanLine()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Resets all image parameters and deallocates the image data.
 ----------------------------------------------------------------------------*/

void QImage::reset()
{
    freeBits();
    setNumColors( 0 );
    data->w = data->h = data->d = 0;
    data->nbytes = 0;
    data->bitordr = IgnoreEndian;
}


/*----------------------------------------------------------------------------
  Determines the host computer byte order.
  Returns QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).
 ----------------------------------------------------------------------------*/

QImage::Endian QImage::systemByteOrder()
{
    static Endian sbo = IgnoreEndian;
    if ( sbo == IgnoreEndian ) {		// initialize
	int  ws;
	bool be;
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
#if defined(_OS_WIN32_)
#undef open					// kill utterly stupid #defines
#undef close
#undef read
#undef write
#endif
#endif

/*----------------------------------------------------------------------------
  Determines the bit order of the display hardware.
  Returns QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).
 ----------------------------------------------------------------------------*/

QImage::Endian QImage::systemBitOrder()
{
#if defined(_WS_X11_)
    return BitmapBitOrder(qt_xdisplay()) == MSBFirst ? BigEndian :LittleEndian;
#else
    return BigEndian;
#endif
}


/*----------------------------------------------------------------------------
  Resizes the color table to \e numColors colors.

  If the color table is expanded, then all new colors will be set to black
  (RGB 0,0,0).

  \sa color(), setColor()
 ----------------------------------------------------------------------------*/

void QImage::setNumColors( int numColors )
{
    if ( numColors == data->ncols )
	return;
    if ( numColors == 0 ) {			// use no color table
	if ( data->ctbl ) {
	    free( data->ctbl );
	    data->ctbl = 0;
	}
	data->ncols = 0;
	return;
    }
    if ( data->ctbl ) {				// already has color table
	data->ctbl = (QRgb*)realloc( data->ctbl, numColors*sizeof(QRgb) );
	if ( data->ctbl && numColors > data->ncols )
	    memset( (char *)&data->ctbl[data->ncols], 0,
		    (numColors-data->ncols)*sizeof(QRgb) );
    }
    else					// create new color table
	data->ctbl = (QRgb*)calloc( numColors*sizeof(QRgb), 1 );
    data->ncols = data->ctbl == 0 ? 0 : numColors;
}


/*----------------------------------------------------------------------------
  Sets the image width, height, depth, number of colors and bit order.
  Returns TRUE if successful, or FALSE if the parameters are incorrect or
  if memory cannot be allocated.

  The \e width and \e height is limited to 32767. \e depth must be 1, 8 or
  32. If \e depth is 1, then \e bitOrder must be set to either
  QImage::LittleEndian or QImage::BigEndian.  For other depths, \e
  bitOrder must be QImage::IgnoreEndian.

  This function allocates a color table and a buffer for the image data.
  The image data is filled with the pixel value 0.

  The image buffer is allocated as a single block that consists of a table
  of \link scanLine() scanline\endlink pointers (jumpTable()) and the
  image data (bits()).

  \sa width(), height(), depth(), numColors(), bitOrder(), jumpTable(),
  scanLine(), bits(), bytesPerLine(), numBytes()
 ----------------------------------------------------------------------------*/

bool QImage::create( int width, int height, int depth, int numColors,
		     QImage::Endian bitOrder )
{
    reset();					// reset old data
    if ( width <= 0 || height <= 0 || depth <= 0 || numColors < 0 )
	return FALSE;				// invalid parameter(s)
    if ( depth == 1 && bitOrder == IgnoreEndian ) {
#if defined(CHECK_RANGE)
	warning( "QImage::create: Bit order is required for 1 bpp images" );
#endif
	return FALSE;
    }
    if ( depth != 1 )
	bitOrder = IgnoreEndian;

#if defined(DEBUG)
    if ( depth == 24 )
	warning( "QImage::create: 24-bpp images no longer supported, "
		 "use 32-bpp instead" );
#endif
    switch ( depth ) {
	case 1:
	case 8:
	case 32:
	    break;
	default:				// invalid depth
	    return FALSE;
    }

    setNumColors( numColors );
    if ( data->ncols != numColors )		// could not alloc color table
	return FALSE;

    int bpl    = ((width*depth+31)/32)*4;	// bytes per scanline
    int pad    = bpl - (width*depth)/8;		// pad with zeros
    int nbytes = bpl*height;			// image size
    int ptbl   = height*sizeof(uchar*);		// pointer table size
    int size   = nbytes + ptbl;			// total size of data block
    uchar **p  = (uchar **)malloc( size );	// alloc image bits
    if ( !p ) {					// no memory
	setNumColors( 0 );
	return FALSE;
    }
    data->w = width;
    data->h = height;
    data->d = depth;
    data->nbytes  = nbytes;
    data->bitordr = bitOrder;
    data->bits = p;				// set image pointer
    uchar *d = (uchar*)p + ptbl;		// setup scanline pointers
    while ( height-- ) {
	*p++ = d;
	if ( pad )
	    memset( d+bpl-pad, 0, pad );
	d += bpl;
    }
    return TRUE;
}


/*----------------------------------------------------------------------------
  \internal
  Initializes the image data structure.
 ----------------------------------------------------------------------------*/

void QImage::init()
{
    data->w = data->h = data->d = data->ncols = 0;
    data->nbytes = 0;
    data->ctbl = 0;
    data->bits = 0;
    data->bitordr = QImage::IgnoreEndian;
}

/*----------------------------------------------------------------------------
  \internal
  Deallocates the image data and sets the bits pointer to 0.
 ----------------------------------------------------------------------------*/

void QImage::freeBits()
{
    if ( data->bits ) {				// dealloc image bits
	free( data->bits );
	data->bits = 0;
    }
}


/*****************************************************************************
  Internal routines for converting image depth.
 *****************************************************************************/

//
// convert_32_to_8:  Converts a 32 bits depth (true color) to an 8 bit
// image with a colormap.  If the 32 bit image has more than 256 colors,
// we convert the red,green and blue bytes into a single byte encoded
// as RRRGGGBB.
//

declare(QIntDictM,char);
declare(QIntDictIteratorM,char);

static bool convert_32_to_8( const QImage *src, QImage *dst )
{
    register QRgb *p;
    uchar  *b;
    bool    do_quant = FALSE;
    int	    y, x;

    if ( !dst->create(src->width(), src->height(), 8, 256) )
	return FALSE;

    QIntDictM(char) cdict( 21001 );		// dict of 24-bit RGB colors
    char *pixel=0, *pix;			// hack: use ptr as int value
    for ( y=0; y<src->height(); y++ ) {		// check if <= 256 colors
	p = (QRgb *)src->scanLine(y);
	b = dst->scanLine(y);
	x = src->width();
	while ( x-- ) {
	    if ( !(pix=cdict.find(*p)) ) {	// new RGB, insert in dict
		cdict.insert( *p, (pix=++pixel) );
		if ( cdict.count() > 256 ) {	// too many colors
		    do_quant = TRUE;
		    y = src->height();
		    break;
		}
	    }
	    *b++ = (uchar)((int)pix - 1);	// map RGB color to pixel
	    p++;
	}
    }
    int ncols = do_quant ? 256 : cdict.count();
    dst->setNumColors( ncols );
    if ( do_quant ) {				// quantization needed
	for ( int i=0; i<ncols; i++ )		// build 3+3+2 color table
	    dst->setColor( i, qRgb( ((i & 0xe0)*255 + 0x70) / 0xe0,
				    (((i << 3) & 0xe0)*255 + 0x70) / 0xe0,
				    (((i << 6) & 0xc0)*255 + 0x60) / 0xc0 ) );
	for ( y=0; y<src->height(); y++ ) {
	    p = (QRgb *)src->scanLine(y);
	    b = dst->scanLine(y);
	    int x = 0;
	    QRgb *end = p + src->width();
	    while ( p < end ) {			// perform fast quantization
		*b++ = (*p & 0xe0) | ((*p >> 11) & 0x1c) | ((*p >> 22) & 0x03);
		p++;
		x++;		
	    }
	    ASSERT( x == src->width() );
	    ASSERT( x == dst->width() );
	}
    } else {					// number of colors <= 256
	QIntDictIteratorM(char) cdictit( cdict );
	while ( cdictit.current() ) {		// build color table
	    dst->setColor( (int)(cdictit.current())-1, cdictit.currentKey() );
	    ++cdictit;
	}
    }
    return TRUE;
}


static bool convert_8_to_32( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 32) )
	return FALSE;				// create failed
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uint *p = (uint *)dst->scanLine(y);
	uchar  *b = src->scanLine(y);
	uint *end = p + dst->width();
	while ( p < end )
	    *p++ = src->color(*b++);
    }
    return TRUE;
}


static bool convert_1_to_32( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 32) )
	return FALSE;				// could not create
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uint *p = (uint *)dst->scanLine(y);
	uchar *b = src->scanLine(y);
	int x;
	if ( src->bitOrder() == QImage::BigEndian ) {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = src->color( (*b >> (7 - (x & 7))) & 1 );
		if ( (x & 7) == 7 )
		    b++;
	    }
	} else {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = src->color( (*b >> (x & 7)) & 1 );
		if ( (x & 7) == 7 )
		    b++;
	    }
	}
    }
    return TRUE;
}


static bool convert_1_to_8( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 8, 2) )
	return FALSE;				// something failed
    dst->setColor( 0, src->color(0) );		// copy color table
    dst->setColor( 1, src->color(1) );
    for ( int y=0; y<dst->height(); y++ ) {	// for each scan line...
	register uchar *p = dst->scanLine(y);
	uchar *b = src->scanLine(y);
	int x;
	if ( src->bitOrder() == QImage::BigEndian ) {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = (*b >> (7 - (x & 7))) & 1;
		if ( (x & 7) == 7 )
		    b++;
	    }
	} else {
	    for ( x=0; x<dst->width(); x++ ) {
		*p++ = (*b >> (x & 7)) & 1;
		if ( (x & 7) == 7 )
		    b++;
	    }
	}
    }
    return TRUE;
}


//
// dither_image:  Uses the Floyd-Steinberg error diffusion algorithm.
// Floyd-Steinberg dithering is a one-pass algorithm that moves errors
// rightwards.
//

static bool dither_image( const QImage *src, QImage *dst )
{
    if ( !dst->create(src->width(), src->height(), 1, 2, QImage::BigEndian) )
	return FALSE;				// something failed
    dst->setColor( 0, qRgb(255, 255, 255) );
    dst->setColor( 1, qRgb(  0,	  0,   0) );
    int	  w = src->width();
    int	  h = src->height();
    uchar gray[256];				// gray map for 8 bit images
    bool  use_gray = src->depth() == 8;
    if ( use_gray ) {				// make gray map
	for ( int i=0; i<src->numColors(); i++ )
	    gray[i] = qGray( src->color(i) );
    }
    int *line1 = new int[w];
    int *line2 = new int[w];
    int bmwidth = (w+7)/8;
    if ( !(line1 && line2) )
	return FALSE;
    register uchar *p;
    uchar *end;
    int *b1, *b2;
    int wbytes = w * (src->depth()/8);
    p = src->bits();
    end = p + wbytes;
    b2 = line2;
    if ( use_gray ) {				// 8 bit image
	while ( p < end )
	    *b2++ = gray[*p++];
    }
    else {					// 32 bit image
	while ( p < end ) {
	    *b2++ = qGray(*p);
	    p += 4;
	}
    }
    int x, y;
    for ( y=0; y<h; y++ ) {			// for each scan line...
	int *tmp = line1; line1 = line2; line2 = tmp;
	bool not_last_line = y < h - 1;
	if ( not_last_line ) {			// calc. grayvals for next line
	    p = src->scanLine(y+1);
	    end = p + wbytes;
	    b2 = line2;
	    if ( use_gray ) {			// 8 bit image
		while ( p < end )
		    *b2++ = gray[*p++];
	    }
	    else {				// 24 bit image
		while ( p < end ) {
		    *b2++ = qGray(*p);
		    p += 4;
		}
	    }
	}
	int err;
	p = dst->scanLine( y );
	memset( p, 0, bmwidth );
	b1 = line1;
	b2 = line2;
	int bit = 7;
	for ( x=1; x<=w; x++ ) {
	    if ( *b1 < 128 ) {			// black pixel
		err = *b1++;
		*p |= 1 << bit;
	    } else {				// white pixel
		err = *b1++ - 255;
	    }
	    if ( bit == 0 ) {
		p++;
		bit = 7;
	    } else {
		bit--;
	    }
	    if ( x < w )
		*b1 += (err*7)>>4;		// spread error to right pixel
	    if ( not_last_line ) {
		b2[0] += (err*5)>>4;		// pixel below
		if ( x > 1 )
		    b2[-1] += (err*3)>>4;	// pixel below left
		if ( x < w )
		    b2[1] += err>>4;		// pixel below right
	    }
	    b2++;
	}
    }
    delete [] line1;
    delete [] line2;
    return TRUE;
}


/*----------------------------------------------------------------------------
  Converts the depth (bpp) of the image to \e depth and returns the
  converted image.

  The \e depth argument must be 1, 8 or 32.

  Returns \c *this if \e depth is equal to the image depth, or a null
  image if this image cannot be converted.

  \sa depth(), isNull()
 ----------------------------------------------------------------------------*/

QImage QImage::convertDepth( int depth ) const
{
    QImage image;
    if ( (data->d == 8 || data->d == 32) && depth == 1 ) // dither
	dither_image( this, &image );
    else if ( data->d == 32 && depth == 8 )	// 32 -> 8
	convert_32_to_8( this, &image );
    else if ( data->d == 8 && depth == 32 )	// 8 -> 32
	convert_8_to_32( this, &image );
    else if ( data->d == 1 && depth == 8 )	// 1 -> 8
	convert_1_to_8( this, &image );
    else if ( data->d == 1 && depth == 32 )	// 1 -> 32
	convert_1_to_32( this, &image );
    else if ( data->d == depth )
	image = *this;				// no conversion
    else {
#if defined(CHECK_RANGE)
	if ( isNull() )
	    warning( "QImage::convertDepth: Image is a null image" );
	else
	    warning( "QImage::convertDepth: Depth %d not supported", depth );
#endif
    }
    return image;
}


/*----------------------------------------------------------------------------
  Converts the bit order of the image to \e bitOrder and returns the converted
  image.

  Returns \c *this if the \e bitOrder is equal to the image bit order, or a
  null image if this image cannot be converted.

  \sa bitOrder(), setBitOrder()
 ----------------------------------------------------------------------------*/

QImage QImage::convertBitOrder( QImage::Endian bitOrder ) const
{
    if ( isNull() || data->d != 1 ||		// invalid argument(s)
	 !(bitOrder == BigEndian || bitOrder == LittleEndian) ) {
	QImage nullImage;
	return nullImage;
    }
    if ( data->bitordr == bitOrder )		// nothing to do
	return *this;

    QImage image( data->w, data->h, 1, data->ncols, bitOrder );
    setup_bitflip();
    register uchar *p;
    uchar *end;
    uchar *b;
    p = bits();
    b = image.bits();
    end = p + numBytes();
    while ( p < end )
	*b++ = bitflip[*p++];
    memcpy( image.colorTable(), colorTable(), numColors()*sizeof(QRgb) );
    return image;
}


/*****************************************************************************
  Standard image io handlers (defined below)
 *****************************************************************************/

// standard image io handlers (defined below)
static void read_gif_image( QImageIO * ) NOT_USED_FN;
static void write_gif_image( QImageIO * ) NOT_USED_FN;
static void read_bmp_image( QImageIO * );
static void write_bmp_image( QImageIO * );
static void read_pbm_image( QImageIO * );
static void write_pbm_image( QImageIO * );
static void read_xbm_image( QImageIO * );
static void write_xbm_image( QImageIO * );
static void read_xpm_image( QImageIO * ) NOT_USED_FN;
static void write_xpm_image( QImageIO * ) NOT_USED_FN;


/*****************************************************************************
  Misc. utility functions
 *****************************************************************************/

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

static void swapPixel01( QImage *image )	// 1-bit: swap 0 and 1 pixels
{
    int i;
    if ( image->depth() == 1 && image->numColors() == 2 ) {
	register uint *p = (uint *)image->bits();
	int nbytes = image->numBytes();
	for ( i=0; i<nbytes/4; i++ ) {
	    *p = ~*p;
	    p++;
	}
	uchar *p2 = (uchar *)p;
	for ( i=0; i<(nbytes&3); i++ ) {
	    *p2 = ~*p2;
	    p2++;
	}
	QRgb t = image->color(0);		// swap color 0 and 1
	image->setColor( 0, image->color(1) );
	image->setColor( 1, t );
    }
}


/*****************************************************************************
  QImageIO member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \class QImageIO qimage.h

  \brief The QImageIO class contains parameters for loading
  and saving images.

  \ingroup images
  \ingroup files

  QImageIO contains a QIODevice object that is used for image data I/O.
  The programmer can install new image file formats in addition to those
  that Qt implements.

  Qt currently supports the following image file formats: BMP, XBM and PNM.
  The different PNM formats are: PBM (P1), PGM (P2), PPM (P3), PBMRAW (P4),
  PGMRAW (P5) and PPMRAW (P6).

  You will normally not need to use this class, QPixmap::load(),
  QPixmap::save() and QImage contain most of the needed functionality.

  \bug
  PNM files can only be read, not written.

  \sa QImage, QPixmap, QFile
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs a QImageIO object with all parameters set to zero.
 ----------------------------------------------------------------------------*/

QImageIO::QImageIO()
{
    iostat = 0;
    iodev  = 0;
    params = descr = 0;
}

/*----------------------------------------------------------------------------
  Constructs a QImageIO object with an I/O device and a format tag.
 ----------------------------------------------------------------------------*/

QImageIO::QImageIO( QIODevice *ioDevice, const char *format )
    : frmt(format)
{
    iostat = 0;
    iodev  = ioDevice;
    params = descr = 0;
}

/*----------------------------------------------------------------------------
  Constructs a QImageIO object with a file name and a format tag.
 ----------------------------------------------------------------------------*/

QImageIO::QImageIO( const char *fileName, const char *format )
    : frmt(format), fname(fileName)
{
    iostat = 0;
    iodev  = 0;
    params = descr = 0;
}

/*----------------------------------------------------------------------------
  Destroys the object an all related data.
 ----------------------------------------------------------------------------*/

QImageIO::~QImageIO()
{
    if ( params )
	delete [] params;
    if ( descr )
	delete [] descr;
}


/*****************************************************************************
  QImageIO image handler functions
 *****************************************************************************/

class QImageHandler
{
public:
    QImageHandler( const char *f, const char *h, bool tm,
		   image_io_handler r, image_io_handler w );
    QString	      format;			// image format
    QRegExp	      header;			// image header pattern
    bool	      text_mode;		// image I/O mode
    image_io_handler  read_image;		// image read function
    image_io_handler  write_image;		// image write function
};

QImageHandler::QImageHandler( const char *f, const char *h, bool t,
			      image_io_handler r, image_io_handler w )
    : format(f), header(h)
{
    text_mode	= t;
    read_image	= r;
    write_image = w;
}

typedef declare(QListM,QImageHandler) QIHList;	// list of image handlers
static QIHList *imageHandlers = 0;

static void cleanup_image_handlers()		// cleanup image handler list
{
    delete imageHandlers;
    imageHandlers = 0;
}

static void init_image_handlers()		// initialize image handlers
{
    if ( !imageHandlers ) {
	imageHandlers = new QIHList;
	CHECK_PTR( imageHandlers );
	imageHandlers->setAutoDelete( TRUE );
	qAddPostRoutine( cleanup_image_handlers );
//	QImageIO::defineIOHandler( "GIF", "^GIF[0-9][0-9][a-z]", 0,
//				   read_gif_image, write_gif_image );
	QImageIO::defineIOHandler( "BMP", "^BM", 0,
				   read_bmp_image, write_bmp_image );
	QImageIO::defineIOHandler( "PBM", "^P1", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PBMRAW", "^P4", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PGM", "^P2", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PGMRAW", "^P5", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PPM", "^P3", "T",
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PPMRAW", "^P6", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "PNM", "^P1", 0,
				   read_pbm_image, write_pbm_image );
	QImageIO::defineIOHandler( "XBM", "^#define", "T",
				   read_xbm_image, write_xbm_image );
//	QImageIO::defineIOHandler( "XPM", "/\\*.XPM.\\*/", "T",
//				   read_xpm_image, write_xpm_image );
    }
}

static QImageHandler *get_image_handler( const char *format )
{						// get pointer to handler
    if ( !imageHandlers )
	init_image_handlers();
    register QImageHandler *p = imageHandlers->first();
    while ( p ) {				// traverse list
	if ( p->format == format )
	    return p;
	p = imageHandlers->next();
    }
    return 0;					// no such handler
}

/*----------------------------------------------------------------------------
  Defines a image IO handler for a specified image format.
  An image IO handler is responsible for reading and writing images.

  \arg \e format is the name of the format.
  \arg \e header is a regular expression that recognizes the image header.
  \arg \e flags is "T" for text formats like PBM; generally you will
	  want to use 0.
  \arg \e read_image is a function to read an image of this format.
  \arg \e write_image is a function to write an image of this format.

  Both read_image and write_image are of type image_io_handler, which is
  a function pointer.

  Example:
  \code
    void readGIF( QImageIO *image )
    {
      // read the image, using the image->ioDevice()
    }

    void writeGIF( QImageIO *image )
    {
      // write the image, using the image->ioDevice()
    }

    // add the GIF image handler

    QImageIO::defineIOHandler( "GIF",
			       "^GIF[0-9][0-9][a-z]",
			       0,
			       read_gif_image,
			       write_gif_image );
  \endcode
 ----------------------------------------------------------------------------*/

void QImageIO::defineIOHandler( const char *format,
				const char *header,
				const char *flags,
				image_io_handler read_image,
				image_io_handler write_image )
{
    if ( !imageHandlers )
	init_image_handlers();
    QImageHandler *p;
    p = new QImageHandler( format, header, flags && *flags == 'T',
			   read_image, write_image );
    CHECK_PTR( p );
    imageHandlers->insert( 0, p );
}


/*****************************************************************************
  QImageIO normal member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \fn const QImage &QImageIO::image() const
  Returns the image currently set.
  \sa setImage()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QImageIO::status() const
  Returns the image IO status.	A non-zero value indicates an error, while 0
  means that the IO operation was successful.
  \sa setStatus()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const char *QImageIO::format() const
  Returns the image format string, or 0 if no format has been set.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QIODevice *QImageIO::ioDevice() const
  Returns the IO device currently set.
  \sa setIODevice()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const char *QImageIO::fileName() const
  Returns the file name currently set.
  \sa setFileName()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const char *QImageIO::parameters() const
  Returns image parameters string.
  \sa setParameters()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const char *QImageIO::description() const
  Returns the image description string.
  \sa setDescription()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Sets the image.
  \sa image()
 ----------------------------------------------------------------------------*/

void QImageIO::setImage( const QImage &image )
{
    im = image;
}

/*----------------------------------------------------------------------------
  Sets the image IO status.  A non-zero value indicates an error, while 0 means
  that the IO operation was successful.
  \sa status()
 ----------------------------------------------------------------------------*/

void QImageIO::setStatus( int status )
{
    iostat = status;
}

/*----------------------------------------------------------------------------
  Sets the image format name of the image about to be read or written.

  It is necessary to specify a format before writing an image.

  It is not necessary to specify a format before reading an image.
  If not format has been set, Qt guesses the image format before reading
  it.  If a format is set, but the image has another (valid) format,
  the image will not be read.

  \sa read(), write(), format()
 ----------------------------------------------------------------------------*/

void QImageIO::setFormat( const char *format )
{
    frmt = format;
}

/*----------------------------------------------------------------------------
  Sets the IO device to be used for reading or writing an image.

  Setting the IO device allows images to be read/written to any
  block-oriented QIODevice.

  If \e ioDevice is not null, this IO device will override file name
  settings.

  \sa setFileName()
 ----------------------------------------------------------------------------*/

void QImageIO::setIODevice( QIODevice *ioDevice )
{
    iodev = ioDevice;
}

/*----------------------------------------------------------------------------
  Sets the name of the file to read or write an image.
  \sa setIODevice()
 ----------------------------------------------------------------------------*/

void QImageIO::setFileName( const char *fileName )
{
    fname = fileName;
}

/*----------------------------------------------------------------------------
  Sets the image parameters string for image handlers that require
  special parameters.

  Although all image formats supported by Qt ignore the parameters string,
  it will be useful for future extentions or contributions (like JPEG).
 ----------------------------------------------------------------------------*/

void QImageIO::setParameters( const char *parameters )
{
    if ( params )
	delete [] params;
    params = qstrdup( parameters );
}

/*----------------------------------------------------------------------------
  Sets the image description string for image handlers that support image
  descriptions.

  Currently, no image format supported by Qt use the description string.
 ----------------------------------------------------------------------------*/

void QImageIO::setDescription( const char *description )
{
    if ( descr )
	delete [] descr;
    descr = qstrdup( description );
}


/*----------------------------------------------------------------------------
  Returns a string that specifies the image format of the file \e fileName,
  or null if the file cannot not be read or if the format is not recognized.
 ----------------------------------------------------------------------------*/

const char *QImageIO::imageFormat( const char *fileName )
{
    QFile file( fileName );
    if ( !file.open(IO_ReadOnly) )
	return 0;
    const char *format = imageFormat( &file );
    file.close();
    return format;
}

/*----------------------------------------------------------------------------
  Returns a string that specifies the image format of the image read from
  \e d, or null if the file cannot be read or if the format is not recognized.
 ----------------------------------------------------------------------------*/

const char *QImageIO::imageFormat( QIODevice *d )
{
    const int buflen = 14;
    char buf[buflen];
    if ( imageHandlers == 0 )
	init_image_handlers();
    int pos   = d->at();			// save position
    int rdlen = d->readBlock( buf, buflen );	// read a few bytes
    const char *format = 0;
    if ( d->status() == IO_Ok && rdlen == buflen ) {
	QImageHandler *p = imageHandlers->first();
	while ( p ) {
	    if ( p->header.match(buf) != -1 ) { // try match with headers
		format = p->format;
		break;
	    }
	    p = imageHandlers->next();
	}
    }
    d->at( pos );				// restore position
    return format;
}


/*----------------------------------------------------------------------------
  Reads an image into memory and returns TRUE if the image was successfully
  read.

  Before reading an image, you must set an IO device or a file name.
  If both an IO device and a file name has been set, then the IO device will
  be used.

  Setting the image file format string is optional.

  Example:

  \code
    QImageIO iio;
    QPixmap  pixmap;
    iio.setFileName( "burger.bmp" );
    if ( image.read() )			// ok
	pixmap = iio.image();		// convert to pixmap
  \endcode

  \sa setIODevice(), setFileName(), setFormat(), write(), QPixmap::load()
 ----------------------------------------------------------------------------*/

bool QImageIO::read()
{
    QFile	   file;
    const char	  *image_format;
    QImageHandler *h;

    if ( iodev ) {				// read from io device
	// ok, already open
    }
    else if ( !fname.isEmpty() ) {		// read from file
	file.setName( fname );
	if ( !file.open(IO_ReadOnly) )
	    return FALSE;			// cannot open file
	iodev = &file;
    }
    else					// no file name or io device
	return FALSE;
    image_format = imageFormat( iodev );	// get image format
    if ( !image_format ||
	 !(frmt.isEmpty() || frmt == image_format) ) {
	if ( file.isOpen() ) {			// unknown or wrong format
	    file.close();
	    iodev = 0;
	}
	return FALSE;
    }
    frmt = image_format;			// set format
    h = get_image_handler( image_format );
    if ( file.isOpen() ) {
#if !defined(UNIX)
	if ( h->text_mode ) {			// reopen in translated mode
	    file.close();
	    file.open( IO_ReadOnly | IO_Translate );
	}
	else
#endif
	    file.at( 0 );			// position to start
    }
    iostat = 1;					// assume error
    (*h->read_image)( this );
    if ( file.isOpen() ) {			// image was read using file
	file.close();
	iodev = 0;
    }
    return iostat == 0;				// image successfully read?
}


/*----------------------------------------------------------------------------
  Writes an image to an IO device and returns TRUE if the image was
  successfully written.

  Before writing an image, you must set an IO device or a file name.
  If both an IO device and a file name has been set, then the IO device will
  be used.

  The image will be written using the specified image format.

  Example:
  \code
    QImageIO iio;
    QImage   im;
    im = pixmap;				// convert to image
    iio.setImage( im );
    iio.setFileName( "burger.bmp" );
    iio.setFormat( "BMP" );
    iio.write();				// TRUE if ok
  \endcode

  \sa setIODevice(), setFileName(), setFormat(), read(), QPixmap::save()
 ----------------------------------------------------------------------------*/

bool QImageIO::write()
{
    if ( frmt.isEmpty() )
	return FALSE;
    QImageHandler *h = get_image_handler( frmt );
    if ( !h ) {
#if defined(CHECK_RANGE)
	warning( "QImageIO::write: No such image format handler: %s",
		 format() );
#endif
	return FALSE;
    }
    QFile file;
    if ( iodev )
	;
    else if ( !fname.isEmpty() ) {
	file.setName( fname );
	int fmode = h->text_mode ? IO_WriteOnly|IO_Translate : IO_WriteOnly;
	if ( !file.open(fmode) )		// couldn't create file
	    return FALSE;
	iodev = &file;
    }
    iostat = 1;
    (*h->write_image)( this );
    if ( file.isOpen() ) {			// image was written using file
	file.close();
	iodev = 0;
    }
    return iostat == 0;				// image successfully written?
}


/*****************************************************************************
  GIF image read/write functions

  The Graphical Interchange Format (c) is the Copyright property of
  CompuServe Incorporated.  GIF (sm) is a Service Mark propery of
  CompuServe Incorporated.
 *****************************************************************************/

#if 0
static void read_gif_image( QImageIO * )	// read GIF image data
{
    warning( "Qt: GIF not supported in this version" );
}


static void write_gif_image( QImageIO * )	// write GIF image data
{
    warning( "Qt: GIF not supported in this version" );
}
#endif


/*****************************************************************************
  BMP (DIB) image read/write functions
 *****************************************************************************/

const int BMP_FILEHDR_SIZE = 14;		// size of BMP_FILEHDR data

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


const int BMP_OLD  = 12;			// old Windows/OS2 BMP size
const int BMP_WIN  = 40;			// new Windows BMP size
const int BMP_OS2  = 64;			// new OS/2 BMP size

const int BMP_RGB  = 0;				// no compression
const int BMP_RLE8 = 1;				// run-length encoded, 8 bits
const int BMP_RLE4 = 2;				// run-length encoded, 4 bits

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


static void read_bmp_image( QImageIO *iio )	// read BMP image data
{
    QIODevice  *d = iio->ioDevice();
    QDataStream s( d );
    BMP_FILEHDR bf;
    BMP_INFOHDR bi;
    int		startpos = d->at();
    QImage	image;

    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    s >> bf;					// read BMP file header
    if ( strncmp(bf.bfType,"BM",2) != 0 )	// not a BMP image
	return;
    s >> bi;					// read BMP info header
    if ( d->atEnd() )				// end of stream/file
	return;
#if 0
    debug( "bfOffBits........%d", bf.bfOffBits );
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
    int w = bi.biWidth,	 h = bi.biHeight,  nbits = bi.biBitCount;
    int t = bi.biSize,	 comp = bi.biCompression;

    if ( !(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 24) ||
	 bi.biPlanes != 1 || comp > BMP_RLE4 )
	return;					// weird BMP image
    if ( !(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) ||
	   (nbits == 8 && comp == BMP_RLE8)) )
	 return;				// weird compression type

    int ncols;
    int depth;
    switch ( nbits ) {
	case 24:
	    depth = 32;
	    break;
	case 8:
	case 4:
	    depth = 8;
	    break;
	default:
	    depth = 1;
    }
    if ( depth == 32 )				// there's no colormap
	ncols = 0;
    else					// # colors used
	ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;

    image.create( w, h, depth, ncols, nbits == 1 ?
		  QImage::BigEndian : QImage::IgnoreEndian );
    if ( image.isNull() )			// could not create image
	return;

    d->at( startpos + BMP_FILEHDR_SIZE + bi.biSize ); // goto start of colormap

    if ( ncols > 0 ) {				// read color table
	uchar rgb[4];
	int   rgb_len = t == BMP_OLD ? 3 : 4;
	for ( int i=0; i<ncols; i++ ) {
	    d->readBlock( (char *)rgb, rgb_len );
	    image.setColor( i, qRgb(rgb[2],rgb[1],rgb[0]) );
	    if ( d->atEnd() )			// truncated file
		return;
	}
    }

    d->at( startpos + bf.bfOffBits );		// start of image data

    int      bpl = image.bytesPerLine();
    uchar **line = image.jumpTable();

    if ( nbits == 1 ) {				// 1 bit BMP image
	while ( --h >= 0 ) {
	    if ( d->readBlock((char*)line[h],bpl) != bpl )
		break;
	}
	if ( ncols == 2 && qGray(image.color(0)) < qGray(image.color(1)) )
	    swapPixel01( &image );		// pixel 0 is white!
    }

    else if ( nbits == 4 ) {			// 4 bit BMP image
	int    buflen = ((w+7)/8)*4;
	uchar *buf    = new uchar[buflen];
	CHECK_PTR( buf );
	if ( comp == BMP_RLE4 ) {		// run length compression
	    int x=0, y=0, b, c, i;
	    register uchar *p = line[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = line[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = line[h-y-1] + x;
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
		register uchar *p = line[h];
		uchar *b = buf;
		for ( int i=0; i<w/2; i++ ) {	// convert nibbles to bytes
		    *p++ = *b >> 4;
		    *p++ = *b++ & 0x0f;
		}
		if ( w & 1 )			// the last nibble
		    *p = *b >> 4;
	    }
	}
	delete [] buf;
    }

    else if ( nbits == 8 ) {			// 8 bit BMP image
	if ( comp == BMP_RLE8 ) {		// run length compression
	    int x=0, y=0, b;
	    register uchar *p = line[h-1];
	    while ( y < h ) {
		if ( (b=d->getch()) == EOF )
		    break;
		if ( b == 0 ) {			// escape code
		    switch ( (b=d->getch()) ) {
			case 0:			// end of line
			    x = 0;
			    y++;
			    p = line[h-y-1];
			    break;
			case 1:			// end of image
			case EOF:		// end of file
			    y = h;		// exit loop
			    break;
			case 2:			// delta (jump)
			    x += d->getch();
			    y += d->getch();
			    p = line[h-y-1] + x;
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
		    memset( p, d->getch(), b ); // repeat pixel
		    x += b;
		    p += b;
		}
	    }
	}
	else if ( comp == BMP_RGB ) {		// uncompressed
	    while ( --h >= 0 ) {
		if ( d->readBlock((char *)line[h],bpl) != bpl )
		    break;
	    }
	}
    }

    else if ( nbits == 24 ) {			// 24 bit BMP image
	register QRgb *p;
	QRgb  *end;
	uchar *buf24 = new uchar[bpl];
	int    bpl24 = ((w*24+31)/32)*4;
	uchar *b;
	while ( --h >= 0 ) {
	    p = (QRgb *)line[h];
	    end = p + w;
	    if ( d->readBlock( (char *)buf24,bpl24) != bpl24 )
		break;
	    b = buf24;
	    while ( p < end ) {
		*p++ = (b[0] << 16) | (b[1] << 8) | b[2];
		b += 3;
	    }
	}
	delete[] buf24;
    }

    iio->setImage( image );
    iio->setStatus( 0 );			// image ok
}


static void write_bmp_image( QImageIO *iio )	// write BMP image data
{
    QIODevice  *d = iio->ioDevice();
    QImage	image = iio->image();
    QDataStream s( d );
    BMP_FILEHDR bf;
    BMP_INFOHDR bi;
    int		bpl = image.bytesPerLine();
    int		bpl_bmp;
    int		nbits;

    if ( image.depth() == 8 && image.numColors() <= 16 ) {
	bpl_bmp = (((bpl+1)/2+3)/4)*4;
	nbits = 4;
    } else if ( image.depth() == 32 ) {
	bpl_bmp = ((image.width()*24+31)/32)*4;
	nbits = 24;
    } else {
	bpl_bmp = bpl;
	nbits = image.depth();
    }

    iio->setStatus( 0 );
    s.setByteOrder( QDataStream::LittleEndian );// Intel byte order
    strncpy( bf.bfType, "BM", 2 );		// build file header
    bf.bfReserved1 = bf.bfReserved2 = 0;	// reserved, should be zero
    bf.bfOffBits   = BMP_FILEHDR_SIZE + BMP_WIN + image.numColors()*4;
    bf.bfSize	   = bf.bfOffBits + bpl_bmp*image.height();
    s << bf;					// write file header

    bi.biSize	       = BMP_WIN;		// build info header
    bi.biWidth	       = image.width();
    bi.biHeight	       = image.height();
    bi.biPlanes	       = 1;
    bi.biBitCount      = nbits;
    bi.biCompression   = BMP_RGB;
    bi.biSizeImage     = bpl_bmp*image.height();
    bi.biXPelsPerMeter = 2834;			// 72 dpi
    bi.biYPelsPerMeter = 2834;
    bi.biClrUsed       = image.numColors();
    bi.biClrImportant  = image.numColors();
    s << bi;					// write info header

    if ( image.depth() != 32 ) {		// write color table
	uchar *color_table = new uchar[4*image.numColors()];
	uchar *rgb = color_table;
	QRgb *c = image.colorTable();
	for ( int i=0; i<image.numColors(); i++ ) {
	    *rgb++ = qBlue ( c[i] );
	    *rgb++ = qGreen( c[i] );
	    *rgb++ = qRed  ( c[i] );
	    *rgb++ = 0;
	}
	d->writeBlock( (char *)color_table, 4*image.numColors() );
	delete [] color_table;
    }

    if ( image.depth() == 1 && image.bitOrder() != QImage::BigEndian )
	image = image.convertBitOrder( QImage::BigEndian );

    int	 y;

    if ( nbits == 1 || nbits == 8 ) {		// direct output
	for ( y=image.height()-1; y>=0; y-- )
	    d->writeBlock( (char*)image.scanLine(y), bpl );
	return;
    }

    uchar *buf	= new uchar[bpl_bmp];
    uchar *b, *end;
    register uchar *p;

    memset( buf, 0, bpl_bmp );
    for ( y=image.height()-1; y>=0; y-- ) {	// write the image bits
	if ( nbits == 4 ) {			// convert 8 -> 4 bits
	    p = image.scanLine(y);
	    b = buf;
	    end = b + image.width()/2;
	    while ( b < end ) {
		*b++ = (*p << 4) | (*(p+1) & 0x0f);
		p += 2;
	    }
	    if ( image.width() & 1 )
		*b = *p << 4;
	} else {				// 32 bits: RGB -> BGR
	    QRgb *p   = (QRgb *)image.scanLine( y );
	    QRgb *end = p + image.width();
	    b = buf;
	    while ( p < end ) {
		*b++ = (uchar)(*p >> 16);
		*b++ = (uchar)(*p >> 8);
		*b++ = (uchar)*p++;
	    }
	}
	d->writeBlock( (char*)buf, bpl_bmp );
    }
    delete[] buf;
}


/*****************************************************************************
  PBM/PGM/PPM (ASCII and RAW) image read/write functions
 *****************************************************************************/

static int read_pbm_int( QIODevice *d )		// read int, skip comments
{
    int	  c;
    int	  val = -1;
    bool  digit;
    const int buflen = 100;
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

static void read_pbm_image( QImageIO *iio )	// read PBM image data
{
    const int	buflen = 300;
    char	buf[buflen];
    QRegExp	r1, r2;
    QIODevice  *d = iio->ioDevice();
    int		w, h, nbits, mcc, y;
    int		pbm_bpl;
    char	type;
    bool	raw;
    QImage	image;

    d->readBlock( buf, 3 );			// read P[1-6]<white-space>
    if ( !(buf[0] == 'P' && isdigit(buf[1]) && isspace(buf[2])) )
	return;
    switch ( (type=buf[1]) ) {
	case '1':				// ascii PBM
	case '4':				// raw PBM
	    nbits = 1;
	    break;
	case '2':				// ascii PGM
	case '5':				// raw PGM
	    nbits = 8;
	    break;
	case '3':				// ascii PPM
	case '6':				// raw PPM
	    nbits = 32;
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
    image.create( w, h, nbits, 0,
		  nbits == 1 ? QImage::BigEndian :  QImage::IgnoreEndian );
    if ( image.isNull() )
	return;

    pbm_bpl = (nbits*w+7)/8;			// bytes per scanline in PBM

    if ( raw ) {				// read raw data
	if ( nbits == 32 ) {			// type 6
	    pbm_bpl = 3*w;
	    uchar *buf24 = new uchar[pbm_bpl], *b;
	    QRgb  *p;
	    QRgb  *end;
	    for ( y=0; y<h; y++ ) {
		d->readBlock( (char *)buf24, pbm_bpl );
		p = (QRgb *)image.scanLine( y );
		end = p + w;
		b = buf24;
		while ( p < end ) {
		    *p++ = qRgb(b[0],b[1],b[2]);
		    b += 3;
		}
	    }
	    delete[] buf24;
	} else {				// type 4,5
	    for ( y=0; y<h; y++ )
		d->readBlock( (char *)image.scanLine(y), pbm_bpl );
	}
    } else {					// read ascii data
	register uchar *p;
	int n;
	for ( y=0; y<h; y++ ) {
	    p = image.scanLine( y );
	    n = pbm_bpl;
	    if ( nbits == 1 ) {
		int b;
		while ( n-- ) {
		    b = 0;
		    for ( int i=0; i<8; i++ )
			b = (b << 1) | (read_pbm_int(d) & 1);
		    *p++ = b;
		}
	    } else if ( nbits == 8 ) {
		while ( n-- ) {
		    *p++ = read_pbm_int( d );
		}
	    } else {				// 32 bits
		n /= 4;
		int r, g, b;
		while ( n-- ) {
		    r = read_pbm_int( d );
		    g = read_pbm_int( d );
		    b = read_pbm_int( d );
		    *((QRgb*)p) = qRgb( r, g, b );
		    p += 4;
		}
	    }
	}
    }

    if ( nbits == 1 ) {				// bitmap
	image.setNumColors( 2 );
	image.setColor( 0, qRgb(255,255,255) ); // white
	image.setColor( 1, qRgb(0,0,0) );	// black
    } else if ( nbits == 8 ) {			// graymap
	image.setNumColors( mcc+1 );
	for ( int i=0; i<=mcc; i++ )
	    image.setColor( i, qRgb(i*255/mcc,i*255/mcc,i*255/mcc) );
    }

    iio->setImage( image );
    iio->setStatus( 0 );			// image ok
}


static void write_pbm_image( QImageIO *iio )	// write PBM image data
{
#if defined(CHECK_RANGE)
    warning( "Qt: %s output not supported in this version",
	     (char *)iio->format() );
#endif
}


/*****************************************************************************
  X bitmap image read/write functions
 *****************************************************************************/

static inline int hex2byte( register char *p )
{
    return ((isdigit(*p)     ? *p     - '0' : toupper(*p)     - 'A' + 10)<< 4)|
	   ( isdigit(*(p+1)) ? *(p+1) - '0' : toupper(*(p+1)) - 'A' + 10);
}

static void read_xbm_image( QImageIO *iio )	// read X bitmap image data
{
    const int	buflen = 300;
    char	buf[buflen];
    QRegExp	r1, r2;
    QIODevice  *d = iio->ioDevice();
    int		i;
    int		w=-1, h=-1;
    QImage	image;

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

    image.create( w, h, 1, 2, QImage::LittleEndian );
    if ( image.isNull() )
	return;

    image.setColor( 0, qRgb(255,255,255) );	// white
    image.setColor( 1, qRgb(0,0,0) );		// black

    int	   x = 0, y = 0;
    uchar *b = image.scanLine(0);
    char  *p = strstr( buf, "0x" );
    w = (w+7)/8;				// byte width

    while ( y < h ) {				// for all encoded bytes...
	if ( p ) {				// p = "0x.."
	    *b++ = hex2byte(p+2);
	    p += 2;
	    if ( ++x == w && ++y < h ) {
		b = image.scanLine(y);
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

    iio->setImage( image );
    iio->setStatus( 0 );			// image ok
}


static void write_xbm_image( QImageIO *iio )	// write X bitmap image data
{
    QIODevice *d = iio->ioDevice();
    QImage     image = iio->image();
    int	       w = image.width();
    int	       h = image.height();
    int	       i;
    QString    s = fbname(iio->fileName());	// get file base name
    char       buf[100];

    sprintf( buf, "#define %s_width %d\n",  (const char *)s, w );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "#define %s_height %d\n", (const char *)s, h );
    d->writeBlock( buf, strlen(buf) );
    sprintf( buf, "static char %s_bits[] = {\n ", (const char *)s );
    d->writeBlock( buf, strlen(buf) );

    iio->setStatus( 0 );

    if ( image.depth() != 1 || image.bitOrder() != QImage::LittleEndian ) {
	image = image.convertDepth( 1 );	// dither
	if ( image.bitOrder() != QImage::LittleEndian )
	    image = image.convertBitOrder( QImage::LittleEndian );
    }
    bool invert = qGray(image.color(0)) < qGray(image.color(1));
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
    uchar *b = image.scanLine(0);
    int	 x=0, y=0;
    int nbytes = image.numBytes();
    w = (w+7)/8;
    while ( nbytes-- ) {			// write all bytes
	*p++ = '0';  *p++ = 'x';
	*p++ = hexrep[*b >> 4];
	*p++ = hexrep[*b++ & 0xf];
	if ( ++x == w && y < h-1 ) {
	    b = image.scanLine(++y);
	    x = 0;
	}
	if ( nbytes > 0 ) {
	    *p++ = ',';
	    if ( ++bcnt > 14 ) {
		*p++ = '\n';
		*p++ = ' ';
		*p   = '\0';
		d->writeBlock( buf, strlen(buf) );
		p = buf;
		bcnt = 0;
	    }
	}
    }
    strcpy( p, " };\n" );
    d->writeBlock( buf, strlen(buf) );
}


/*****************************************************************************
  XPM image read/write functions
 *****************************************************************************/

#if 0
static int read_xpm_char( QIODevice *d ) NOT_USED_FN;

static int read_xpm_char( QIODevice *d )
{
    static bool inside_quotes = FALSE;
    static int	save_char = 256;
    int c;
    if ( save_char != 256 ) {
	c = save_char;
	save_char = 256;
	return c;
    }
    if ( (c=d->getch()) == EOF )
	return EOF;
    if ( c == '"' )                             // start or end of quotes
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


static void read_xpm_image( QImageIO * /* iio */ ) // read XPM image data
{
#if 0
    const int	buflen = 200;
    char	buf[buflen];
    char       *p = buf;
    QRegExp	r = "/\\*.XPM.\\*/";
    QIODevice  *d = image->iodev;
    int		c, cpp, ncols, w, h;

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


static void write_xpm_image( QImageIO * /* iio */ ) // write XPM image data
{
#if 0
    QIODevice *d = image->iodev;
    int w = image->width, h = image->height, depth = image->depth, i;
    QString s = fbname(image->fname);		// get file base name
    char buf[100];
#endif
}

#endif

