/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap_x11.cpp#20 $
**
** Implementation of QPixmap class for X11
**
** Author  : Haavard Nord
** Created : 940501
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdc.h"
#include "q2matrix.h"
#include "qapp.h"
#include <malloc.h>
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap_x11.cpp#20 $";
#endif


/*!
\class QPixmap qpixmap.h
\brief The QPixmap class is an off-screen buffer paint device.

A standard use of the QPixmap class is to enable smooth updating of widgets.
Whenever something complex needs to be drawn, you can use a pixmap to
obtain flicker-free drawing.

<ol plain>
<li> Create a pixmap with the necessary size.
<li> Fill the pixmap with the widget background color.
<li> Draw into the pixmap using a QPainter.
<li> bitBlt the contens of the pixmap onto the widget.
</ol>

The code may look like this:
\code
void MyWidget::paintEvent( QPaintEvent * e) {
  QPixmap  pm( width(), height());	// create pixmap
  QPainter p;				// our painter
  pm.fill( backgroundColor() );		// initialize pixmap
  p.begin( &pm );			// paint inside pixmap
  ...					// do complex drawing
  p.end();				// painting done
  bitBlt( this, 0, 0, &pm, 0, 0, -1, -1 );// copy pixmap to widget
}
\endcode

The bitBlt() function is explained in the QPaintDevice documentation.

Pixel data in a pixmap is internal and managed by the underlying window system.
Pixels can only be accessed through QImage, Painter functions and the bitBlt.

A pixmap can be converted to a QImage to get direct access to the pixels.
A QImage can also be converted back to a pixmap.

The QPixmap class is internally based on implicit data sharing.
Shared classes contains a data block with a reference count.
The reference count reports the number of objects that are referring
to the data block.

Making a copy of the object (called shallow copy) is accomplished
just by setting a pointer and incrementing the reference count.
An object that is destroyed decrements the reference count.
If the reference count reaches zero, then the destructor deletes
the data block.

It is much more expensive to make a deep copy.	A deep copy requires that
all data is copied.

The advantages of data sharing is that objects can easily be assigned,
sent as parameters to functions and returned from functions.
Shared classes also allocates less memory and require less copying of data.

*/ /*
The disadvantage of data sharing is that changing one object will
automatically change all other objects that refer to the same data.
This is always the case when using explicite sharing.  The programmer
has to be careful when modifying objects, and in some cases, make
copies or detach an object from the rest.

Implicit sharing solves these problems. A class that uses implicit
sharing knows when it is about to be modified, and then detaches
itself from the rest.  Implicit sharing is only possible when the
class has total control of the call interface.

Implicit sharing cannot be done when a class exports some of its
internal data. Therefore, classes like QString and QImage must be explicitly
shared classes.
*/

// --------------------------------------------------------------------------
// Internal functions
//

static uchar *flip_bits( uchar *bits, int len )	// flip bits in bitmap
{
    extern char *qt_get_bitflip_array();	// defined in qimage.cpp
    register uchar *p = bits;
    uchar *end = p + len;
    uchar *newdata = new uchar[len];
    uchar *b = newdata;
    uchar *f = (uchar *)qt_get_bitflip_array();
    while ( p < end )
	*b++ = f[*p++];
    return newdata;
}

static int highest_bit( ulong v )
{
    int i;
    ulong b = (uint)1 << 31;			// get pos of highest set bit
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}


// --------------------------------------------------------------------------
// QPixmap member functions
//

void QPixmap::init()
{
    data = new QPixmapData;
    CHECK_PTR( data );
    data->dirty	 = data->optim = FALSE;
    data->uninit = TRUE;
    data->bitmap = FALSE;
    data->ximage = 0;
}


/*!
Constructs a null pixmap.

\sa isNull().
*/

QPixmap::QPixmap()
    : QPaintDevice( PDT_PIXMAP )
{
    init();
    data->w = data->h = 0;
    data->d = 0;
    hd = 0;
}

/*!
Constructs a pixmap with \e w width, \e h height and of \e depth bits per
pixels.

The contents of the pixmap is uninitialized.

The \e depth can be either 1 (monochrome) or the default depth
supported by the hardware (normally 8 bits).
If \e depth is negative, then the hardware depth will be used.
*/

