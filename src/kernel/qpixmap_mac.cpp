#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qwmatrix.h"
#include "qt_mac.h"
extern const uchar *qt_get_bitflip_array();		// defined in qimage.cpp

QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( QInternal::Pixmap )
{
    init(w, h, 1, TRUE, DefaultOptim);
    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifdef QMAC_NO_QUARTZ
#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)hd)), *drow, q;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)hd));
    char mode = true32b;
    SwapMMUMode(&mode);
    for(int yy=0;yy<h;yy++) {
	drow = (long *)((char *)dptr + (yy * dbpr));
	int sy = yy * ((w+7)/8);
	for(int xx=0;xx<w;xx++) {
	    char one_bit = *(bits + (sy + (xx / 8)));
	    if(!isXbitmap)
		one_bit = one_bit >> (7 - (xx % 8));
	    else
		one_bit = one_bit >> (xx % 8);
	    q = 0;
	    if(!(one_bit & 0x01))
		q = (255 << 16) | (255 << 8) | 255;
	    *(drow + xx) = q;
	}
    }
    SwapMMUMode(&mode);
#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif
#else //!QMAC_NO_QUARTZ
    //FIXME
#endif
}

static inline QRgb qt_conv16ToRgb( ushort c ) {
    static const int qt_rbits = (565/100);
    static const int qt_gbits = (565/10%10);
    static const int qt_bbits = (565%10);
    static const int qt_red_shift = qt_bbits+qt_gbits-(8-qt_rbits);
    static const int qt_green_shift = qt_bbits-(8-qt_gbits);
    static const int qt_neg_blue_shift = 8-qt_bbits;
    static const int qt_blue_mask = (1<<qt_bbits)-1;
    static const int qt_green_mask = (1<<(qt_gbits+qt_bbits))-((1<<qt_bbits)-1);
    static const int qt_red_mask = (1<<(qt_rbits+qt_gbits+qt_bbits))-(1<<(qt_gbits+qt_bbits));

    const int r=(c & qt_red_mask);
    const int g=(c & qt_green_mask);
    const int b=(c & qt_blue_mask);
    const int tr = r >> qt_red_shift;
    const int tg = g >> qt_green_shift;
    const int tb = b << qt_neg_blue_shift;

    return qRgb(tr,tg,tb);
}

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(QT_CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }

    QImage image = img;
    int    d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
		       (conversion_flags & ColorMode_Mask)==MonoOnly );
    if ( force_mono ) {                         // must be monochrome
	if ( d != 1 ) {
	    image = image.convertDepth( 1, conversion_flags );  // dither
	    d = 1;
	}
    } else {                                    // can be both
	bool conv8 = FALSE;
	if ( d > 8 && dd <= 8 ) {               // convert to 8 bit
	    if ( (conversion_flags & DitherMode_Mask) == AutoDither )
		conversion_flags = (conversion_flags & ~DitherMode_Mask)
				   | PreferDither;
	    conv8 = TRUE;
	} else if ( (conversion_flags & ColorMode_Mask) == ColorOnly ) {
	    conv8 = d == 1;                     // native depth wanted
	} else if ( d == 1 ) {
	    if ( image.numColors() == 2 ) {
		QRgb c0 = image.color(0);       // Auto: convert to best
		QRgb c1 = image.color(1);
		conv8 = QMIN(c0,c1) != qRgb(0,0,0) || QMAX(c0,c1) != qRgb(255,255,255);
	    } else {
		// eg. 1-color monochrome images (they do exist).
		conv8 = TRUE;
	    }
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8, conversion_flags );
	    d = 8;
	}
    }

    if(image.depth()==1) {
	image.setColor( 0, qRgba(255,255,255, 0) );
	image.setColor( 1, qRgba(0,0,0, 0) );
    }

    int w = image.width();
    int h = image.height();

    if ( width() == w && height() == h && ( (d == 1 && depth() == 1) ||
					    (d != 1 && depth() != 1) ) ) {
	// same size etc., use the existing pixmap
	detach();

	if ( data->mask ) {                     // get rid of the mask
	    delete data->mask;
	    data->mask = 0;
	}
    } else {
	// different size or depth, make a new pixmap
	QPixmap pm( w, h, d == 1 ? 1 : -1 );
	pm.data->bitmap = data->bitmap;         // keep is-a flag
	pm.data->optim  = data->optim;          // keep optimization flag
	*this = pm;
    }

    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifdef QMAC_NO_QUARTZ
