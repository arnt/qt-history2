#include "qpainter.h"
#include "qpixmap.h"
#include "qwidget.h"
#include "qintdict.h"
#include "q1xcompatibility.h"
#include "qpaintdevicemetrics.h"
#include "qbitmap.h"
#include "qpaintdevice.h"
#include "qfontdata_p.h"
#include "qtextcodec.h"
#include <stdio.h>
#include "qt_mac.h"

const int TxNone=0;
const int TxTranslate=1;
const int TxScale=2;
const int TxRotShear=3;

void QPainter::drawPolyInternal( const QPointArray &a, bool close )
{
    pdev->fixport();
    RgnHandle myrgn=NewRgn();
    OpenRgn();
    int loopc;
    MoveTo(a[0].x(),a[0].y());
    for(loopc=1;loopc<a.size();loopc++) {
	LineTo(a[loopc].x(),a[loopc].y());
	MoveTo(a[loopc].x(),a[loopc].y());
    }
    LineTo(a[0].x(),a[0].y());
    CloseRgn(myrgn);
    if(close) {
	updateBrush();
	PaintRgn(myrgn);
    }
    if(cpen.style()!=NoPen) {
	updatePen();
	FrameRgn(myrgn);
    }
    DisposeRgn(myrgn);
}

void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd(PDC_DRAWPIE,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat ); // arc polyline
            int n = pa.size();
            int cx, cy;
            xmat.map(x+w/2, y+h/2, &cx, &cy);
            pa.resize( n+2 );
            pa.setPoint( n, cx, cy );   // add legs
            pa.setPoint( n+1, pa.at(0) );
            drawPolyInternal( pa );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }
    pdev->fixport();
    Rect bounds;
    SetRect(&bounds,x,y,x+w,y+h);
    //PaintArc(&bounds,a*16,alen*16);
    int aa,bb;
    if(!a) {
	aa=0;
    } else {
	aa=a/16;
    }
    if(!alen) {
	bb=0;
    } else {
	bb=alen/16;
    }
    if(aa<0)
	aa=0;
    if(bb<1)
	bb=1;
    if(this->brush().style()==SolidPattern) {
	updateBrush();
	PaintArc(&bounds,aa,bb);
    }
    if(cpen.style()!=NoPen) {
	updatePen();
	FrameArc(&bounds,aa,bb);
    }
}

void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd(PDC_DRAWARC,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat );    // arc polyline
            drawPolyInternal( pa, FALSE );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    pdev->fixport();
    Rect bounds;
    SetRect(&bounds,x,y,x+w,y+h);
    updatePen();
    FrameArc(&bounds,a*16,-(alen*16));
}

void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{
    if ( !isActive() )
        return;
    if ( xRnd <= 0 || yRnd <= 0 ) {
        drawRect( x, y, w, h );                 // draw normal rectangle
        return;
    }
    if ( xRnd >= 100 )                          // fix ranges
        xRnd = 99;
    if ( yRnd >= 100 )
        yRnd = 99;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = xRnd;
            param[2].ival = yRnd;
            if ( !pdev->cmd(PDC_DRAWROUNDRECT,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a;
            if ( w <= 0 || h <= 0 )
                fix_neg_rect( &x, &y, &w, &h );
            w--;
            h--;
            int rxx = w*xRnd/200;
            int ryy = h*yRnd/200;
            int rxx2 = 2*rxx;
            int ryy2 = 2*ryy;
            int xx, yy;

            // ###### WWA: this should use the new makeArc (with xmat)

            a.makeEllipse( x, y, rxx2, ryy2 );
            int s = a.size()/4;
            int i = 0;
            while ( i < s ) {
                a.point( i, &xx, &yy );
                xx += w - rxx2;
                a.setPoint( i++, xx, yy );
            }
            i = 2*s;
            while ( i < 3*s ) {
                a.point( i, &xx, &yy );
                yy += h - ryy2;
                a.setPoint( i++, xx, yy );
            }
            while ( i < 4*s ) {
                a.point( i, &xx, &yy );
                xx += w - rxx2;
                yy += h - ryy2;
                a.setPoint( i++, xx, yy );
            }
            drawPolyInternal( xForm(a) );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }

    pdev->fixport();
    Rect rect;
    SetRect(&rect,x,y,x+w,y+h);
    if(this->brush().style()==SolidPattern) {
	updateBrush();
	PaintRoundRect(&rect,w*xRnd/100,h*yRnd/100);
    }
    if(cpen.style()!=NoPen) {
	updatePen();
	FrameRoundRect(&rect,w*xRnd/100,h*yRnd/100);
    }
}

void QPainter::setBackgroundMode( BGMode m )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
        warning( "QPainter::setBackgroundMode: Call begin() first" );
#endif
        return;
    }
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(CHECK_RANGE)
        warning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
        return;
    }
    bg_mode = m;
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].ival = m;
        if ( !pdev->cmd(PDC_SETBKMODE,this,param) /* || !hdc */ )
            return;
    }
}

