#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qwmatrix.h"
#include "qt_mac.h"

QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPaintDevice( QInternal::Pixmap )
{
  init(w, h, 1, TRUE, DefaultOptim);

  data->uninit = FALSE;
  data->w = w;
  data->h = h;
  data->d = 1;

  //at the end of this function this will go out of scope and the destructor will restore the state
  QMacSavedPortInfo saveportstate; 

  SetGWorld((GWorldPtr)hd,0);
  Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));

  RGBColor tmpc;
  tmpc.red = tmpc.green = tmpc.blue = 0;
  RGBForeColor(&tmpc);
  tmpc.red = tmpc.green = tmpc.blue = ~0;
  RGBBackColor( &tmpc );

  // Slow and icky
  RGBColor r;
  for(int y=0;y<h;y++) {
    int sy = y * (w / 8);
    for(int x=0;x<w;x++) {
      char one_bit;
      if(isXbitmap)
	one_bit = (*(bits + (sy + (x / 8))) >> (7 - (x % 8))) & 0x01;
      else
	one_bit = (*(bits + (sy + (x / 8))) >> (x % 8)) & 0x01;
      r.green = r.blue = r.red = one_bit ? 255 : 0;
      SetCPixel(x,y,&r);
    }
  }
  UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
}

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(QT_CHECK_NULL)
	warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return FALSE;
    }

    //FIXME@!!!!!!@$!@$!@#
    init( img.width(), img.height(), 32, isQBitmap(), DefaultOptim);
    if(!hd)
	return FALSE;

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

    if ( d == 1 )                               // 1 bit pixmap (bitmap)
	image = image.convertBitOrder( QImage::BigEndian );

 
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


    //at the end of this function this will go out of scope and the destructor will restore the state
    QMacSavedPortInfo saveportstate; 

    SetGWorld((GWorldPtr)hd,0);
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));

    RGBColor tmpc;
    tmpc.red = tmpc.green = tmpc.blue = 0;
    RGBForeColor(&tmpc);
    tmpc.red = tmpc.green = tmpc.blue = ~0;
    RGBBackColor( &tmpc );

    //OPTIMIZATION FIXME, we should not be iterating all the pixels, fix this on optimization pass
    RGBColor r;
    int loopc,loopc2;
    QRgb q;
    for(loopc=0;loopc<image.width();loopc++) {
	for(loopc2=0;loopc2<image.height();loopc2++) {
	    q=image.pixel(loopc,loopc2);
	    r.red=qRed(q)*256;
	    r.green=qGreen(q)*256;
	    r.blue=qBlue(q)*256;
	    SetCPixel(loopc,loopc2,&r);
	}
    }
    data->uninit = FALSE;

    if ( img.hasAlphaBuffer() ) {
	QBitmap m;
	m = img.createAlphaMask( conversion_flags );
	setMask( m );
    }

    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
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
    //do we want to FIXME??? Might want to support indexed color modes?
    if( depth() != 1 ) {
	d = 32;
	ncols = 0;
    }
#endif

    QImage * image=new QImage( w, h, d, ncols, QImage::BigEndian );

    //first we copy the clut
    //handle bitmap case, what about other indexed depths?
    if(d == 1) {
	image->setNumColors( 2 );
	image->setColor( 0, qRgba(255,255,255, 0) );
	image->setColor( 1, qRgba(0,0,0, 0) );
    } else if(d == 8) {
	//figure out how to copy clut into image FIXME???
    }

    //at the end of this function this will go out of scope and the destructor will restore the state
    QMacSavedPortInfo saveportstate; 

    SetGWorld((GWorldPtr)hd,0);
    Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));

    RGBColor tmpc;
    tmpc.red = tmpc.green = tmpc.blue = 0;
    RGBForeColor(&tmpc);
    tmpc.red = tmpc.green = tmpc.blue = ~0;
    RGBBackColor( &tmpc );

    //OPTIMIZATION FIXME, we should not be iterating all the pixels, fix this on optimization pass
    RGBColor r;
    int loopc,loopc2;
    QRgb q;
    for(loopc=0;loopc<w;loopc++) {
	for(loopc2=0;loopc2<h;loopc2++) {
	    GetCPixel(loopc,loopc2,&r);
	    q=qRgba(r.red/256,r.green/256,r.blue/256, 0); //FIXME, should I be doing that to the alpha?
	    if(ncols) {
		image->setPixel(loopc,loopc2,get_index(image,q));
	    } else {
		image->setPixel(loopc,loopc2,q);
	    }
	}
    }

    //how do I handle a mask?
    const QBitmap* msk = mask();

    if (msk) {
	QImage alpha = msk->convertToImage();
//	bool ale = alpha.bitOrder() == QImage::LittleEndian;

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

    UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
    return *image;
}

void QPixmap::fill( const QColor &fillColor )
{
    if(hd) {
	Rect r;
	RGBColor rc;

	//at the end of this function this will go out of scope and the destructor will restore the state
	QMacSavedPortInfo saveportstate; 

	SetGWorld((GWorldPtr)hd,0);
	Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)hd)));

	rc.red=fillColor.red()*256;
	rc.green=fillColor.green()*256;
	rc.blue=fillColor.blue()*256;
	RGBForeColor(&rc);
	SetRect(&r,0,0,width(),height());
	PaintRect(&r);

	UnlockPixels(GetGWorldPixMap((GWorldPtr)hd));    
    }
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
    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth || m == QPaintDeviceMetrics::PdmHeight ) {
        if ( m == QPaintDeviceMetrics::PdmWidth )
            val = width();
        else
            val = height();
    } else {
        switch ( m ) {
	case QPaintDeviceMetrics::PdmWidthMM:
	    break;
	case QPaintDeviceMetrics::PdmHeightMM:
	    break;
	case QPaintDeviceMetrics::PdmNumColors:
	    val = 1 << depth();
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
        if ( hd && qApp ) {
            DisposeGWorld((GWorldPtr)hd);
            hd = 0;
        }
        delete data;
    }
}