#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)hd)), *drow;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)hd));

    QRgb q=0;
    int sdpt = image.depth();
    unsigned short sbpr = image.bytesPerLine();
    uchar *sptr = image.bits(), *srow;
    QImage::Endian sord = image.bitOrder();

    char mode = true32b;
    SwapMMUMode(&mode);
    for(int yy=0;yy<h;yy++) {
 	drow = (long *)((char *)dptr + (yy * dbpr));
	srow = sptr + (yy * sbpr);
	for(int xx=0;xx<w;xx++) {
	    switch(sdpt) {
	    case 1:
	    {
		char one_bit = *(srow + (xx / 8));
		if(sord==QImage::BigEndian)
		    one_bit = one_bit >> (7 - (xx % 8));
		else
		    one_bit = one_bit >> (xx % 8);
		q = 0;
		if(!(one_bit & 0x01))
		    q = (255 << 16) | (255 << 8) | 255;
		break;
	    }
	    case 8:
		q = image.color(*(srow + xx));
		break;
	    case 16:
		q = qt_conv16ToRgb(*(((ushort *)srow) + xx));
		break;
	    case 32:
		q = *(((QRgb *)srow) + xx);
		break;
	    default:
		qDebug("Oops: Forgot a depth %s:%d", __FILE__, __LINE__);
		break;
	    }
	    *(drow + xx) = q;
	}
    }
    SwapMMUMode(&mode);
#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif
#else //!QMAC_NO_QUARTZ
    //FIXME
#endif

    data->uninit = FALSE;

    if ( img.hasAlphaBuffer() ) {
	QBitmap m;
	m = img.createAlphaMask( conversion_flags );
	setMask( m );
    }
    return TRUE;
}

int get_index(QImage * qi,QRgb mycol)
{
    int loopc;
    for(loopc=0;loopc<qi->numColors();loopc++) {
	if(qi->color(loopc)==mycol) 
	    return loopc;
    }
    qi->setNumColors(qi->numColors()+1);
    qi->setColor(qi->numColors(),mycol);
    return qi->numColors();
}

QImage QPixmap::convertToImage() const
{
    if ( data->w == 0 ) {
#if defined(QT_CHECK_NULL)
        warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
        QImage nullImage;
        return nullImage;
    }
    if( hd==0 ) {
        QImage nullImage;
        return nullImage;
    }  
    int w = data->w;
    int h = data->h;
    int d = data->d;
    int ncols = 2;

#if 0
    if ( d > 1 && d <= 8 ) {                    // set to nearest valid depth
        d = 8;                                  //   2..7 ==> 8
        ncols = 256;
    } else if ( d > 8 ) {
        d = 32;                                 //   > 8  ==> 32
        ncols = 0;
    }
#else
    if( d != 1 ) { //do we want to FIXME??? Might want to support indexed color modes?
	d = 32;
	ncols = 0;
    }
#endif

    QImage image( w, h, d, ncols, QImage::BigEndian );
    //first we copy the clut
    //handle bitmap case, what about other indexed depths?
    if(d == 1) {
	image.setNumColors( 2 );
	image.setColor( 0, qRgba(255, 255, 255, 0) );
	image.setColor( 1, qRgba(0, 0, 0, 0) );
    } else if(d == 8) {
	//figure out how to copy clut into image FIXME???
    }

    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifdef QMAC_NO_QUARTZ
#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
    QRgb q;
    long *sptr = (long *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)hd)), *srow, r;
    unsigned short sbpr = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)hd));

    char mode = true32b;
    SwapMMUMode(&mode);
    for(int yy=0;yy<h;yy++) {
	srow = (long *)((char *)sptr + (yy * sbpr));
	for(int xx=0;xx<w;xx++) {
	    r = *(srow + xx);
	    q=qRgba((r >> 16) & 0xFF, (r >> 8) & 0xFF, r & 0xFF, /*(r >> 24) & 0xFF*/0 );
	    if(d == 1) {
		image.setPixel(xx, yy, q ? 0 : 1);
	    } else {
		if(ncols) {
		    image.setPixel(xx, yy, get_index(&image,q));
		} else {
		    image.setPixel(xx,yy,q);
		}
	    }
	}
    }
    SwapMMUMode(&mode);