void QPainter::drawPolygon( const QPointArray &a, bool winding,
                            int index, int npoints )
{
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            if ( npoints != (int)a.size() ) {
                pa = QPointArray( npoints );
                for ( int i=0; i<npoints; i++ )
                    pa.setPoint( i, a.point(index+i) );
            }
            QPDevCmdParam param[2];
            param[0].ptarr = (QPointArray*)&pa;
            param[1].ival = winding;
            if ( !pdev->cmd(PDC_DRAWPOLYGON,this,param) /*|!hdc*/ )
                return;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index   = 0;
                npoints = pa.size();
            }
        }
    }
    // Hmm
    drawPolyInternal(pa,true);
}

void QPainter::drawWinFocusRect( int x, int y, int w, int h )
{
    drawWinFocusRect( x, y, w, h, TRUE, color0 );
}

void QPainter::drawWinFocusRect(int x,int y,int w,int h,
                                const QColor &bgColor)
{
    drawWinFocusRect(x,y,w,h,FALSE,bgColor);
}

typedef QIntDict<QPaintDevice> QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;

bool QPainter::begin( const QPaintDevice *pd )
{
    if ( isActive() ) {                         // already active painting
#if defined(CHECK_STATE)
        warning( "QPainter::begin: Painter is already active."
                 "\n\tYou must end() the painter before a second begin()" );
#endif
        return FALSE;
    }
    if ( pd == 0 ) {
#if defined(CHECK_NULL)
        warning( "QPainter::begin: Paint device cannot be null" );
#endif
        return FALSE;
    }

    QWidget *copyFrom = 0;
    if ( pdev_dict ) {                          // redirected paint device?
        pdev = pdev_dict->find( (long)pd );
        if ( pdev ) {
            if ( pd->devType() == QInternal::Widget )
                copyFrom = (QWidget *)pd;       // copy widget settings
        } else {
            pdev = (QPaintDevice *)pd;
        }
    } else {
        pdev = (QPaintDevice *)pd;
    }

    if ( pdev->isExtDev() && pdev->paintingActive() ) {
	// somebody else is already painting
#if defined(CHECK_STATE)
        warning( "QPainter::begin: Another QPainter is already painting "
                 "this device;\n\tAn extended paint device can only be painted "
                 "by one QPainter at a time." );
#endif
        return FALSE;
    }

    bool reinit = flags != IsStartingUp;        // 2nd or 3rd etc. time called
    flags = IsActive | DirtyFont;               // init flags
    int dt = pdev->devType();                   // get the device type

    if ( (pdev->devFlags & QInternal::ExternalDevice) != 0 )
	// this is an extended device
        setf(ExtDev);
    else if ( dt == QInternal::Pixmap )         // device is a pixmap
        ((QPixmap*)pdev)->detach();             // will modify it

    if ( testf(ExtDev) ) {                      // external device
        if ( !pdev->cmd(QPaintDevice::PdcBegin,this,0) ) {   // could not begin painting
            pdev = 0;
            return FALSE;
        }
        if ( tabstops )                         // update tabstops for device
            setTabStops( tabstops );
        if ( tabarray )                         // update tabarray for device
            setTabArray( tabarray );
    }

    pdev->painters++;                           // also tell paint device
    hd=pdev->hd;
    bro = QPoint( 0, 0 );
    if ( reinit ) {
        bg_mode = TransparentMode;              // default background mode
        rop = CopyROP;                          // default ROP
        wxmat.reset();                          // reset world xform matrix
        txop = txinv = 0;
        if ( dt != QInternal::Widget ) {
            QFont  defaultFont;                 // default drawing tools
            QPen   defaultPen;
            QBrush defaultBrush;
            cfont  = defaultFont;               // set these drawing tools
            cpen   = defaultPen;
            cbrush = defaultBrush;
            bg_col = white;                     // default background color
	    // was white
        }
    }
    wx = wy = vx = vy = 0;                      // default view origins

    if ( dt == QInternal::Widget ) {                    // device is a widget
        QWidget *w = (QWidget*)pdev;
        cfont = w->font();                      // use widget font
        cpen = QPen( w->foregroundColor() );    // use widget fg color
        if ( reinit ) {
            QBrush defaultBrush;
            cbrush = defaultBrush;
        }
        bg_col = w->backgroundColor();          // use widget bg color
        ww = vw = w->width();                   // default view size
        wh = vh = w->height();
        if ( w->testWFlags(WPaintUnclipped) ) { // paint direct on device
            setf( NoCache );
            updatePen();
            updateBrush();
            //XSetSubwindowMode( dpy, gc, IncludeInferiors );
            //XSetSubwindowMode( dpy, gc_brush, IncludeInferiors );
        }
    } else if ( dt == QInternal::Pixmap ) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)pdev;
        if ( pm->isNull() ) {
#if defined(CHECK_NULL)
            warning( "QPainter::begin: Cannot paint null pixmap" );
#endif
            end();
            return FALSE;
        }
        ww = vw = pm->width();                  // default view size
        wh = vh = pm->height();
    } else if ( testf(ExtDev) ) {               // external device
        ww = vw = pdev->metric( PDM_WIDTH );
        wh = vh = pdev->metric( PDM_HEIGHT );
    }
    if ( ww == 0 )
        ww = wh = vw = vh = 1024;
    if ( copyFrom ) {                           // copy redirected widget
        cfont = copyFrom->font();
        cpen = QPen( copyFrom->foregroundColor() );
        bg_col = copyFrom->backgroundColor();
    }
    if ( testf(ExtDev) ) {                      // external device
        setBackgroundColor( bg_col );           // default background color
        setBackgroundMode( TransparentMode );   // default background mode
        setRasterOp( CopyROP );                 // default raster operation
    }
    updateBrush();
    updatePen();
    return true;
}

