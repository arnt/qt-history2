#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qwmatrix.h"
#include "qt_mac.h"

#ifdef QMAC_VIRTUAL_PIXMAP_SUPPORT
#include <qintcache.h>

class QMacInternalPixmapCache : public QIntCache<QPixmap>
{
public:
    static int pixmap_key, cache_used;
    QMacInternalPixmapCache() : QIntCache<QPixmap>(640*480*4, 149) { } //640x480 32bit
    ~QMacInternalPixmapCache() { }
    Qt::HANDLE getGWorld(const QPixmap *);
protected:
    virtual void deleteItem(Item);
};

void QMacInternalPixmapCache::deleteItem(Item d)
{
    QPixmap *p = (QPixmap *)d;
    if(p->data->cache.gworld) {
	p->data->cache.img = new QImage(p->convertToImage());
#ifdef ONE_PIXEL_LOCK
	UnlockPixels(GetGWorldPixMap((GWorldPtr)p->data->cache.gworld));
#endif
	DisposeGWorld((GWorldPtr)p->data->cache.gworld);
	p->data->cache.gworld = NULL;
	p->data->cache.key = -1;
    }
}
Qt::HANDLE QMacInternalPixmapCache::getGWorld(const QPixmap *p)
{
    QPixmap *pm = const_cast<QPixmap *>(p); //mutable
    if(pm->data->cache.gworld) {
	find(pm->data->cache.key); //most recently used now baby!
	return pm->data->cache.gworld;
    }

    //not in the cache already..
    int w = pm->width(), h = pm->height();
    if(w<1 || h<1) 
	return NULL;
    Rect rect;
    SetRect(&rect,0,0,w,h);
    int cost = w * h * pm->depth() / 4; //some cost

    for(int tries = 1; TRUE; tries++) {
	QDErr e = 0;
	const int params = alignPix | stretchPix | newDepth;
#if 0    
	if(w <= 300 && h <= 100) //try to get it into distant memory
	    e = NewGWorld((GWorldPtr *)&pm->data->cache.gworld, 32, &rect, pm->data->clut ? &pm->data->clut : NULL, 0, 
			  useDistantHdwrMem | params);
	if(!pm->data->cache.gworld) //oh well I tried
#endif  
	    e = NewGWorld((GWorldPtr *)&pm->data->cache.gworld, 32, &rect, pm->data->clut ? &pm->data->clut : NULL, 0,
			  params);

	/* error? */
	if((e & gwFlagErr)!=0) {
	    pm->data->cache.gworld=0; //just to be sure
	    if(int mc = maxCost()) {
		setMaxCost(mc > (cost * tries) ? mc - (cost*tries) : 0); //throw away some things
		setMaxCost(mc + (cost * tries)); //get ready to try again
	    } else {
		qDebug( "QPixmap::init Something went wrong");
		Q_ASSERT(0);
		break;
	    }
	} else {
#ifdef ONE_PIXEL_LOCK
	    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)pm->data->cache.gworld)));
#endif
	    cache_used += cost;
	    if(cache_used > maxCost())
		setMaxCost(cache_used);
	    insert(pm->data->cache.key = pixmap_key++, p, cost);
	    if(pm->data->cache.img) { 	    //back like it was before
		pm->convertFromImage(*pm->data->cache.img);
		delete pm->data->cache.img;
		pm->data->cache.img = NULL;
	    }
	    break;
	}
    }
    return pm->data->cache.gworld;
}
int QMacInternalPixmapCache::pixmap_key = 0;
int QMacInternalPixmapCache::cache_used = 0;
static QMacInternalPixmapCache g_pixmap_cache;

/*! \internal
  This is conditionally implemented to support virtual pixmaps.
*/
Qt::HANDLE QPixmap::handle() const
{
    Qt::HANDLE hd = g_pixmap_cache.getGWorld(this);
    return hd;
}
#endif

extern const uchar *qt_get_bitflip_array();		// defined in qimage.cpp

QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( QInternal::Pixmap )
{
    init(w, h, 1, TRUE, DefaultOptim);
    GWorldPtr hd = (GWorldPtr)handle();
    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifndef ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap(hd)));
#endif

    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap(hd)), *drow, q;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(hd));

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
#ifndef ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap(hd));    
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

    GWorldPtr hd = (GWorldPtr)handle();
    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifndef ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap(hd)));
#endif

    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap(hd)), *drow;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(hd));

    QRgb q;
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
    data->uninit = FALSE;

    if ( img.hasAlphaBuffer() ) {
	QBitmap m;
	m = img.createAlphaMask( conversion_flags );
	setMask( m );
    }