#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif

#else //!QMAC_NO_QUARTZ
    //FIXME
#endif

    //how do I handle a mask?
    const QBitmap* msk = data->mask;
    if (msk) {
	QImage alpha = msk->convertToImage();
	image.setAlphaBuffer( TRUE );
	switch ( d ) {
	case 8: {
	    int used[256];
	    memset( used, 0, sizeof(int)*256 );
	    uchar* p = image.bits();
	    int l = image.numBytes();
	    while (l--) {
		used[*p++]++;
	    }
	    int trans=0;
	    int bestn=INT_MAX;
	    for ( int i=0; i<256; i++ ) {
		if ( used[i] < bestn ) {
		    bestn = used[i];
		    trans = i;
		    if ( !bestn )
			break;
		}
	    }
	    image.setColor( trans, image.color(trans)&0x00ffffff );
	    for ( int y=0; y<image.height(); y++ ) {
		uchar* mb = alpha.scanLine(y);
		uchar* ib = image.scanLine(y);
		uchar bit = 0x80;
		int i=image.width();
		while (i--) {
		    if ( !(*mb & bit) )
			*ib = trans;
		    bit /= 2; if ( !bit ) mb++,bit = 0x80; // ROL
		    ib++;
		}
	    }
	} break;
	case 32: {
	    for ( int y=0; y<image.height(); y++ ) {
		uchar* mb = alpha.scanLine(y);
		QRgb* ib = (QRgb*)image.scanLine(y);
		uchar bit = 0x80;
		int i=image.width();
		while (i--) {
		    if ( *mb & bit )
			*ib |= 0xff000000;
		    else
			*ib &= 0x00ffffff;
		    bit /= 2; if ( !bit ) mb++,bit = 0x80; // ROL
		    ib++;
		}
	    }
	} break;
	}
    }

#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif
    return image;
}

void QPixmap::fill( const QColor &fillColor )
{
    if(!width() || !height())
	return;
    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifdef QMAC_NO_QUARTZ
    //at the end of this function this will go out of scope and the destructor will restore the state
    QMacSavedPortInfo saveportstate(this); 
#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
    detach();					// detach other references
    if(depth() == 1 || depth() == 32) { //small optimization over QD
	ulong *dptr = (ulong *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)hd));
	int dbytes = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)hd))*height();
	if(!dptr || !dbytes)
	    qDebug("Some weirdness! %s %d", __FILE__, __LINE__);
	QRgb colr = qRgba(fillColor.red(),fillColor.green(), fillColor.blue(), 0);
	if(depth() == 1 || !colr) {
	    memset(dptr, colr ? 0xFF : 0x00, dbytes);
	} else if(depth() == 32) {
	    for(int i = 0; i < (int)(dbytes/sizeof(ulong)); i++)
		*(dptr + i) = colr;
	}
    } else {
	Rect r;
	RGBColor rc;
	rc.red=fillColor.red()*256;
	rc.green=fillColor.green()*256;
	rc.blue=fillColor.blue()*256;
	RGBForeColor(&rc);
	SetRect(&r,0,0,width(),height());
	PaintRect(&r);
    }
#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif
#else //!QMAC_NO_QUARTZ
    //FIXME
#endif
}

void QPixmap::detach()
{
    if ( data->uninit || data->count == 1 )
        data->uninit = FALSE;
    else
        *this = copy();
}

