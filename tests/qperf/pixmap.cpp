#include "qperf.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qimage.h>


QPixmap *pm_64x64;
QPixmap *pm_256x256;
QBitmap *bm_64x64;
QBitmap *bm_256x256;
QPixmap *pmm_64x64;
QPixmap *pmm_256x256;
QBitmap *bmm_64x64;
QBitmap *bmm_256x256;

QString  pm_64x64_file;
QString  pm_256x256_file;

static void pixmap_init()
{
    QPainter p;
    QString  fn;

    pm_64x64 = new QPixmap(64,64);
    pm_64x64->fill( Qt::yellow );
    p.begin( pm_64x64 );
    p.setPen( Qt::blue );
    p.drawLine( pm_64x64->rect().topLeft(),
		pm_64x64->rect().bottomRight() );
    p.drawLine( pm_64x64->rect().bottomLeft(),
		pm_64x64->rect().topRight() );
    p.end();
    pm_64x64_file = "pm64";
    pm_64x64_file += qperf_imageExt();
    pm_64x64->save( pm_64x64_file, qperf_imageFormat() );

    pm_256x256 = new QPixmap(256,256);
    pm_256x256->fill( Qt::red );
    p.begin( pm_256x256 );
    p.setPen( Qt::white );
    p.drawLine( pm_256x256->rect().topLeft(),
		pm_256x256->rect().bottomRight() );
    p.drawLine( pm_256x256->rect().bottomLeft(),
		pm_256x256->rect().topRight() );
    p.end();
    pm_256x256_file = "pm256";
    pm_256x256_file += qperf_imageExt();
    pm_256x256->save( pm_256x256_file, qperf_imageFormat() );

    bm_64x64 = new QBitmap( 64, 64 );
    bm_64x64->fill( Qt::color0 );
    p.begin( bm_64x64 );
    p.setBrush( Qt::color1 );
    p.drawEllipse( bm_64x64->rect() );
    p.end();

    bm_256x256 = new QBitmap( 256, 256 );
    bm_256x256->fill( Qt::color0 );
    p.begin( bm_256x256 );
    p.setBrush( Qt::color1 );
    p.drawEllipse( bm_256x256->rect() );
    p.end();

    pmm_64x64 = new QPixmap( *pm_64x64 );
    pmm_64x64->setMask( *bm_64x64 );

    pmm_256x256 = new QPixmap( *pm_256x256 );
    pmm_256x256->setMask( *bm_256x256 );

    bmm_64x64 = new QBitmap( *bm_64x64 );
    bmm_64x64->setMask( *bmm_64x64 );

    bmm_256x256 = new QBitmap( *bm_256x256 );
    bmm_256x256->setMask( *bmm_256x256 );
}

static int pixmap_create_null()
{
    int i;
    for ( i=0; i<10000; i++ ) {
	QPixmap nullPixmap;
    }
    return i;
}

static int pixmap_create_64x64()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	QPixmap pm(64,64);
    }
    return i;
}

static int pixmap_create_256x256()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	QPixmap pm(256,256);
    }
    return i;
}

static int pixmap_fill_64x64()
{
    int i;
    QPixmap pm(64,64);
    for ( i=0; i<1000; i++ ) {
	pm.fill( Qt::white );
    }
    return i;
}

static int pixmap_fill_256x256()
{
    int i;
    QPixmap pm(256,256);
    for ( i=0; i<1000; i++ ) {
	pm.fill( Qt::white );
    }
    return i;
}

static int pixmap_bitblt( QPixmap *pm, int w, int h, int numIter=1000 )
{
    int i;
    QPainter *p = qperf_painter();
    QPaintDevice *pd = qperf_paintDevice();
    for ( i=0; i<numIter; i++ ) {
	bitBlt( pd, qrnd(640-w), qrnd(480-h), pm );
	//p->drawPixmap( qrnd(640-w), qrnd(480-h), *pm );
    }
    return i;
}

static int pixmap_bitblt_64x64()
{
    return pixmap_bitblt( pm_64x64, 64, 64 );
}

static int pixmap_bitblt_256x256()
{
    return pixmap_bitblt( pm_256x256, 256, 256 );
}

static int pixmap_monobitblt_64x64()
{
    return pixmap_bitblt( bm_64x64, 64, 64, 500 );
}

static int pixmap_monobitblt_256x256()
{
    return pixmap_bitblt( bm_256x256, 256, 256, 500 );
}