QPixmap::QPixmap( int w, int h, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    init();
    int dd = DefaultDepth( dpy, qt_xscreen() );
    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( depth == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( depth < 0 || depth == dd )	// compatible pixmap
	data->d = dd;
    else
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
	hd = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    hd = XCreatePixmap( dpy, DefaultRootWindow(dpy), w, h, data->d );
}

/*!
Constructs a pixmap with size \e sz and of \e depth bits per pixels.

The contents of the pixmap is uninitialized.

The \e depth can be either 1 (monochrome) or the default depth
supported by the hardware (normally 8 bits).
If \e depth is negative, then the hardware depth will be used.
*/

QPixmap::QPixmap( QSize sz, int depth )
    : QPaintDevice( PDT_PIXMAP )
{
    int w = sz.width();
    int h = sz.height();
    init();
    int dd = DefaultDepth( dpy, qt_xscreen() );
    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( depth == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( depth < 0 || depth == dd )	// compatible pixmap
	data->d = dd;
    else
	data->d = 0;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	data->w = data->h = 0;
	data->d = 0;
	hd = 0;
#if defined(CHECK_RANGE)
	if ( !make_null )			// invalid parameters
	    warning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }
    data->w = w;
    data->h = h;
    hd = XCreatePixmap( dpy, DefaultRootWindow(dpy), w, h, data->d );
}

/*!
Constructs a monochrome pixmap which is initialized with the data in \e bits.
This constructor is protected and used by the QBitmap class.
*/

QPixmap::QPixmap( int w, int h, const char *bits, bool isXbitmap )
    : QPaintDevice( PDT_PIXMAP )
{						// for bitmaps only
    init();
    data->uninit = FALSE;
    if ( w <= 0 ) w = 1;
    if ( h <= 0 ) h = 1;
    data->w = w;  data->h = h;	data->d = 1;
    uchar *flipped_bits;
    if ( isXbitmap )
	flipped_bits = 0;
    else {					// not X bitmap -> flip bits
	flipped_bits = flip_bits( (uchar *)bits, ((w+7)/8)*h );
	bits = (const char *)flipped_bits;
    }
    hd = XCreateBitmapFromData( dpy, DefaultRootWindow(dpy), bits, w, h );
    delete flipped_bits;
}

/*!
Constructs a pixmap which is a shallow copy of \e pm.
*/

QPixmap::QPixmap( const QPixmap &pm )
    : QPaintDevice( PDT_PIXMAP )
{
    data = pm.data;
    data->ref();
    devFlags = pm.devFlags;			// copy QPaintDevice flags
    dpy = pm.dpy;				// copy QPaintDevice display
    hd	= pm.hd;				// copy QPaintDevice drawable
}

/*!
Dereferences the internal pixmap data, and deletes it if this is the
last reference.
*/

QPixmap::~QPixmap()
{
    if( data->deref() ) {			// last reference lost
	if ( data->ximage )
	    XDestroyImage( (XImage*)data->ximage );
	if ( hd )
	    XFreePixmap( dpy, hd );
	delete data;
    }
}


/*!
Assigns a shallow copy of \e p to this pixmap and returns a reference
to this pixmap.

\sa copy().
*/

QPixmap &QPixmap::operator=( const QPixmap &p )
{
    p.data->ref();				// avoid 'x = x'
    if ( data->deref() ) {			// last reference lost
	if ( data->ximage )
	    XDestroyImage( (XImage*)data->ximage );
	if ( hd )
	    XFreePixmap( dpy, hd );
	delete data;
    }
    data     = p.data;
    devFlags = p.devFlags;			// copy QPaintDevice flags
    dpy = p.dpy;				// copy QPaintDevice display
    hd	= p.hd;					// copy QPaintDevice drawable
    return *this;
}

/*!
Converts the image \e im to a pixmap that is assigned to this pixmap.
Returns a reference to the pixmap.

\sa convertFromImage().
*/

QPixmap &QPixmap::operator=( const QImage &im )
{
    convertFromImage( im );
    return *this;
}


/*!
\fn bool QPixmap::isNull() const
Returns TRUE if it is a null pixmap.

A null pixmap has zero width, zero height and no contents.
You cannot draw in a null pixmap or bitBlt anything to it.

\sa isNull().
*/


/*!
\fn int QPixmap::width() const
Returns the width of the pixmap.
*/

/*!
\fn int QPixmap::height() const
Returns the height of the pixmap.
*/

/*!
\fn QSize QPixmap::size() const
Returns the size of the pixmap.
*/

/*!
\fn QRect QPixmap::rect() const
Returns the enclosing rectangle of the pixmap.
*/

/*!
\fn int QPixmap::depth() const
Returns the depth of the image.

The pixmap depth is also called bits per pixel (bpp) or bit planes of a pixmap.
*/

/*!
\fn int QPixmap::numColors() const
Returns the maximum number of colors that can be used for the pixmap.

Similar to 2^depth.
*/


/*!
Enables the internal image cache if \e enable is TRUE, or disables it if
\e enable is FALSE.  Returns the previous setting of the image cache.

This cache is disabled by default.

Enabling the internal image cache speeds up functions that fetch the
pixmap contents; convertToImage() and xForm().
*/

bool QPixmap::enableImageCache( bool enable )
{
    bool v = data->optim;
    data->optim = enable ? 1 : 0;
    data->dirty = FALSE;
    if ( data->ximage ) {
	XDestroyImage( (XImage*)data->ximage );
	data->ximage = 0;
    }
    return v;
}


/*!
Fills the pixmap with the color \e fillColor.
*/

void QPixmap::fill( const QColor &fillColor )	// fill pixmap contents
{
    if ( isNull() )
	return;
    detach();					// detach other references
    GC gc = qt_xget_temp_gc( depth()==1 );
    XSetForeground( dpy, gc, fillColor.pixel() );
    XFillRectangle( dpy, hd, gc, 0, 0, width(), height() );
}


/*!
Returns pixmap metics.	This is a reimplementation of the virtual function
QPaintDevice::metric().
*/

long QPixmap::metric( int m ) const		// get metric information
{
    long val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = width();
	else
	    val = height();
    }
    else {
	int scr = qt_xscreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = ((long)DisplayWidthMM(dpy,scr)*width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
		val = ((long)DisplayHeightMM(dpy,scr)*height())/
		      DisplayHeight(dpy,scr);
		break;
	    case PDM_NUMCOLORS:
		val = 1 << depth();
		break;
	    case PDM_NUMPLANES:
		val = depth();
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QPixmap::metric: Invalid metric command" );
#endif
	}
    }
    return val;
}