int QPixmap::metric(int m) const
{
    int val=0;
    switch ( m ) {
	case QPaintDeviceMetrics::PdmWidth:
	    val = width();
	    break;
	case QPaintDeviceMetrics::PdmHeight:
	    val = height();
	    break;
	case QPaintDeviceMetrics::PdmWidthMM:
	case QPaintDeviceMetrics::PdmHeightMM:
	    break;
	case QPaintDeviceMetrics::PdmNumColors:
	    val = 1 << depth();
	    break;
	case QPaintDeviceMetrics::PdmDpiX: // ### fill with real values!
	case QPaintDeviceMetrics::PdmPhysicalDpiX:
	case QPaintDeviceMetrics::PdmDpiY:
	case QPaintDeviceMetrics::PdmPhysicalDpiY:
	    val = 72;
	    break;
	case QPaintDeviceMetrics::PdmDepth:
	    val = depth();
	    break;
	default:
	    val = 0;
#if defined(QT_CHECK_RANGE)
	    warning( "QPixmap::metric: Invalid metric command" );
#endif
    }
    return val;
}

void QPixmap::deref()
{
    if ( data && data->deref() ) {     // Destroy image if last ref
        if ( data->mask ) {
            delete data->mask;
            data->mask = 0;
        }

        if ( hd && qApp ) {
#ifdef QMAC_NO_QUARTZ
#ifdef QMAC_ONE_PIXEL_LOCK
	    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));
#endif
	    DisposeGWorld((GWorldPtr)hd);    

#else //!QMAC_NO_QUARTZ
	    //FIXME
#endif
        }
        delete data;
	data = NULL;
    }
}

void scaledBitBlt( QPaintDevice *dst, int dx, int dy, int dw, int dh,
		   const QPaintDevice *src, int sx, int sy, int sw, int sh, 
		   Qt::RasterOp rop, bool imask);

QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    int	   w, h;				// size of target pixmap
    int	   ws, hs;				// size of source pixmap
    uchar *dptr;				// data in target pixmap
    int	   dbpl, dbytes;			// bytes per line/bytes total
    uchar *sptr;				// data in original pixmap
    int	   sbpl;				// bytes per line in original
    int	   bpp;					// bits per pixel

    if ( isNull() )				// this is a null pixmap
	return copy();

    ws = width();
    hs = height();

    QWMatrix mat = trueMatrix( matrix, ws, hs ); // true matrix

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix

	h = qRound( mat.m22()*hs );
	w = qRound( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );

	if(w==0 || h==0) 
	    return *this;

	QPixmap pm( w, h, depth(), NormalOptim );
	scaledBitBlt(&pm, 0, 0, w, h, this, 0, 0, width(), height(), Qt::CopyROP, TRUE);
	if ( data->mask ) {
	    QBitmap bm = data->selfmask ? *((QBitmap*)(&pm)) : data->mask->xForm(matrix);
	    pm.setMask( bm );
	}
	return pm;
    } else {					// rotation or shearing
	QPointArray a( QRect(0,0,ws,hs) );
	a = mat.map( a );
	QRect r = a.boundingRect().normalize();
	w = r.width();
	h = r.height();
    }
    bool invertible;
    mat = mat.invert( &invertible );		// invert matrix

    if ( h == 0 || w == 0 || !invertible ) {	// error, return null pixmap
	QPixmap pm;
	pm.data->bitmap = data->bitmap;
	return pm;
    }

#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
    sptr = (uchar *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)hd));
    sbpl = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)hd));
    ws=width();
    hs=height();

    QPixmap pm( w, h, depth(), optimization() );