static int pixmap_maskblt_64x64()
{
    return pixmap_bitblt( pmm_64x64, 64, 64, 100 );
}

static int pixmap_maskblt_256x256()
{
    return pixmap_bitblt( pmm_256x256, 256, 256, 100 );
}

static int pixmap_selfmaskblt_64x64()
{
    return pixmap_bitblt( bmm_64x64, 64, 64, 100 );
}

static int pixmap_selfmaskblt_256x256()
{
    return pixmap_bitblt( bmm_256x256, 256, 256, 100 );
}

static int pixmap_to_image_64x64()
{
    int i;
    QImage image;
    for ( i=0; i<50; i++ ) {
	image = *pm_64x64;
    }
    return i;
}

static int pixmap_to_image_256x256()
{
    int i;
    QImage image;
    for ( i=0; i<50; i++ ) {
	image = *pm_256x256;
    }
    return i;
}

static int pixmap_from_image_64x64()
{
    int i;
    QImage image;
    image = *pm_64x64;
    QPixmap pm;
    for ( i=0; i<50; i++ ) {
	pm = image;
    }
    return i;
}

static int pixmap_from_image_256x256()
{
    int i;
    QImage image;
    image = *pm_256x256;
    QPixmap pm;
    for ( i=0; i<50; i++ ) {
	pm = image;
    }
    return i;
}

static int pixmap_load( const char *fileName, int numIter=10 )
{
    int i;
    QPixmap pm;
    for ( i=0; i<numIter; i++ ) {
	pm.load(fileName);
    }
    return i;
}

static int pixmap_load_64x64()
{
    return pixmap_load( pm_64x64_file );
}

static int pixmap_load_256x256()
{
    return pixmap_load( pm_256x256_file );
}

static int pixmap_save( QPixmap *pm, const QString &fileName, int numIter=10 )
{
    int i;
    QString format = qperf_imageFormat();
    for ( i=0; i<numIter; i++ ) {
	pm->save(fileName,format);
    }
    return i;
}

static int pixmap_save_64x64()
{
    QString fn = "out-";
    fn += pm_64x64_file;
    return pixmap_save( pm_64x64, fn );
}

static int pixmap_save_256x256()
{ 
    QString fn = "out-";
    fn += pm_256x256_file;
    return pixmap_save( pm_256x256, fn );
}

QPERF_BEGIN(pixmap,"QPixmap tests")
    QPERF(pixmap_create_null,"Create and destroy null pixmaps")
    QPERF(pixmap_create_64x64,"Create and destroy 64x64 pixmaps")
    QPERF(pixmap_create_256x256,"Create and destroy 256x256 pixmaps")
    QPERF(pixmap_fill_64x64,"Fill a 64x64 pixmap with white")
    QPERF(pixmap_fill_256x256,"Fill a 256x256 pixmap with white")
    QPERF(pixmap_bitblt_64x64,"Bitblt a 64x64 pixmap")
    QPERF(pixmap_bitblt_256x256,"Bitblt a 256x256 pixmap")
    QPERF(pixmap_monobitblt_64x64,"Bitblt a 64x64 mono bitmap")
    QPERF(pixmap_monobitblt_256x256,"Bitblt a 256x256 mono bitmap")
    QPERF(pixmap_maskblt_64x64,"Bitblt a masked 64x64 pixmap")
    QPERF(pixmap_maskblt_256x256,"Bitblt a masked 256x256 pixmap")
    QPERF(pixmap_selfmaskblt_64x64,"Bitblt a masked 64x64 bitmap with selfmask")
    QPERF(pixmap_selfmaskblt_256x256,"Bitblt a masked 256x256 bitmap with selfmask")
    QPERF(pixmap_to_image_64x64,"Convert a 64x64 pixmap to an image")
    QPERF(pixmap_to_image_256x256,"Convert a 256x256 pixmap to an image")
    QPERF(pixmap_from_image_64x64,"Convert a 64x64 image to a pixmap")
    QPERF(pixmap_from_image_256x256,"Convert a 256x256 image to a pixmap")
    QPERF(pixmap_load_64x64,"Load a 64x64 pixmap")
    QPERF(pixmap_load_256x256,"Load a 256x256 pixmap")
    QPERF(pixmap_save_64x64,"Save a 64x64 pixmap")
    QPERF(pixmap_save_256x256,"Save a 256x256 pixmap")
QPERF_END(pixmap)