/*!
Converts the pixmap to an image. Returns a null image if the operation failed.

If the pixmap has 1 bit depth, the returned image will also get 1 bit
depth.
If the pixmap has 2-8 bits depth, the returned image gets 8 bit depth.
If the pixmap has greater than 8 bits depth, the returned image gets
24 bits depth.

\bug Does not support 2 or 4 bit display hardware. This function needs
to be tested on different types of X servers.
*/

QImage QPixmap::convertToImage() const
{
    QImage image;
    if ( isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertToImage: Cannot convert null pixmap" );
#endif
	return image;
    }

    int	    scr	   = qt_xscreen();
    Visual *visual = DefaultVisual(dpy,scr);
    bool    trucol = visual->c_class == TrueColor;
    XImage *xi = 0;				// get pixmap data from server
    int	    w  = width();
    int	    h  = height();
    int	    d  = depth();
    bool    mono = d == 1;

    if ( d > 1 && d < 8 )			// set to nearest valid depth
	d = 8;					//   2..7 ==> 8
    else if ( d > 8 || trucol )
	d = 24;					//   > 9  ==> 24
    
    if ( data->optim ) {
	if ( !data->dirty )
	    xi = (XImage *)data->ximage;
	else if ( data->ximage ) {
	    XDestroyImage( (XImage*)data->ximage );
	    ((QPixmap *)this)->data->ximage = 0;
	}
    }
    if ( !xi )					// fetch data from X server
	xi = XGetImage( dpy, hd, 0, 0, w, h, AllPlanes,
			mono ? XYPixmap : ZPixmap );
    CHECK_PTR( xi );

    int bitOrder = QImage::IgnoreEndian;
    if ( mono ) {
	bitOrder = xi->bitmap_bit_order == LSBFirst ?
	    QImage::LittleEndian : QImage::BigEndian;
    }
    image.create( w, h, d, 0, bitOrder );
    if ( image.isNull() )			// could not create image
	return image;

    if ( trucol ) {				// truecolor
	uint red_mask	 = visual->red_mask;
	uint green_mask	 = visual->green_mask;
	uint blue_mask	 = visual->blue_mask;
	int  red_shift	 = highest_bit( red_mask )   - 7;
	int  green_shift = highest_bit( green_mask ) - 7;
	int  blue_shift	 = highest_bit( blue_mask )  - 7;
	int  r, g, b;

	uchar *dst = image.bits();
	uchar *src;
	ulong  pixel;
	int    bppc = xi->bits_per_pixel;

	if ( bppc > 8 && xi->byte_order == LSBFirst )
	    bppc++;

	for ( int y=0; y<h; y++ ) {
	    src = (uchar *)xi->data + xi->bytes_per_line*y;
	    for ( int x=0; x<w; x++ ) {
		switch ( bppc ) {
		    case 8:
			pixel = *src++;
			break;
		    case 16:			// 16 bit MSB
			pixel = src[1] | (ushort)src[0] << 8;
			src += 2;
			break;
		    case 17:			// 16 bit LSB
			pixel = src[0] | (ushort)src[1] << 8;
			src += 2;
			break;
		    case 24:			// 24 bit MSB
			pixel = src[2] | (ushort)src[1] << 8 |
				(ulong)src[0] << 16;
			src += 3;
			break;
		    case 25:			// 24 bit LSB
			pixel = src[0] | (ushort)src[1] << 8 |
				(ulong)src[2] << 16;
			src += 3;
			break;
		    case 32:			// 32 bit MSB
			pixel = src[3] | (ushort)src[2] << 8 |
				(ulong)src[1] << 16 | (ulong)src[0] << 24;
			src += 4;
			break;
		    case 33:			// 32 bit LSB
			pixel = src[0] | (ushort)src[1] << 8 |
				(ulong)src[2] << 16 | (ulong)src[3] << 24;
			src += 4;
			break;
		}
		if ( red_shift > 0 )
		    r = (pixel & red_mask) >> red_shift;
		else
		    r = (pixel & red_mask) << -red_shift;
		if ( green_shift > 0 )
		    g = (pixel & green_mask) >> green_shift;
		else
		    g = (pixel & green_mask) << -green_shift;
		if ( blue_shift > 0 )
		    b = (pixel & blue_mask) >> blue_shift;
		else
		    b = (pixel & blue_mask) << -blue_shift;
		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
	    }
	}
    }
    else if ( xi->bits_per_pixel == d ) {	// compatible depth
	char *xidata = xi->data;		// copy each scanline
	int bpl = image.bytesPerLine();
	for ( int y=0; y<h; y++ ) {
	    memcpy( image.scanline(y), xidata, bpl );
	    xidata += xi->bytes_per_line;
	}
    }
    else {					// !!! to be implemented
	/* Typically 2 or 4 bits display depth */
#if defined(CHECK_RANGE)
	warning( "QPixmap::convertToImage: DISPLAY NOT SUPPORTED (BPP=%d)",
		 xi->bits_per_pixel );
#endif
	image.reset();
	return image;
    }

    if ( mono ) {				// bitmap
	image.setNumColors( 2 );
	image.setColor( 0, QRGB(255,255,255) );
	image.setColor( 1, QRGB(0,0,0) );
    }
    else if ( !trucol ) {			// pixmap with colormap
	register uchar *p;
	uchar  use[256];			// pixel-in-use table
	uchar  pix[256];			// pixel translation table
	int    ncols, i;
	memset( use, 0, 256 );
	memset( pix, 0, 256 );
	p = image.bits();  i = image.numBytes();
	while ( i-- )				// what pixels are used?
	    use[*p++] = 1;
	ncols = 0;
	for ( i=0; i<256; i++ )	{		// build translation table
	    if ( use[i] )
		pix[i] = ncols++;
	}
	p = image.bits();  i = image.numBytes();
	while ( i-- )				// translate pixels
	    *p++ = pix[*p];

	Colormap cmap	= DefaultColormap( dpy, scr );
	int	 ncells = DisplayCells( dpy, scr );
	XColor *carr = new XColor[ncells];
	for ( i=0; i<ncells; i++ )
	    carr[i].pixel = i;
	XQueryColors( dpy, cmap, carr, ncells );// get default colormap

	image.setNumColors( ncols );		// create color table
	int j = 0;
	for ( i=0; i<256; i++ ) {		// translate pixels
	    if ( use[i] ) {
		image.setColor( j++,
				QRGB( (carr[i].red   >> 8) & 255,
				      (carr[i].green >> 8) & 255,
				      (carr[i].blue  >> 8) & 255 ) );
	    }
	}
	delete carr;
    }
    if ( data->optim ) {			// keep ximage that we fetched
	((QPixmap*)this)->data->dirty  = FALSE;
	((QPixmap*)this)->data->ximage = xi;
    }
    else
	XDestroyImage( xi );
    return image;
}