bool QPainter::end()
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
        warning( "QPainter::end: Missing begin() or begin() failed" );
#endif
        return FALSE;
    }
    if ( testf(FontMet) )                       // remove references to this
        QFontMetrics::reset( this );
    if ( testf(FontInf) )                       // remove references to this
        QFontInfo::reset( this );

    if ( testf(ExtDev) )
        pdev->cmd( PDC_END, this, 0 );

    flags = 0;
    pdev->painters--;
    pdev = 0;
    return TRUE;
}

extern const unsigned char * p_str(const char * c);

void QPainter::drawText( int x, int y, const QString &str, int len )
{
    if ( !isActive() )
        return;
    if ( len < 0 )
        len = str.length();
    if ( len == 0 )                             // empty string
        return;

    pdev->fixport();
    updateBrush();
    updatePen();

    if ( testf(DirtyFont|ExtDev|VxF|WxF) ) {
        if ( testf(DirtyFont) )
            updateFont();

        if ( testf(ExtDev) ) {
            QPDevCmdParam param[2];
            QPoint p( x, y );
            QString newstr = str.left(len);
            param[0].point = &p;
            param[1].str = &newstr;
            if ( !pdev->cmd(PDC_DRAWTEXT2,this,param) || !hd )
                return;
        }

        if ( txop >= TxScale ) {
            const QFontMetrics & fm = fontMetrics();
            QFontInfo    fi = fontInfo();
            QRect bbox = fm.boundingRect( str, len );
            int w=bbox.width(), h=bbox.height();
            int aw, ah;
            int tx=-bbox.x(),  ty=-bbox.y();    // text position
            QWMatrix mat1( m11(), m12(), m21(), m22(), dx(),  dy() );
            QFont dfont( cfont );
            QWMatrix mat2;
            if ( txop == TxScale ) {
                int newSize = qRound( m22() * (double)cfont.pointSize() ) - 1;
                newSize = QMAX( 6, QMIN( newSize, 72 ) ); // empirical values
                dfont.setPointSize( newSize );
                QFontMetrics fm2( dfont );
                QRect abbox = fm2.boundingRect( str, len );
                aw = abbox.width();
                ah = abbox.height();
                tx = -abbox.x();
                ty = -abbox.y();        // text position - off-by-one?
                if ( aw == 0 || ah == 0 )
                    return;
                double rx = (double)bbox.width() * mat1.m11() / (double)aw;
                double ry = (double)bbox.height() * mat1.m22() /(double)ah;
                mat2 = QWMatrix( rx, 0, 0, ry, 0, 0 );
            }
            else {
                mat2 = QPixmap::trueMatrix( mat1, w, h );
                aw = w;
                ah = h;
            }
            bool empty = aw == 0 || ah == 0;
            QBitmap * wx_bm=0;
            bool create_new_bm = wx_bm == 0;
            if ( create_new_bm && !empty ) {    // no such cached bitmap
                QBitmap bm( aw, ah );           // create bitmap
                bm.fill( color0 );
                QPainter paint;
                paint.begin( &bm );             // draw text in bitmap
                paint.setFont( dfont );
                paint.drawText( tx, ty, str, len );
                paint.end();
                wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
                if ( wx_bm->isNull() ) {
                    delete wx_bm;               // nothing to draw
                    return;
                }
            }
            if ( bg_mode == OpaqueMode ) {      // opaque fill
                int fx = x;
                int fy = y - fm.ascent();
                int fw = fm.width(str,len);
                int fh = fm.ascent() + fm.descent();
                int m, n;
                QPointArray a(5);
                mat1.map( fx,    fy,    &m, &n );  a.setPoint( 0, m, n );
		a.setPoint( 4, m, n );
                mat1.map( fx+fw, fy,    &m, &n );  a.setPoint( 1, m, n );
                mat1.map( fx+fw, fy+fh, &m, &n );  a.setPoint( 2, m, n );
                mat1.map( fx,    fy+fh, &m, &n );  a.setPoint( 3, m, n );
                QBrush oldBrush = cbrush;
                setBrush( backgroundColor() );
                updateBrush();
                //XFillPolygon( dpy, hd, gc_brush, (XPoint*)a.data(), 4,
                //              Nonconvex, CoordModeOrigin );
                //XDrawLines( dpy, hd, gc_brush, (XPoint*)a.data(), 5,
                //            CoordModeOrigin );
                setBrush( oldBrush );
            }
            if ( empty )
                return;
            double fx=x, fy=y, nfx, nfy;
            mat1.map( fx,fy, &nfx,&nfy );
            double tfx=tx, tfy=ty, dx, dy;
            mat2.map( tfx, tfy, &dx, &dy );     // compute position of bitmap
            x = qRound(nfx-dx);
            y = qRound(nfy-dy);
            return;
        }
        if ( txop == TxTranslate )
            map( x, y, &x, &y );
    }

    QCString mapped='?';

    const QTextCodec* mapper = cfont.d->mapper();
    if ( mapper ) {
        // translate from Unicode to font charset encoding here
        mapped = mapper->fromUnicode(str,len);
    }

    if(!mapped) {
	char * soo=new char[2];
	soo[0]='?';
	soo[1]='\0';
	mapped=soo;
    }

    if ( !cfont.handle() ) {
        if ( mapped.isNull() )
            warning("Fontsets only apply to mapped encodings");
        else {
	    MoveTo(x,y);
	    DrawString(p_str(mapped));
        }
    } else {
        if ( !mapped.isNull() ) {
	    MoveTo(x,y);
	    DrawString(p_str(mapped));
        } else {
            // Unicode font

            QString v = str;
#ifdef QT_BIDI
            v.compose();  // apply ligatures (for arabic, etc...)
            v = v.visual(); // visual ordering
            len = v.length();
#endif

	    MoveTo(x,y);
	    DrawString(p_str((char *)v.unicode()));
        }
    }

    if ( cfont.underline() || cfont.strikeOut() ) {
        const QFontMetrics & fm = fontMetrics();
        int lw = fm.lineWidth();
        int tw = fm.width( str, len );
    }

}