#ifndef QMAC_ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)pm.handle())));
#endif
    dptr = (uchar *)GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)pm.handle()));
    dbpl = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)pm.handle()));
    bpp = 32;
    dbytes = dbpl*h;

    if ( bpp == 1 ) 
	memset( dptr, 0x00, dbytes );
    else if ( bpp == 8 ) 
	memset( dptr, white.pixel(), dbytes );
    else if( bpp == 32) 
	pm.fill(0x00FFFFFF);
    else 
	memset( dptr, 0xff, dbytes );

    char mode = true32b;
    SwapMMUMode(&mode);
    int	xbpl = bpp == 1 ? ((w+7)/8) : ((w*bpp)/8);
    if ( !qt_xForm_helper( mat, 0, QT_XFORM_TYPE_MSBFIRST, bpp, 
			   dptr, xbpl, dbpl - xbpl, h, sptr, sbpl, ws, hs ) ){
#if defined(QT_CHECK_RANGE)
	qWarning( "QPixmap::xForm: display not supported (bpp=%d)",bpp);
#endif
	QPixmap pm;
	return pm;
    }
    SwapMMUMode(&mode);
#ifndef QMAC_ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
    UnlockPixels(GetGWorldPixMap((GWorldPtr)pm.handle()));    
#endif

    if ( depth() == 1 ) {
	if ( data->mask ) {
	    if ( data->selfmask )               // pixmap == mask
		pm.setMask( *((QBitmap*)(&pm)) );
	    else
		pm.setMask( data->mask->xForm(matrix) );
	}
    } else if ( data->mask ) {
	pm.setMask( data->mask->xForm(matrix) );
    }
    return pm;
}

void QPixmap::init( int w, int h, int d, bool bitmap, Optimization optim )
{
    if(d != 32 && d != 1)
	d = 32; //magic number.. we always use a 32 bit depth for non-bitmaps

    static int serial = 0;
    
    hd = 0;
    data = new QPixmapData;
    Q_CHECK_PTR( data );
    memset( data, 0, sizeof(QPixmapData) );
    data->count=1;
    data->uninit=TRUE;
    data->bitmap=bitmap;
    data->clut = NULL;
    data->ser_no=++serial;
    data->optim=optim;

    int dd = 32; //magic number? 32 seems to be default?
    bool make_null = w == 0 || h == 0;		// create null pixmap
    if ( d == 1 )				// monocrome pixmap
	data->d = 1;
    else if ( d < 0 || d == dd )		// def depth pixmap
	data->d = dd;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
	hd = 0;
#if defined(QT_CHECK_RANGE)
	if ( !make_null )
	    qWarning( "QPixmap: Invalid pixmap parameters" );
#endif
	return;
    }

    if(w<1 || h<1) 
	return;
    data->w=w;
    data->h=h;

#ifdef QMAC_NO_QUARTZ
    Rect rect;
    SetRect(&rect,0,0,w,h);
    QDErr e = -1;
    const int params = alignPix | stretchPix | newDepth;
#if 1
    if(optim == BestOptim) //try to get it into distant memory
	e = NewGWorld((GWorldPtr *)&hd, 32, &rect, 
		      data->clut ? &data->clut : NULL, 0, useDistantHdwrMem | params);
    if(e != noErr) //oh well I tried
#endif  
	e = NewGWorld((GWorldPtr *)&hd, 32, &rect, 
		      data->clut ? &data->clut : NULL, 0, params);

    /* error? */
    if(e != noErr) {
	data->w = data->h = 0;
	hd=0; //just to be sure
	qDebug( "QPixmap::init Something went wrong");
	Q_ASSERT(0);
    } else {
#ifdef QMAC_ONE_PIXEL_LOCK
	Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));
#endif
	data->w=w;
	data->h=h;
    }

#else //!QMAC_NO_QUARTZ
    //FIXME
#endif
}

int QPixmap::defaultDepth()
{
    GDHandle gd;
    gd=GetMainDevice();
    int wug=(**gd).gdCCDepth;
    if(wug) {
	return wug;
    } else {
	return 16;
    }
}

void QPixmap::setOptimization( Optimization  )
{
}

QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    QPixmap pm;
    QWidget *widget = QWidget::find( window );
    if ( widget ) {
	if(w == -1)
	    w = widget->width() - x;
	if(h == -1)
	    h = widget->height() - y;
	pm = QPixmap(w, h, 32);
	bitBlt(&pm, 0, 0, widget, x, y, w, h);
    }
    return pm;
}


bool QPixmap::hasAlpha() const
{
    return data->mask;
}