/*!
Converts the image data and sets this pixmap. Returns TRUE if successful.

If \e image has more colors than the number of available colors, we try
to pick the most important colors.

If this pixmap is an instance of QBitmap and \e image has 8 or 24 bits
depth, then the image will be dithered using the Floyd-Steinberg dithering
algorithm.

\bug Does not support 2 or 4 bit display hardware. This function needs
to be tested on different types of X servers.
*/

bool QPixmap::convertFromImage( const QImage &img )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot set null image" );
#endif
	return FALSE;
    }
    detach();					// detach other references
    QImage  image = img;
    int w   = image.width();
    int h   = image.height();
    int d   = image.depth();
    int scr = qt_xscreen();
    int dd  = DefaultDepth(dpy,scr);

    if ( (dd == 1 || data->bitmap) && d > 1 ) {	// force to bitmap
	image = image.convertDepth( 1 );	// dither
	d = 1;
    }

    if ( d == 1 ) {				// 1 bit pixmap (bitmap)
	if ( hd )				// delete old X pixmap
	    XFreePixmap( dpy, hd );
	char *bits;
	uchar *flipped_bits;
	if ( image.bitOrder() == QImage::BigEndian ) {
	    flipped_bits = flip_bits( image.bits(), image.numBytes() );
	    bits = (char *)flipped_bits;
	}
	else {
	    bits = (char *)image.bits();
	    flipped_bits = 0;
	}
	hd = XCreateBitmapFromData( dpy, DefaultRootWindow(dpy), bits, w, h );
	delete flipped_bits;
	data->w = w;  data->h = h;  data->d = 1;
	if ( data->optim ) {
	    data->dirty = FALSE;
	    if ( data->ximage ) {		// we got no X image data
		XDestroyImage( (XImage*)data->ximage );
		data->ximage = 0;
	    }
	}
	return TRUE;
    }

    Visual *visual = DefaultVisual(dpy,scr);
    XImage *xi	   = 0;
    bool    trucol = visual->c_class == TrueColor;
    int	    nbytes = image.numBytes();
    uchar  *newbits;
    register uchar *p;

    if ( trucol ) {				// truecolor display
	ulong pix[256];				// pixel translation table
	bool  d8 = d == 8;
	uint  red_mask	  = visual->red_mask;
	uint  green_mask  = visual->green_mask;
	uint  blue_mask	  = visual->blue_mask;
	int   red_shift	  = highest_bit( red_mask )   - 7;
	int   green_shift = highest_bit( green_mask ) - 7;
	int   blue_shift  = highest_bit( blue_mask )  - 7;
	int   r, g, b;

	if ( d8 ) {				// setup pixel translation
	    for ( int i=0; i<image.numColors(); i++ ) {
		r = QRED  (image.color(i));
		g = QGREEN(image.color(i));
		b = QBLUE (image.color(i));
		r = red_shift	> 0 ? r << red_shift   : r >> -red_shift;
		g = green_shift > 0 ? g << green_shift : g >> -green_shift;
		b = blue_shift	> 0 ? b << blue_shift  : b >> -blue_shift;
		pix[i] = (b & blue_mask) | (g & green_mask) | (r & red_mask);
	    }
	}

	xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0 );
	CHECK_PTR( xi );
	newbits = (uchar *)malloc( xi->bytes_per_line*h );
	uchar *src = image.bits();
	uchar *dst;
	ulong  pixel;
	int    bppc = xi->bits_per_pixel;

	if ( bppc > 8 && xi->byte_order == LSBFirst )
	    bppc++;

	for ( int y=0; y<h; y++ ) {
	    dst = newbits + xi->bytes_per_line*y;
	    for ( int x=0; x<w; x++ ) {
		if ( d8 )
		    pixel = pix[*src++];
		else {
		    r = *src++;
		    g = *src++;
		    b = *src++;
		    r = red_shift   > 0 ? r << red_shift   : r >> -red_shift;
		    g = green_shift > 0 ? g << green_shift : g >> -green_shift;
		    b = blue_shift  > 0 ? b << blue_shift  : b >> -blue_shift;
		    pixel = (b & blue_mask)|(g & green_mask) | (r & red_mask);
		}
		switch ( bppc ) {
		    case 8:
			*dst++ = pixel;
			break;
		    case 16:			// 16 bit MSB
			*dst++ = (pixel >> 8);
			*dst++ = pixel;
			break;
		    case 17:			// 16 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			break;
		    case 24:			// 24 bit MSB
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 8;
			*dst++ = pixel;
			break;
		    case 25:			// 24 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			*dst++ = pixel >> 16;
			break;
		    case 32:			// 32 bit MSB
			*dst++ = pixel >> 24;
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 8;
			*dst++ = pixel;
			break;
		    case 33:			// 32 bit LSB
			*dst++ = pixel;
			*dst++ = pixel >> 8;
			*dst++ = pixel >> 16;
			*dst++ = pixel >> 24;
			break;
		}
	    }
	}
	xi->data = (char *)newbits;
    }

    if ( d == 24 && !trucol ) {			// convert to 8 bit
	image = image.convertDepth( 8 );
	d = 8;
	nbytes = image.numBytes();		// recalc image size
    }

    if ( d == 8 && !trucol ) {			// 8 bit pixmap
	int    i, j;
	long   pop[256];			// pixel popularity
	newbits = (uchar *)malloc( nbytes );
	if ( !newbits )				// no memory
	    return FALSE;
	p = newbits;
	memcpy( p, image.bits(), nbytes );	// copy image data into newbits
	memset( pop, 0, sizeof(long)*256 );	// reset popularity array
	for ( i=0; i<nbytes; i++ )		// compute popularity
	    pop[*p++]++;

/*
 * The following lines of code that sort the color table comes from XV 3.10
 * (C) John Bradley.
 */

	struct PIX {				// pixel sort element
	    uchar r,g,b,n;			// color + pad
	    int	  use;				// popularity
	    int	  index;			// index in colormap
	    int	  mindist;
	};
	int ncols = 0;
	for ( i=0; i<256; i++ )			// compute number of colors
	    if ( pop[i] > 0 )
		ncols++;
	if ( ncols > image.numColors() )	// shouldn't happen
	    ncols = image.numColors();

	PIX *pixarr	   = new PIX[ncols];	// pixel array
	PIX *pixarr_sorted = new PIX[ncols];	// pixel array (sorted)
	PIX *px = &pixarr[0];
	int  maxpop = 0;
	int  maxpix = 0;
	CHECK_PTR( pixarr );
	j = 0;
	for ( i=0; i<256; i++ ) {		// init pixel array
	    if ( pop[i] > 0 ) {
		px->r = QRED  ( image.color(i) );
		px->g = QGREEN( image.color(i) );
		px->b = QBLUE ( image.color(i) );
		px->use = pop[i];
		if ( pop[i] > maxpop ) {	// select most popular entry
		    maxpop = pop[i];
		    maxpix = j;
		}
		px->index = i;
		px->mindist = 1000000;
		px++;
		j++;
	    }
	}
	memcpy( &pixarr_sorted[0], &pixarr[maxpix], sizeof(PIX) );
	pixarr[maxpix].use = 0;

	for ( i=1; i<ncols; i++ ) {		// sort pixels
	    int minpix = -1, mindist = -1;
	    px = &pixarr_sorted[i-1];
	    int r = px->r;
	    int g = px->g;
	    int b = px->b;
	    int dist;
	    if ( (i & 1) || i<10 ) {		// sort on max distance
		for ( j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->mindist > mindist ) {
			    mindist = px->mindist;
			    minpix = j;
			}
		    }
		}
	    }
	    else {				// sort on max popularity
		for ( j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->use > mindist ) {
			    mindist = px->use;
			    minpix = j;
			}
		    }
		}
	    }
	    memcpy( &pixarr_sorted[i], &pixarr[minpix], sizeof(PIX) );
	    pixarr[minpix].use = 0;
	}

	int pix[256];				// pixel translation table
	px = &pixarr_sorted[0];
	for ( i=0; i<ncols; i++ ) {		// allocate colors
	    QColor c( px->r, px->g, px->b );
	    pix[px->index] = c.pixel();
	    px++;
	}
	delete pixarr;
	delete pixarr_sorted;

	p = newbits;
	for ( i=0; i<nbytes; i++ )		// translate pixels
	    *p++ = pix[*p];
    }

    if ( data->optim ) {			// image data optimization
	if ( data->ximage ) {			// kill old image data
	    XDestroyImage( (XImage*)data->ximage );
	    data->ximage = 0;
	}
    }
    if ( !xi ) {				// x image not created
	xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 8, 0 );
	if ( xi->bits_per_pixel == 16 ) {	// convert 8 bpp ==> 16 bpp
	    ushort *p2;
	    int	    p2inc = xi->bytes_per_line/sizeof(ushort);
	    ushort *newerbits = (ushort *)malloc( xi->bytes_per_line * h );
	    CHECK_PTR( newerbits );
	    p = newbits;
	    for ( int y=0; y<h; y++ ) {		// OOPS: Do right byte order!!
		p2 = newerbits + p2inc*y;
		for ( int x=0; x<w; x++ )
		    *p2++ = *p++;
	    }
	    free( newbits );
	    newbits = (uchar *)newerbits;	
	}
	else if ( xi->bits_per_pixel != 8 ) {	// !!! to be implemented
#if defined(CHECK_RANGE)
	    warning( "QPixmap::setImageData: DISPLAY NOT SUPPORTED (BPP=%d)",
		     xi->bits_per_pixel );
#endif	    
	}
	xi->data = (char *)newbits;
    }

    if ( hd && (width() != w || height() != h || depth() != dd) ) {
	XFreePixmap( dpy, hd );			// don't reuse old pixmap
	hd = 0;
    }
    if ( !hd )					// create new pixmap
	hd = XCreatePixmap( dpy, DefaultRootWindow(dpy), w, h, dd );

    XPutImage( dpy, hd, qt_xget_readonly_gc(), xi, 0, 0, 0, 0, w, h );
    if ( data->optim ) {			// keep ximage that we created
	data->dirty  = FALSE;
	data->ximage = xi;
    }
    else
	XDestroyImage( xi );
    data->w = w;  data->h = h;	data->d = dd;
    return TRUE;
}