#ifndef ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap(hd));    
#endif
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
    if ( isNull() ) {
#if defined(QT_CHECK_NULL)
        warning( "QPixmap::convertToImage: Cannot convert a null pixmap" );
#endif
        QImage nullImage;
        return nullImage;
    }
    GWorldPtr hd = (GWorldPtr)handle();
    if( hd==0 ) {
        QImage nullImage;
        return nullImage;
    }  
    int w = width();
    int h = height();
    int d = depth();
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

    QImage * image=new QImage( w, h, d, ncols, QImage::BigEndian );
    //first we copy the clut
    //handle bitmap case, what about other indexed depths?
    if(d == 1) {
	image->setNumColors( 2 );
	image->setColor( 0, qRgba(255, 255, 255, 0) );
	image->setColor( 1, qRgba(0, 0, 0, 0) );
    } else if(d == 8) {
	//figure out how to copy clut into image FIXME???
    }

    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);

#ifndef ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap(hd)));
#endif

    QRgb q;
    long *sptr = (long *)GetPixBaseAddr(GetGWorldPixMap(hd)), *srow, r;
    unsigned short sbpr = GetPixRowBytes(GetGWorldPixMap(hd));
    char mode = true32b;
    SwapMMUMode(&mode);

    for(int yy=0;yy<h;yy++) {
	srow = (long *)((char *)sptr + (yy * sbpr));
	for(int xx=0;xx<w;xx++) {
	    r = *(srow + xx);
	    q=qRgba((r >> 16) & 0xFF, (r >> 8) & 0xFF, r & 0xFF, /*(r >> 24) & 0xFF*/0 );
	    if(d == 1) {
		image->setPixel(xx, yy, q ? 0 : 1);
	    } else {
		if(ncols) {
		    image->setPixel(xx, yy, get_index(image,q));
		} else {
		    image->setPixel(xx,yy,q);
		}
	    }
	}
    }

    SwapMMUMode(&mode);