void QPainter::lineTo(int x,int y)
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd(PDC_LINETO,this,param) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    pdev->fixport();
    updateBrush();
    updatePen();
    MoveTo(penx,peny);
    LineTo(x,y);
    penx=x;
    peny=y;
}

void QPainter::moveTo(int x,int y)
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd(PDC_MOVETO,this,param) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    penx=x;
    peny=y;
}

void QPainter::setBrushOrigin( int x, int y )
{
}

void QPainter::drawPolyline( const QPointArray &a, int index, int npoints )
{
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            if ( npoints != (int)pa.size() ) {
                pa = QPointArray( npoints );
                for ( int i=0; i<npoints; i++ )
                    pa.setPoint( i, a.point(index+i) );
                index = 0;
            }
            QPDevCmdParam param[1];
            param[0].ptarr = (QPointArray*)&pa;
            if ( !pdev->cmd(PDC_DRAWPOLYLINE,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
		index   = 0;
                npoints = pa.size();
            }
        }
    }
    int x1, y1, x2, y2, xsave, ysave;
    pa.point( index+npoints-2, &x1, &y1 );      // last line segment
    pa.point( index+npoints-1, &x2, &y2 );
    xsave = x2; ysave = y2;
    bool plot_pixel = FALSE;
    if ( x1 == x2 ) {                           // vertical
        if ( y1 < y2 )
            y2++;
        else
            y2--;
    } else if ( y1 == y2 ) {                    // horizontal
        if ( x1 < x2 )
            x2++;
        else
            x2--;
    } else {
        plot_pixel = cpen.style() == SolidLine; // plot last pixel
    }
    int loopc;
    pdev->fixport();
    updateBrush();
    updatePen();
    MoveTo(pa[0].x(),pa[0].y());
    for(loopc=1;loopc<pa.size();loopc++) {
	LineTo(pa[loopc].x(),pa[loopc].y());
    }
}