/*!
Grabs the contents of a window and makes a pixmap out of it.
Returns the pixmap.

The argments \e x and \e y specify the offset in the window, while
\e w and \e h specify the width and height of the area to be copied.

If \e w is negative, the function copies everything to the right
border of the window.
If \e h is negative, the function copies everything to the bottom of
the window. 
*/

QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    Display *dpy = qt_xdisplay();
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 ) {
	    QPixmap nullPixmap;
	    return nullPixmap;
	}
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	if ( w < 0 )
	    w = a.width - x;
	if ( h < 0 )
	    h = a.height - y;
    }
    XImage *xi;					// get pixmap data from window
    xi = XGetImage( dpy, window, x, y, w, h, AllPlanes, ZPixmap );
    CHECK_PTR( xi );
    QPixmap pm( w, h );				// create new pixmap
    XPutImage( dpy, pm.handle(), qt_xget_readonly_gc(), xi, x, y, 0, 0, w, h);
    XDestroyImage( xi );
    return pm;
}


static inline int d2i_round( double d )		// double -> int, rounded
{
    return d > 0 ? int(d+0.5) : int(d-0.5);
}


/*!
Transforms the pixmap using \e matrix, and returns the transformed pixmap.

Qt uses this function to implement rotated text on window systems that
do not support such complex features.

Example of how to manually draw a rotated text at (100,200) in a widget:
\code
  QWidget  w;				\/ our widget
  char	  *str = "Trolls R Qt";		\/ text to be drawn
  QFont	   f( "Charter", 24 );		\/ use Charter 24pt font
  QFontMetrics fm( f );			\/ get font metrics
  QRect	   r = fm.boundingRect( str );	\/ get text rectangle

  QPixmap  pm( r.width(), r.height() );	\/ pixmap to be rotated
  QPoint   bl = -r.topLeft();		\/ baseline position
  QPainter p;				\/ paints pm

  pm.fill();				\/ fills pm with white
  p.begin( &pm );			\/ begin painting pm
  p.setFont( f );			\/ set the font
  p.setPen( blue );			\/ set blue text color
  p.drawText( 0,bl, str );		\/ draw the text
  p.end();				\/ painting done

  Q2DMatrix m;				\/ transformation matrix
  m.rotate( -33.4 );			\/ rotate coordinate system
  QPixmap rp = pm.xForm( m );		\/ rp is rotated pixmap

  Q2DMatrix t = QPixmap::trueMatrix( m, pm.width(), pm.height() );
  int x, y;
  t.map( bl.x(),bl.y(), &x,&y );	\/ get pm's baseline pos in rp

  bitBlt( &w, 100-x, 200-y,		\/ blt rp into the widget
	  &rp, 0, 0, -1, -1 );
\endcode

\sa trueMatrix().

\bug 2 and 4 bits pixmaps not supported.
*/

