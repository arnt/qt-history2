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
    init( 0, 0, 0, FALSE, NormalOptim );
    if ( w <= 0 || h <= 0 )                     // create null pixmap
        return;

    data->uninit = FALSE;
    data->w = w;
    data->h = h;
    data->d = 1;
}

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
        warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
        return FALSE;
    }
    if(hd==0) {
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

    if(hd) {
	SetGWorld((GWorldPtr)hd,0);
    }

    // Slow and icky, but the proper way crashed
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

    return TRUE;
}

int get_index(QImage * qi,QRgb mycol)
{
    int loopc;
    for(loopc=0;loopc<qi->numColors();loopc++) {
	if(qi->color(loopc)==mycol) {
	    return loopc;
	}
    }
    qi->setNumColors(qi->numColors()+1);
    qi->setColor(qi->numColors(),mycol);
    return qi->numColors();
}

QImage QPixmap::convertToImage() const
{
    if ( isNull() ) {
#if defined(CHECK_NULL)
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

    if ( d > 1 && d <= 8 ) {                    // set to nearest valid depth
        d = 8;                                  //   2..7 ==> 8
        ncols = 256;
    } else if ( d > 8 ) {
        d = 32;                                 //   > 8  ==> 32
        ncols = 0;
    }

    QImage * image=new QImage( w, h, d, ncols, QImage::BigEndian );

    if(hd) {
	SetGWorld((GWorldPtr)hd,0);
    } else {
	QImage nullImage;
	return nullImage;
    }

    // Slow and icky, but the proper way crashed
    RGBColor r;
    int loopc,loopc2;
    QRgb q;
    for(loopc=0;loopc<w;loopc++) {
	for(loopc2=0;loopc2<h;loopc2++) {
	    GetCPixel(loopc,loopc2,&r);
	    q=qRgb(r.red/256,r.green/256,r.blue/256);
	    if(ncols) {
		image->setPixel(loopc,loopc2,get_index(&image,q));
	    } else {
		image->setPixel(loopc,loopc2,q);
	    }
	}
    }

#if 0
#error "Need to take QPixmap::mask() into account here, "\
            "by adding a transparent color (if possible), and "\
            "changing masked-out pixels to that color index."

#endif


    for ( int i=0; i<ncols; i++ ) {             // copy color table
	image.setColor( i, qRgb(0,
				0,
				0));
    }

    QImage * flippy=new QImage();
    *flippy=image;
    return *flippy;
}

void QPixmap::fill( const QColor &fillColor )
{
    if(hd) {
	Rect r;
	RGBColor rc;
	SetPort((GrafPort *)hd);
	rc.red=fillColor.red()*256;
	rc.green=fillColor.green()*256;
	rc.blue=fillColor.blue()*256;
	RGBForeColor(&rc);
	SetRect(&r,0,0,width(),height());
	PaintRect(&r);
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
#if defined(CHECK_RANGE)
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

QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
    QPixmap ret=*this;
    return ret;
}

void QPixmap::init( int w, int h, int d, bool bitmap, Optimization optim )
{
    // Hmm.
    hd=0;
    if(w>1024 || h > 1024) {
	hd=0;
	return;
    }
    if(w<1 || h<1) {
	hd=0;
	return;
    }

    static int serial = 0;
    
    data = new QPixmapData;
    CHECK_PTR( data );
    memset( data, 0, sizeof(QPixmapData) );
    data->count=1;
    data->uninit=TRUE;
    data->bitmap=bitmap;
    data->ser_no=++serial;
    data->optim=optim;
    
    if(d<1) {
	d=defaultDepth();
    }
    if(w==0 && h==0) {
	data->w=data->h=0;
	data->d=0;
	return;
    }
    data->w=0;
    data->h=0;
    QDErr e;
    GWorldFlags someflags;
    Rect rect;
    // God knows what this does
    someflags=alignPix | stretchPix | newDepth;
    // Depth of 0=deepest screen depth
    SetRect(&rect,0,0,w,h);
    e=NewGWorld((CGrafPort **)&hd,d,&rect,0,0,someflags);
    if((e & gwFlagErr)!=0) {
	// Something went wrong
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

void QPixmap::setOptimization( Optimization optimization )
{
}