#ifndef ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap(hd));    
#endif

    //how do I handle a mask?
    const QBitmap* msk = mask();

    if (msk) {
	QImage alpha = msk->convertToImage();
	image->setAlphaBuffer( TRUE );
	switch ( d ) {
	case 8: {
	    int used[256];
	    memset( used, 0, sizeof(int)*256 );
	    uchar* p = image->bits();
	    int l = image->numBytes();
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
	    image->setColor( trans, image->color(trans)&0x00ffffff );
	    for ( int y=0; y<image->height(); y++ ) {
		uchar* mb = alpha.scanLine(y);
		uchar* ib = image->scanLine(y);
		uchar bit = 0x80;
		int i=image->width();
		while (i--) {
		    if ( !(*mb & bit) )
			*ib = trans;
		    bit /= 2; if ( !bit ) mb++,bit = 0x80; // ROL
		    ib++;
		}
	    }
	} break;
	case 32: {
	    for ( int y=0; y<image->height(); y++ ) {
		uchar* mb = alpha.scanLine(y);
		QRgb* ib = (QRgb*)image->scanLine(y);
		uchar bit = 0x80;
		int i=image->width();
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

#ifndef ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap(hd));    
#endif
    return *image;
}

void QPixmap::fill( const QColor &fillColor )
{
    GWorldPtr hd = (GWorldPtr)handle();
    if(!hd)
	qDebug("Some weirdness! %s %d", __FILE__, __LINE__);
	
    Rect r;
    RGBColor rc;

    //at the end of this function this will go out of scope and the destructor will restore the state
    QMacSavedPortInfo saveportstate(this); 
#ifndef ONE_PIXEL_LOCK
    Q_ASSERT(LockPixels(GetGWorldPixMap(hd)));
#endif

    rc.red=fillColor.red()*256;
    rc.green=fillColor.green()*256;
    rc.blue=fillColor.blue()*256;
    RGBForeColor(&rc);
    SetRect(&r,0,0,width(),height());
    PaintRect(&r);

#ifndef ONE_PIXEL_LOCK
    UnlockPixels(GetGWorldPixMap(hd));    
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
    // Destroy image if last ref
    if ( data && data->deref() ) {                      // last reference lost
        if ( data->mask ) {
            delete data->mask;
            data->mask = 0;
        }
#ifdef QMAC_VIRTUAL_PIXMAP_SUPPORT
	g_pixmap_cache.take(data->cache.key);
        if ( data->cache.gworld && qApp ) {
#ifdef ONE_PIXEL_LOCK
	    UnlockPixels(GetGWorldPixMap((GWorldPtr)data->cache.gworld));
#endif
            DisposeGWorld((GWorldPtr)data->cache.gworld);
            data->cache.gworld = NULL;
        }
	delete data->cache.img;
	data->cache.img = NULL;
#else
        if ( hd && qApp ) {
#ifdef ONE_PIXEL_LOCK
	    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
#endif
            DisposeGWorld((GWorldPtr)hd);
            hd = 0;
        }
#endif
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
    bool   depth1 = depth() == 1;
    int	   y;

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

    QImage srcImg = convertToImage();
    sptr=srcImg.scanLine(0);
    sbpl=srcImg.bytesPerLine();
    ws=width();
    hs=height();

    QImage destImg( w, h, srcImg.depth(), srcImg.numColors(), srcImg.bitOrder() );
    //FIXME: we don't want a static color here it belongs in image creation
    if (destImg.depth() == 1) {
	destImg.setNumColors( 2 );
	destImg.setColor( 0, qRgba(255,255,255, 0) );
	destImg.setColor( 1, qRgba(0,0,0, 0) );
    }
    dptr=destImg.scanLine(0);
    dbpl=destImg.bytesPerLine();
    bpp=destImg.depth();

    dbytes = dbpl*h;

    if ( depth1 )
	memset( dptr, 0x00, dbytes );
    else if ( bpp == 8 )
	memset( dptr, white.pixel(), dbytes );
    else if ( bpp == 32 ) {
	destImg.fill( 0x00FFFFFF );
    } else
	memset( dptr, 0xff, dbytes );

    int m11 = qRound((double)mat.m11()*65536.0);
    int m12 = qRound((double)mat.m12()*65536.0);
    int m21 = qRound((double)mat.m21()*65536.0);
    int m22 = qRound((double)mat.m22()*65536.0);
    int dx  = qRound((double)mat.dx() *65536.0);
    int dy  = qRound((double)mat.dy() *65536.0);

    int	  m21ydx = dx, m22ydy = dy;
    uint  trigx, trigy;
    uint  maxws = ws<<16, maxhs=hs<<16;
    uchar *p	= dptr;
    int	  xbpl, p_inc;

    if ( depth1 ) {
	xbpl  = (w+7)/8;
	p_inc = dbpl - xbpl;
    } else {
	xbpl  = (w*bpp)/8;
	p_inc = dbpl - xbpl;
    }

    for ( y=0; y<h; y++ ) {			// for each target scanline
	trigx = m21ydx;
	trigy = m22ydy;
	uchar *maxp = p + xbpl;
	if ( !depth1 ) {
	    switch ( bpp ) {
		case 8:				// 8 bpp transform
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*p = *(sptr+sbpl*(trigy>>16)+(trigx>>16));
		    trigx += m11;
		    trigy += m12;
		    p++;
		}
		break;

		case 16:			// 16 bpp transform
		while ( p < maxp ) {
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
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs ) {
			p2 = sptr+sbpl*(trigy>>16) + ((trigx>>16)*3);
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
		while ( p < maxp ) {
		    if ( trigx < maxws && trigy < maxhs )
			*((uint*)p) = *((uint *)(sptr+sbpl*(trigy>>16) +
						   ((trigx>>16)<<2)));
		    trigx += m11;
		    trigy += m12;
		    p += 4;
		}
		break;

		default: {
#if defined(QT_CHECK_RANGE)
		qWarning( "QPixmap::xForm: Display not supported (bpp=%d)",bpp);
#endif
		return QPixmap( 0, 0, 0, data->bitmap, data->optim );
		}
	    }
	} else {
	    // mono bitmap LSB first
	    while ( p < maxp ) {
#undef IWX
#define IWX(b)  if ( trigx < maxws && trigy < maxhs ) {                       \
		    if ( *(sptr+sbpl*(trigy>>16)+(trigx>>19)) &               \
			 (1 << (7-((trigx>>16)&7))) )                             \
			*p |= b;                                              \
		}                                                             \
		trigx += m11;                                                 \
		trigy += m12;
		// END OF MACRO
		IWX(128)
		IWX(64)
		IWX(32)
		IWX(16)
		IWX(8)
		IWX(4)
		IWX(2)
		IWX(1)
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }

    QPixmap pm( destImg.width(), destImg.height(), depth(), data->bitmap, NormalOptim );
    pm.convertFromImage( destImg );
    if ( depth1 ) {
	if ( data->mask ) {
	    if ( data->selfmask )               // pixmap == mask
		pm.setMask( *((QBitmap*)(&pm)) );
	    else
		pm.setMask( data->mask->xForm(matrix) );
	}
    } else {
	if ( data->mask )
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
#ifdef QMAC_VIRTUAL_PIXMAP_SUPPORT
    data->cache.img = NULL;
    data->cache.gworld = NULL;
    data->cache.key = -1;
#endif
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


  

