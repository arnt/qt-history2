#include "qgfxshadowfb_qws.h"

#ifndef QT_NO_QWS_VNC

#ifndef QT_NO_QWS_CURSOR
class QShadowScreenCursor : public QScreenCursor
{
public:
    QShadowScreenCursor();

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
};

QShadowScreenCursor::QShadowScreenCursor() : QScreenCursor()
{
}

void QShadowScreenCursor::set( const QImage &image, int hotx, int hoty )
{
    QWSDisplay::grab( TRUE );
    QRect r( data->x - hotx, data->y - hoty, image.width(), image.height() );
    qt_screen->setDirty( data->bound | r );
    QScreenCursor::set( image, hotx, hoty );
    QWSDisplay::ungrab();
}

void QShadowScreenCursor::move( int x, int y )
{
    QWSDisplay::grab( TRUE );
    QRect r( x - data->hotx, y - data->hoty, data->width, data->height );
    qt_screen->setDirty( r | data->bound );
    QScreenCursor::move( x, y );
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
QGfxShadow<depth,type>::QGfxShadow(unsigned char *b,int w,int h)
    : QGfxRaster<depth, type>( b, w, h )
{
}

template <const int depth, const int type>
QGfxShadow<depth,type>::~QGfxShadow()
{
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPoint( int x, int y )
{
    QWSDisplay::grab( TRUE );
    qt_screen->setDirty( QRect( x+xoffs, y+yoffs, 1, 1 ) & clipbounds );
    QGfxRaster<depth,type>::drawPoint( x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPoints( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qt_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPoints( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawLine( int x1,int y1,int x2,int y2 )
{
    QWSDisplay::grab( TRUE );
    QRect r;
    r.setCoords( x1+xoffs, y1+yoffs, x2+xoffs, y2+yoffs );
    qt_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawLine( x1, y1, x2, y2 );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::fillRect( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    qt_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) & clipbounds );
    QGfxRaster<depth,type>::fillRect( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPolyline( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qt_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPolyline( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::drawPolygon( const QPointArray &pa,bool w,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qt_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPolygon( pa, w, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::blt( int x,int y,int w,int h, int sx, int sy )
{
    QWSDisplay::grab( TRUE );
    qt_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) & clipbounds );
    QGfxRaster<depth,type>::blt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxShadow<depth,type>::scroll( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    int dy = sy - y;
    int dx = sx - x;
    qt_screen->setDirty( QRect(QMIN(x,sx) + xoffs, QMIN(y,sy) + yoffs,
			   w+abs(dx), h+abs(dy)) & clipbounds );
    QGfxRaster<depth,type>::scroll( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxShadow<depth,type>::stretchBlt( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    qt_screen->setDirty( QRect( x + xoffs, y + yoffs, w, h) & clipbounds );
    QGfxRaster<depth,type>::stretchBlt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
void QGfxShadow<depth,type>::tiledBlt( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    qt_screen->setDirty( QRect(x + xoffs, y + yoffs, w, h) & clipbounds );
    QGfxRaster<depth,type>::tiledBlt( x, y, w, h );
    QWSDisplay::ungrab();
}

QShadowFbScreen::QShadowFbScreen( int display_id )
{
}

QShadowFbScreen::~QShadowFbScreen()
{
}

QShadowFbScreen::initDevice()
{
}

bool QShadowFbScreen::connect( const QString &displaySpec )
{
}

void QShadowFbScreen::disconnect()
{
}

int QShadowFbScreen::initCursor(void*, bool)
{
}

void QShadowFbScreen::shutdownDevice()
{
}

QGfx * QShadowFbScreen::createGfx(unsigned char *,int,int,int,int)
{
}

void QShadowFbScreen::save()
{
}

void QShadowFbScreen::restore()
{
}

void QShadowFbScreen::setMode(int nw,int nh,int nd)
{
}

void QShadowFbScreen::setDirty( const QRect& r )
{
}

#endif