void QPainter::drawPoint( int x, int y )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd(PDC_DRAWPOINT,this,param) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    pdev->fixport();
    // This is a bit silly
    updatePen();
    MoveTo(x,y);
    LineTo(x,y);
}

void QPainter::drawQuadBezier( const QPointArray &a, int index )
{
    if ( !isActive() )
        return;
    if ( (int)a.size() - index < 4 ) {
#if defined(CHECK_RANGE)
        warning( "QPainter::drawQuadBezier: Cubic Bezier needs 4 control "
                 "points" );
#endif
        return;
    }
    QPointArray pa( a );
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( index != 0 || a.size() > 4 ) {
            pa = QPointArray( 4 );
            for ( int i=0; i<4; i++ )
                pa.setPoint( i, a.point(index+i) );
            index = 0;
        }
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            param[0].ptarr = (QPointArray*)&pa;
            if ( !pdev->cmd(PDC_DRAWQUADBEZIER,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop != TxNone )
            pa = xForm( pa );
    }
}

void QPainter::drawChord( int x, int y, int w, int h, int a, int alen )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd(PDC_DRAWCHORD,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w-1, h-1, a, alen, xmat ); // arc polygon
            int n = pa.size();
            pa.resize( n+1 );
            pa.setPoint( n, pa.at(0) );         // connect endpoints
            drawPolyInternal( pa );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }
}

void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{
    if ( nlines < 0 )
        nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
        nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            if ( nlines != (int)pa.size()/2 ) {
                pa = QPointArray( nlines*2 );
                for ( int i=0; i<nlines*2; i++ )
                    pa.setPoint( i, a.point(index+i) );
                index = 0;
            }
            QPDevCmdParam param[1];
            param[0].ptarr = (QPointArray*)&pa;
            if ( !pdev->cmd(QPaintDevice::PdcDrawLineSegments,this,param) /*|| !hdc*/)
                return;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, nlines*2 );
            if ( pa.size() != a.size() ) {
                index  = 0;
                nlines = pa.size()/2;
            }
        }
    }

    int  x1, y1, x2, y2;
    uint i = index;
    bool solid = cpen.style() == SolidLine;

    pdev->fixport();
    updatePen();
    while ( nlines-- ) {
        pa.point( i++, &x1, &y1 );
        pa.point( i++, &x2, &y2 );
        MoveTo(x1,y1);
        LineTo(x2,y2);
    }
}

void QPainter::drawEllipse( int x, int y, int w, int h )
{
    if ( !isActive() ) {
        return;
    }
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            if ( !pdev->cmd(PDC_DRAWELLIPSE,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a;
            a.makeArc( x, y, w, h, 0, 360*16, xmat );
            drawPolyInternal( a );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }
    pdev->fixport();
    Rect r;
    SetRect(&r,x,y,x+w,y+h);
    if(this->brush().style()==SolidPattern) {
	updateBrush();
	PaintOval(&r);
    }
    updatePen();
    FrameOval(&r);
}

void QPainter::setRasterOp( RasterOp r )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
        warning( "QPainter::setRasterOp: Call begin() first" );
#endif
        return;
    }
    if ( (uint)r > LastROP ) {
#if defined(CHECK_RANGE)
        warning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
        return;
    }
    rop = r;
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].ival = r;
        if ( !pdev->cmd(PDC_SETROP,this,param) /*|| !hdc*/ )
            return;
    }
}