void scaledBitBlt( QPaintDevice *dst, int dx, int dy, int dw, int dh,
		   const QPaintDevice *src, int sx, int sy, int sw, int sh, 
		   Qt::RasterOp rop, bool imask);

QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
#if 0
    qDebug("Grr.. I really need to work on this function..");
    return *this;
#endif

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

    const double dt = 0.0001;
    double x1,y1, x2,y2, x3,y3, x4,y4;		// get corners
    double xx = (double)ws - 1;
    double yy = (double)hs - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    double ymin = y1;				// lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    double xmin = x1;				// lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );	// true matrix
    mat = matrix * mat;

    if ( matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
	 matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F ) {
	if ( mat.m11() == 1.0F && mat.m22() == 1.0F )
	    return *this;			// identity matrix

	h = qRound( mat.m22()*hs );
	w = qRound( mat.m11()*ws );
	h = QABS( h );
	w = QABS( w );

	if(w==0 || h==0) {
	    return *this;
	}

	QPixmap pm( w, h, depth(), NormalOptim );
	scaledBitBlt(&pm, 0, 0, w, h, this, ws, hs, width(), height(), Qt::CopyROP, TRUE);

	if ( 0 && data->mask ) {
	    QBitmap bm =
		data->selfmask ? *((QBitmap*)(&pm)) : data->mask->xForm(matrix);
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

    QImage destImg;
    QPixmap pm( 1, 1, depth(), data->bitmap, NormalOptim );
    pm.data->uninit = FALSE;
    destImg.create( w, h, srcImg.depth(), srcImg.numColors(), srcImg.bitOrder() );
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
			 (1 << ((trigx>>16)&7)) )                             \
			*p |= b;                                              \
		}                                                             \
		trigx += m11;                                                 \
		trigy += m12;
		// END OF MACRO
		IWX(1)
		IWX(2)
		IWX(4)
		IWX(8)
		IWX(16)
		IWX(32)
		IWX(64)
		IWX(128)
		p++;
	    }
	}
	m21ydx += m21;
	m22ydy += m22;
	p += p_inc;
    }

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
  static int serial = 0;
    
  data = new QPixmapData;
  Q_CHECK_PTR( data );
  memset( data, 0, sizeof(QPixmapData) );
  data->count=1;
  data->uninit=TRUE;
  data->bitmap=bitmap;
  data->clut = NULL;
  data->ser_no=++serial;
  data->optim=optim;

  data->d = d;
    
  hd=0;
  if(w>1024 || h > 1024) {
    hd=0;
    return;
  }
  if(w<1 || h<1) {
    hd=0;
    return;
  }

  if(d<1) {
    d=defaultDepth();
  }
  if(w==0 && h==0) {
    data->w=data->h=0;
//    data->d=0;
    return;
  }
  data->w=0;
  data->h=0;
  QDErr e;
  GWorldFlags someflags;
  Rect rect;
  // God knows what this does
  someflags=alignPix | stretchPix | newDepth;
  SetRect(&rect,0,0,w,h);

  /* actually create world */
  e=NewGWorld( (GWorldPtr *)&hd, 0, &rect, data->clut ? &data->clut : NULL, 0, someflags );

  /* error? */
  if((e & gwFlagErr)!=0) {
    qDebug( "QPixmap::init Something went wrong" );
    Q_ASSERT(0);
    hd=0;
  } else {
    data->w=w;
    data->h=h;
  }
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

QWMatrix QPixmap::trueMatrix(const QWMatrix & matrix,int w,int h)
{
    const double dt = (double)0.0001;
    double x1,y1, x2,y2, x3,y3, x4,y4;          // get corners
    double xx = (double)w - 1;
    double yy = (double)h - 1;

    matrix.map( dt, dt, &x1, &y1 );
    matrix.map( xx, dt, &x2, &y2 );
    matrix.map( xx, yy, &x3, &y3 );
    matrix.map( dt, yy, &x4, &y4 );

    double ymin = y1;                           // lowest y value
    if ( y2 < ymin ) ymin = y2;
    if ( y3 < ymin ) ymin = y3;
    if ( y4 < ymin ) ymin = y4;
    double xmin = x1;                           // lowest x value
    if ( x2 < xmin ) xmin = x2;
    if ( x3 < xmin ) xmin = x3;
    if ( x4 < xmin ) xmin = x4;

    QWMatrix mat( 1, 0, 0, 1, -xmin, -ymin );   // true matrix
    mat = matrix * mat;
    return mat;
}

void QPixmap::setOptimization( Optimization  )
{
}

QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
    QPixmap pm;
#if 0     //FIXME FIXME FIXME, THIS DOES NOT WORK FIXME!
    QWidget *widget = QWidget::find( window );
    if ( widget ) {
	pm = QPixmap(w, h, 32);
	bitBlt(&pm, 0, 0, widget, x, y, w, h);
    }
#endif
    return pm;
}





  