QPixmap QPixmap::xForm( const Q2DMatrix &matrix ) const
{
    int	   w, h;				// size of target pixmap
    int	   ws, hs;				// size of source pixmap
    uchar *dptr;				// data in target pixmap
    int	   dbpl, dbytes;			// bytes per line/bytes total
    uchar *sptr;				// data in original pixmap
    int	   sbpl;				// bytes per line in original
    int	   bpp;					// bits per pixel
    bool   depth1 = depth() == 1;

    if ( isNull() ) {				// this is a null pixmap
	return copy();
    }

    ws = width();
    hs = height();

    const float dt = 0.0001;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)ws - 1;
    float yy = (float)hs - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    Q2DMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;

    if ( mat.m12() != 0.0 || mat.m21() != 0.0 ) {
	QPointArray a( QRect(0,0,ws,hs) );
	a = mat.map( a );
	QRect r = a.boundingRect();
	r.fixup();
	h = r.height();
	w = r.width();
    }
    else {					// no rotation/shearing
	h = d2i_round( mat.m22()*hs );
	w = d2i_round( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );
    }
    bool invertible;
    mat = mat.invert( &invertible );		// invert matrix

    if ( h == 0 || w == 0 || !invertible ) {	// null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

    XImage *xi = 0;				// get bitmap data from server
    if ( data->optim ) {
	if ( !data->dirty )
	    xi = (XImage*)data->ximage;
	else if ( data->ximage ) {
	    XDestroyImage( (XImage*)data->ximage );
	    data->ximage = 0;
	}
    }
    if ( !xi )
	xi = XGetImage( display(), handle(), 0, 0, ws, hs, AllPlanes,
			depth1 ? XYPixmap : ZPixmap );
    sbpl = xi->bytes_per_line;
    sptr = (uchar *)xi->data;
    bpp	 = xi->bits_per_pixel;

    if ( depth1 )
	dbpl = (w+7)/8;
    else
	dbpl = (((w*bpp)/8 + 3)/4)*4;
    dbytes = dbpl*h;
    dptr   = (uchar *)malloc( dbytes );		// create buffer for bits
    CHECK_PTR( dptr );
    if ( depth1 )				// fill with zeros
	memset( dptr, 0, dbytes );
    else if ( bpp == 8 )			// fill with background color
	memset( dptr, white.pixel(), dbytes );
    else
	memset( dptr, 0xff, dbytes );

// #define DEBUG_XIMAGE
#if defined(DEBUG_XIMAGE)
    debug( "----IMAGE--INFO--------------" );
    debug( "width............. %d", xi->width );
    debug( "height............ %d", xi->height );
    debug( "xoffset........... %d", xi->xoffset );
    debug( "format............ %d", xi->format );
    debug( "byte order........ %d", xi->byte_order );
    debug( "bitmap unit....... %d", xi->bitmap_unit );
    debug( "bitmap bit order.. %d", xi->bitmap_bit_order );
    debug( "depth............. %d", xi->depth );
    debug( "bytes per line.... %d", xi->bytes_per_line );
    debug( "bits per pixel.... %d", xi->bits_per_pixel );
#endif

    int	m11 = d2i_round((double)mat.m11()*65536.0);
    int	m12 = d2i_round((double)mat.m12()*65536.0);
    int	m21 = d2i_round((double)mat.m21()*65536.0);
    int	m22 = d2i_round((double)mat.m22()*65536.0);
    int	dx  = d2i_round((double)mat.dx() *65536.0);
    int	dy  = d2i_round((double)mat.dy() *65536.0);
#if 0
    dx += (dx > 0) ? 32767 : -32768;		// gives error when scaling
    dy += (dy > 0) ? 32767 : -32768;
#endif
    int	  m21ydx = dx + (xi->xoffset<<16), m22ydy = dy;
    ulong trigx, trigy;
    ulong maxws = ws<<16, maxhs=hs<<16;
    uchar *p	= dptr;
    int	  maxx;
    int	  p_inc;
    int	  cntx;
    bool  msbfirst = xi->bitmap_bit_order == MSBFirst;

    if ( depth1 ) {
	maxx = (w+7)/8;
	p_inc = dbpl - maxx;
    }
    else {
	maxx = w;
	p_inc = dbpl - (maxx*bpp)/8;
    }

    for ( int y=0; y<h; y++ ) {			// for each target scanline
	trigx = m21ydx;
	trigy = m22ydy;
	cntx = maxx;
	int x;
	if ( !depth1 ) {
	    switch ( bpp ) {
		case 8:				// 8 bpp transform
		while ( cntx-- ) {
		    if ( trigx < maxws && trigy < maxhs )
			*p = *(sptr+sbpl*(trigy>>16)+(trigx>>16));
		    trigx += m11;
		    trigy += m12;
		    p++;
		}
		break;

		case 16:			// 16 bpp transform
		while ( cntx-- ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((ushort*)p) = *((ushort *)(sptr+sbpl*(trigy>>16) +
						     ((trigx>>16)<<1)));
		    trigx += m11;
		    trigy += m12;
		    p++;
		    p++;
		}
		break;

		case 24: {			// 24 bpp transform
		uchar *p2;
		while ( cntx-- ) {
		    if ( trigx < maxws && trigy < maxhs ) {
			p2 = sptr+sbpl*(trigy>>16) + ((trigx>>16)<<2);
			p[0] = p2[0];
			p[1] = p2[1];
			p[2] = p2[2];
		    }
		    trigx += m11;
		    trigy += m12;
		    p += 3;
		}
		}
		break;

		case 32:			// 32 bpp transform
		while ( cntx-- ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((ulong*)p) = *((ulong *)(sptr+sbpl*(trigy>>16) +
						   ((trigx>>16)<<3)));
		    trigx += m11;
		    trigy += m12;
		    p += 4;
		}
		break;

		default: {
#if defined(CHECK_RANGE)
		warning( "QPixmap::xForm: DISPLAY NOT SUPPORTED (BPP=%d)",bpp);
#endif
		QPixmap pm;
		return pm;
		}
	    }
	}
	else if ( msbfirst ) {
	    while ( cntx-- ) {			// for each target bit
#undef IWX
#define IWX(b)	if ( trigx < maxws && trigy < maxhs ) {			      \
		    x = trigx >> 16;					      \
		    if ( *(sptr+sbpl*(trigy>>16)+(x>>3)) & (1 << 7-(x&7)) )   \
			*p |= b;					      \
		}							      \
		trigx += m11;						      \
		trigy += m12;
	// END OF MACRO
		IWX(1);
		IWX(2);
		IWX(4);
		IWX(8);
		IWX(16);
		IWX(32);
		IWX(64);
		IWX(128);
		p++;
	    }
	}
	else {
	    while ( cntx-- ) {			// for each target bit
#undef IWX
#define IWX(b)	if ( trigx < maxws && trigy < maxhs ) {			      \
		    x = trigx >> 16;					      \
		    if ( *(sptr+sbpl*(trigy>>16)+(x>>3)) & (1 << (x&7)) )     \
			*p |= b;					      \
		}							      \
		trigx += m11;						      \
		trigy += m12;
	// END OF MACRO
		IWX(1);
		IWX(2);
		IWX(4);
		IWX(8);
		IWX(16);
		IWX(32);
		IWX(64);
		IWX(128);
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }
    if ( data->optim ) {			// keep ximage that we fetched
	data->dirty  = FALSE;
	data->ximage = xi;
    }
    else
	XDestroyImage( xi );

    if ( depth1 ) {
	QPixmap pm( w, h, (const char *)dptr, TRUE );
	pm.data->bitmap = data->bitmap;
	free( dptr );
	return pm;
    }
    else {
	Display *dpy = display();
	int scr = qt_xscreen();
	int dd	= DefaultDepth(dpy,scr);
	GC  gc	= qt_xget_readonly_gc();
	xi = XCreateImage( dpy, DefaultVisual(dpy,scr), dd, ZPixmap, 0,
			   (char *)dptr, w, h, 32, 0 );
	QPixmap pm( w, h, dd );
	pm.data->bitmap = data->bitmap;
	XPutImage( dpy, pm.handle(), gc, xi, 0, 0, 0, 0, w, h);
	XDestroyImage( xi );	
	return pm;
    }
}


/*!
Returns the actual matrix used for transforming a pixmap with \e w
width and \e h height.

When transforming a pixmap with xForm(), the transformation matrix is
internally adjusted to compensate for unwanted translation, i.e. xForm()
returns the smallest pixmap containing all transformed points of the
original pixmap.

This function returns the modified matrix, which maps points correctly
from the original pixmap into the new pixmap.

\sa xForm().
*/

Q2DMatrix QPixmap::trueMatrix( const Q2DMatrix &matrix, int w, int h )
{						// get true wxform matrix
    const float dt = 0.0001;
    float x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    float xx = (float)w - 1;
    float yy = (float)h - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    float ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    float xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    Q2DMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;
    return mat;
}