void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[2];
            QPoint p1(x1, y1), p2(x2, y2);
            param[0].point = &p1;
            param[1].point = &p2;
            if ( !pdev->cmd(PDC_DRAWLINE,this,param) /*|| !hdc*/ )
                return;
        }
        map( x1, y1, &x1, &y1 );
        map( x2, y2, &x2, &y2 );
    }
    pdev->fixport();
    updatePen();
    MoveTo(x1,y1);
    LineTo(x2,y2);
}


void QPainter::drawPoints( const QPointArray& a, int index, int npoints )
{
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 1 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            for (int i=0; i<npoints; i++) {
                QPoint p( pa[index+i].x(), pa[index+i].y() );
                param[0].point = &p;
                if ( !pdev->cmd(PDC_DRAWPOINT,this,param))
                    return;
            }
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index = 0;
                npoints = pa.size();
            }
        }
    }
    if ( cpen.style() != NoPen ) {
        for (int i=0; i<npoints; i++) {
        }
    }
}

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
                           int sx, int sy, int sw, int sh )
{
    if ( !isActive() || pixmap.isNull() ) {
        return;
    }
    if ( sw < 0 )
        sw = pixmap.width() - sx;
    if ( sh < 0 )
        sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
        x -= sx;
        sw += sx;
        sx = 0;
    }
    if ( sw + sx > pixmap.width() )
        sw = pixmap.width() - sx;
    if ( sy < 0 ) {
        y -= sy;
        sh += sy;
        sy = 0;
    }
    if ( sh + sy > pixmap.height() )
        sh = pixmap.height() - sy;
    if ( sw <= 0 || sh <= 0 ) {
        return;
    }

    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) || (txop == TxScale && pixmap.mask()) ||
             txop == TxRotShear ) {
            if ( sx != 0 || sy != 0 ||
                 sw != pixmap.width() || sh != pixmap.height() ) {
                QPixmap tmp( sw, sh, pixmap.depth() );
                bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, TRUE );
                if ( pixmap.mask() ) {
                    QBitmap mask( sw, sh );
                    bitBlt( &mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
                            CopyROP, TRUE );
                    tmp.setMask( mask );
                }
                drawPixmap( x, y, tmp );
                return;
            }
            if ( testf(ExtDev) ) {
                QPDevCmdParam param[2];
                QPoint p(x,y);
                param[0].point  = &p;
                param[1].pixmap = &pixmap;
                if ( !pdev->cmd(PDC_DRAWPIXMAP,this,param) /* || !hdc */ ) {
                    return;
		}
            }
        }
        if ( txop == TxTranslate )
            map( x, y, &x, &y );
    }

    bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop );
}

void QPainter::init()
{
    flags = IsStartingUp;
    bg_col = white;                             // default background color
    bg_mode = TransparentMode;                  // default background mode
    rop = CopyROP;                              // default ROP
    tabstops = 0;                               // default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    wm_stack = 0;
    pdev = 0;
    txop = txinv = 0;
    penRef = brushRef = 0;
    hd=0;
}

void QPainter::updateFont()
{
    clearf(DirtyFont);
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].font = &cfont;
        if ( !pdev->cmd(PDC_SETFONT,this,param) || !hd )
            return;
    }
    setf(NoCache);
    if ( penRef )
        updatePen();                            // force a non-cached GC
    HANDLE h = cfont.handle();
    cfont.macSetFont((void *)pdev->hd);
}

void QPainter::setFont(const QFont &font)
{
#if defined(CHECK_STATE)
    if ( !isActive() )
        warning( "QPainter::setFont: Will be reset by begin()" );
#endif
    if ( cfont.d != font.d ) {
        cfont = font;
        setf(DirtyFont);
    }
}

void QPainter::drawRect( int x, int y, int w, int h )
{
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            if ( !pdev->cmd(PDC_DRAWRECT,this,param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a( QRect(x,y,w,h) );
            drawPolyInternal( xForm(a) );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }

    ::RGBColor c;

    pdev->fixport();

    Rect rect;
    SetRect(&rect,x,y,x+w,y+h);
    if(this->brush().style()==SolidPattern) {
	updateBrush();
	PaintRect(&rect);
    }
    if(cpen.style()!=NoPen) {
	updatePen();
	FrameRect(&rect);
    }
}

void QPainter::setClipping( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
        warning( "QPainter::setClipping: Will be reset by begin()" );
#endif
    if ( !isActive() || enable == testf(ClipOn) )
        return;
    setf(ClipOn,enable);
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].ival = enable;
        if ( !pdev->cmd(PDC_SETCLIP,this,param) /*| !hd */)
            return;
    }
    if ( enable ) {
        if ( penRef )
            updatePen();
        if ( brushRef )
            updateBrush();
    } else {
    }
}

void QPainter::setClipRect( const QRect &r )
{
    QRegion rgn( r );
    setClipRegion( rgn );
}

void QPainter::setClipRegion( const QRegion &rgn )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
        warning( "QPainter::setClipRegion: Will be reset by begin()" );
#endif
    crgn = rgn;
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].rgn = &crgn;
        if (( !pdev->cmd(PDC_SETCLIPRGN,this,param) /*|| !hd */) &&
	    (pdev->devType() != QInternal::Printer ))
            return;
    }
    clearf( ClipOn );                           // be sure to update clip rgn
    setClipping( TRUE );
}

void QPainter::setBackgroundColor( const QColor &c )
{
}

static void drawTile( QPainter *p, int x, int y, int w, int h,
                      const QPixmap &pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if ( yPos + drawH > y + h )        // Cropping last row
            drawH = y + h - yPos;
        xPos = x;
        xOff = xOffset;
        while( xPos < x + w ) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if ( xPos + drawW > x + w )    // Cropping last column
                drawW = x + w - xPos;
            p->drawPixmap( xPos, yPos, pixmap, xOff, yOff, drawW, drawH );
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

void QPainter::drawTiledPixmap( int x, int y, int w, int h,
                                const QPixmap &pixmap, int sx, int sy )
{
    int sw = pixmap.width();
    int sh = pixmap.height();
    if (!sw || !sh )
	return;
    if ( sx < 0 )
        sx = sw - -sx % sw;
    else
        sx = sx % sw;
    if ( sy < 0 )
        sy = sh - -sy % sh;
    else
        sy = sy % sh;
    /*
      Requirements for optimizing tiled pixmaps:
      - not an external device
      - not scale or rotshear
      - no mask
    */
    QBitmap *mask = (QBitmap *)pixmap.mask();
    if ( !testf(ExtDev) && txop <= TxTranslate && mask == 0 ) {
        if ( txop == TxTranslate )
            map( x, y, &x, &y );
        return;
    }
    if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
        int tw = sw, th = sh;
        while ( tw*th < 32678 && tw < w/2 )
            tw *= 2;
        while ( tw*th < 32678 && th < h/2 )
            th *= 2;
        QPixmap tile( tw, th, pixmap.depth() );
        if ( mask ) {
            QBitmap tilemask( tw, th );
            tile.setMask( tilemask );
        }
        drawTile( this, x, y, w, h, tile, sx, sy );
    } else {
        drawTile( this, x, y, w, h, pixmap, sx, sy );
    }
}

void QPainter::updateBrush()
{
    if(!pdev->handle())
	return;
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].brush = &cbrush;
        if ( !pdev->cmd(PDC_SETBRUSH,this,param) || !hd )
            return;
    }

    int  bs      = cbrush.style();

    uchar *pat = 0;                             // pattern
    int d = 0;                                  // defalt pattern size: d*d

    if(!pdev)
	return;

    if ( bs == CustomPattern || pat ) {
        QPixmap *pm;
        pm = cbrush.data->pixmap;
    }
    ::RGBColor f;
    f.red=cbrush.color().red()*256;
    f.green=cbrush.color().green()*256;
    f.blue=cbrush.color().blue()*256;
    RGBForeColor(&f);
    if(pdev->devType()==QInternal::Widget) {
	bg_col=((QWidget *)pdev)->backgroundColor();
    }
}

void QPainter::updatePen()
{
    if(!pdev->handle())
	return;
    WindowPtr p=(WindowPtr)pdev->handle();
    SetPort(p);
    ::RGBColor f;
    f.red=cpen.color().red()*256;
    f.green=cpen.color().green()*256;
    f.blue=cpen.color().blue()*256;
    RGBForeColor(&f);
    int s=cpen.width();
    if(s<1)
	s=1;
    PenSize(s,s);
}

void QPainter::flush()
{
    // Do nothing
}


















