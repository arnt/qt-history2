/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_mac.cpp#316 $
**
** Implementation of QPainter class for Mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qlist.h"
#include "qintdict.h"
#include "qfontdata_p.h"
#include "qtextcodec.h"
#include <ctype.h>
#include <stdlib.h>
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h"
#include "qt_mac.h"
#include <qstack.h>
#include <qtextcodec.h>
#include <qprinter.h>

const int TxNone=0;
const int TxTranslate=1;
const int TxScale=2;
const int TxRotShear=3;

/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

static void drawTile( QPainter *, int, int, int, int, const QPixmap &, int, int );
QPoint posInWindow(QWidget *w);
typedef QIntDict<QPaintDevice> QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;
QRegion make_region(RgnHandle handle);
void unclippedBitBlt( QPaintDevice *dst, int dx, int dy, 
		      const QPaintDevice *src, int sx, int sy, int sw, int sh, 
		      Qt::RasterOp rop, bool imask);

//
// Generate a string that describes a transformed bitmap. This string is used
// to insert and find bitmaps in the global pixmap cache.
//

static QString gen_text_bitmap_key( const QWMatrix &m, const QFont &font,
				    const QString &str, int len )
{
    QString fk = font.key();
    int sz = 4*2 + len*2 + fk.length()*2 + sizeof(double)*6;
    QByteArray buf(sz);
    uchar *p = (uchar *)buf.data();
    *((double*)p)=m.m11();  p+=sizeof(double);
    *((double*)p)=m.m12();  p+=sizeof(double);
    *((double*)p)=m.m21();  p+=sizeof(double);
    *((double*)p)=m.m22();  p+=sizeof(double);
    *((double*)p)=m.dx();   p+=sizeof(double);
    *((double*)p)=m.dy();   p+=sizeof(double);
    QChar h1( '$' );
    QChar h2( 'q' );
    QChar h3( 't' );
    QChar h4( '$' );
    *((QChar*)p)=h1;  p+=2;
    *((QChar*)p)=h2;  p+=2;
    *((QChar*)p)=h3;  p+=2;
    *((QChar*)p)=h4;  p+=2;
    memcpy( (char*)p, (char*)str.unicode(), len*2 );  p += len*2;
    memcpy( (char*)p, (char*)fk.unicode(), fk.length()*2 ); p += fk.length()*2;
    return QString( (QChar*)buf.data(), buf.size()/2 );
}

static QBitmap *get_text_bitmap( const QString &key )
{
    return (QBitmap*)QPixmapCache::find( key );
}

static void ins_text_bitmap( const QString &key, QBitmap *bm )
{
    if ( !QPixmapCache::insert(key,bm) )	// cannot insert pixmap
	delete bm;
}

/* paintevent magic to provide Windows semantics on MAC
 */
class paintevent_item
{
    QPaintDevice* dev;
    QRegion clipRegion;
public:
    paintevent_item(QPaintDevice *d, QRegion r) : dev(d), clipRegion(r) { }
    inline int operator==( QPaintDevice *rhs ) const { return rhs == dev; }
    inline QPaintDevice *device() const { return dev; }
    inline QRegion region() const { return clipRegion; }
};
QStack<paintevent_item> paintevents;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    QRegion r = region;
    if(dev && dev->devType() == QInternal::Widget) {
	QWidget *w = (QWidget *)dev;
	QPoint mp(posInWindow(w));
	r.translate(mp.x(), mp.y());

	QRegion wclip = w->clippedRegion();
	if(!wclip.isNull())
	    r &= wclip;
    }
    paintevents.push(new paintevent_item(dev, r));
}

void qt_clear_paintevent_clipping( QPaintDevice *dev )
{
    if(paintevents.isEmpty() || !((*paintevents.current()) == dev)) {
	qDebug("Whoa, now that is messed up!");
	return;
    }
    delete paintevents.pop();
}


void QPainter::initialize()
{
}

void QPainter::cleanup()
{
}

//I assume this will work, but I need to find a test case FIXME!
void QPainter::redirect( QPaintDevice *pdev, QPaintDevice *replacement )
{
    qDebug("Need to test this! %s:%d", __FILE__, __LINE__);
    if ( pdev_dict == 0 ) {
	if ( replacement == 0 )
	    return;
	pdev_dict = new QPaintDeviceDict;
	Q_CHECK_PTR( pdev_dict );
    }
#if defined(QT_CHECK_NULL)
    if ( pdev == 0 )
	qWarning( "QPainter::redirect: The pdev argument cannot be 0" );
#endif
    if ( replacement ) {
	pdev_dict->insert( (long)pdev, replacement );
    } else {
	pdev_dict->remove( (long)pdev );
	if ( pdev_dict->count() == 0 ) {
	    delete pdev_dict;
	    pdev_dict = 0;
	}
    }
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
    hd = 0;
    saved = 0;
    brush_style_pix = 0;
    unclipped = FALSE;
}


void QPainter::setFont( const QFont &font )
{
#if defined(QT_CHECK_STATE)
    if ( !isActive() )
        warning( "QPainter::setFont: Will be reset by begin()" );
#endif
    if ( cfont.d != font.d ) {
        cfont = font;
        setf(DirtyFont);
    }
}


void QPainter::updateFont()
{
    clearf(DirtyFont);
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].font = &cfont;
        if ( !pdev->cmd(QPaintDevice::PdcSetFont,this,param) || !hd )
            return;
    }
    setf(NoCache);
    if ( penRef )
        updatePen();                            // force a non-cached GC
    cfont.macSetFont(pdev);
}

static int ropCodes[] = {			// ROP translation table
    patCopy, patOr, patXor, patBic, notPatCopy,
    notPatOr, notPatXor, notPatBic, 
    666, 666, 666, 666, 666, 666, 666, 666, 666
};

void QPainter::updatePen()
{
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	if ( !pdev->cmd( QPaintDevice::PdcSetPen, this, param ) || !hd )
	    return;
    }

    //pen color
    ::RGBColor f;
    f.red = cpen.color().red()*256;
    f.green = cpen.color().green()*256;
    f.blue = cpen.color().blue()*256;
    RGBForeColor( &f );

    //pen size
    int dot = cpen.width();
    if ( dot < 1 )
	dot = 1;
    PenSize( dot, dot );

    int	ps = cpen.style();
    Pattern pat;
    switch ( ps ) {
	case DotLine:
	case DashDotLine:
	case DashDotDotLine:
	    qDebug( "Penstyle not implemented %s - %d", __FILE__, __LINE__ );
	case DashLine:
	    GetQDGlobalsGray( &pat );
	    break;
	default:
            GetQDGlobalsBlack( &pat );
            break;
    }    
    PenPat( &pat );

    //penmodes
    PenMode(ropCodes[rop]);
}


void QPainter::updateBrush()
{
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[1];
        param[0].brush = &cbrush;
        if ( !pdev->cmd( QPaintDevice::PdcSetBrush,this,param) || !hd )
            return;
    }

    //color
    ::RGBColor f;
    f.red = cbrush.color().red()*256;
    f.green = cbrush.color().green()*256;
    f.blue = cbrush.color().blue()*256;
    RGBForeColor( &f );
    if ( pdev->devType() == QInternal::Widget ) 
	bg_col = ((QWidget *)pdev)->backgroundColor();

    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static uchar hor_pat[] = {			// horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {			// vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {			// cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {			// backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {			// forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {			// diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    brush_style_pix = 0;
    int bs = cbrush.style();
    if( bs >= Dense1Pattern && bs <= DiagCrossPattern ) {
	QString key;
	key.sprintf( "$qt-brush$%d", bs );
	brush_style_pix = QPixmapCache::find( key );
	if ( !brush_style_pix ) {                        // not already in pm dict
	    uchar *pat=pat_tbl[ bs-Dense1Pattern ];
	    int d = 16;
	    if(bs<=Dense7Pattern)
		d=8;
	    else if (bs<=CrossPattern)
		d=24;
	    brush_style_pix = new QPixmap( d, d );
	    Q_CHECK_PTR( brush_style_pix );
	    brush_style_pix->setMask(QBitmap( d, d, pat, FALSE ));
	    QPixmapCache::insert( key, brush_style_pix );
	}
	brush_style_pix->fill(cbrush.color());
    } 

    //penmodes
    PenMode(ropCodes[rop]);
}

typedef QIntDict<QPaintDevice> QPaintDeviceDict;

bool QPainter::begin( const QPaintDevice *pd )
{
    if ( isActive() ) {                         // already active painting
#if defined(QT_CHECK_STATE)
        warning( "QPainter::begin: Painter is already active."
                 "\n\tYou must end() the painter before a second begin()" );
#endif
        return FALSE;
    }
    if ( pd == 0 ) {
#if defined(QT_CHECK_NULL)
        warning( "QPainter::begin: Paint device cannot be null" );
#endif
        return FALSE;
    }

    //save the gworld now, we'll reset it in end()
    saved = new QMacSavedPortInfo;

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
#if defined(QT_CHECK_STATE)
        warning( "QPainter::begin: Another QPainter is already painting "
                 "this device;\n\tAn extended paint device can only be painted "
                 "by one QPainter at a time." );
#endif
        return FALSE;
    }

    int dt = pdev->devType();                   // get the device type
    bool reinit = flags != IsStartingUp;        // 2nd or 3rd etc. time called
    flags = IsActive | DirtyFont;               // init flags

    if ( (pdev->devFlags & QInternal::ExternalDevice) != 0 )
	// this is an extended device
        setf(ExtDev);
    else if ( dt == QInternal::Pixmap )         // device is a pixmap
        ((QPixmap*)pdev)->detach();             // will modify it

    if ( testf( ExtDev ) ) {                      // external device
        if ( !pdev->cmd( QPaintDevice::PdcBegin, this, 0) ) {   // could not begin painting
	    clearf( IsActive );
            pdev = 0;
            return FALSE;
        }
        if ( tabstops )                         // update tabstops for device
            setTabStops( tabstops );
        if ( tabarray )                         // update tabarray for device
            setTabArray( tabarray );
    }


    pdev->painters++;                           // also tell paint device
    hd = pdev->hd;
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
    offx = offy = wx = wy = vx = vy = 0;                      // default view origins

    unclipped = FALSE;
    if ( pdev->devType() == QInternal::Widget ) {                    // device is a widget
        QWidget *w = (QWidget*)pdev;

	initPaintDevice();

	
        cfont = w->font();                      // use widget font
        cpen = QPen( w->foregroundColor() );    // use widget fg color
        if ( reinit ) {
            QBrush defaultBrush;
            cbrush = defaultBrush;
        }
        bg_col = w->backgroundColor();          // use widget bg color
        ww = vw = w->width();                   // default view size
        wh = vh = w->height();
	unclipped = w->testWFlags(WPaintUnclipped);
        if ( unclipped ) // paint direct on device
            setf( NoCache );

    } else if ( pdev->devType() == QInternal::Pixmap ) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)pdev;
        if ( pm->isNull() ) {
#if defined(QT_CHECK_NULL)
            warning( "QPainter::begin: Cannot paint null pixmap" );
#endif
            end();
            return FALSE;
        }
        ww = vw = pm->width();                  // default view size
        wh = vh = pm->height();

	initPaintDevice();

    } 

    if ( testf(ExtDev) ) {               // external device
        // FIXME: Untested modification
        ww = vw = pdev->metric( QPaintDeviceMetrics::PdmWidth ); // sanders
        wh = vh = pdev->metric( QPaintDeviceMetrics::PdmHeight ); // sanders
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
    return TRUE;
}

bool fuckery=FALSE;
bool QPainter::end()				// end painting
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
        warning( "QPainter::end: Missing begin() or begin() failed" );
#endif
        return FALSE;
    }

    if(fuckery)
	qDebug("foo..");
    if ( testf(ExtDev) ) {
	if(fuckery)
	    qDebug("bar..");
	pdev->cmd( QPaintDevice::PdcEnd, this, 0 );
    }

    if ( testf( FontMet ) )                       // remove references to this
        QFontMetrics::reset( this );
    if ( testf( FontInf ) )                       // remove references to this
        QFontInfo::reset( this );

#ifndef ONE_PIXEL_LOCK
    if ( pdev->devType() == QInternal::Pixmap )
	UnlockPixels(GetGWorldPixMap((GWorldPtr)pdev->handle()));
#endif

    //reset the value we got in begin()
    delete saved;
    saved = NULL;
    
    flags = 0;
    pdev->painters--;
    pdev = 0;
    return TRUE;
}

void QPainter::flush()
{
    if(!isActive())
	return;

    if ( pdev->devType() == QInternal::Widget ) 
	QDFlushPortBuffer(GetWindowPort((WindowPtr)((QWidget *)pdev)->handle()), NULL);
    else if( pdev->devType() == QInternal::Pixmap )
	QDFlushPortBuffer((GWorldPtr)((QPixmap *)pdev)->handle(), NULL);
}

void QPainter::setBackgroundColor( const QColor &c )
{
    if ( !isActive() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QPainter::setBackgroundColor: Call begin() first" );
#endif
	return;
    }
    bg_col = c;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].color = &bg_col;
	if ( !pdev->cmd( QPaintDevice::PdcSetBkColor, this, param ) || !hd )
	    return;
    }
    if ( !penRef )
	updatePen();				// update pen setting
    if ( !brushRef )
	updateBrush();				// update brush setting

}

void QPainter::setBackgroundMode( BGMode m)
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::setBackgroundMode: Call begin() first" );
#endif
	return;
    }
    if ( m != TransparentMode && m != OpaqueMode ) {
#if defined(CHECK_RANGE)
	qWarning( "QPainter::setBackgroundMode: Invalid mode" );
#endif
	return;
    }
    bg_mode = m;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = m;
	if ( !pdev->cmd( QPaintDevice::PdcSetBkMode, this, param ) || !hd )
	    return;
    }
    if ( !penRef )
	updatePen();				// update pen setting
    if ( !brushRef )
	updateBrush();				// update brush setting
}

void QPainter::setRasterOp( RasterOp r )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::setRasterOp: Call begin() first" );
#endif
	return;
    }
    if ( (uint)r > LastROP ) {
#if defined(CHECK_RANGE)
	qWarning( "QPainter::setRasterOp: Invalid ROP code" );
#endif
	return;
    }
    if(ropCodes[r] == 666) {
	qWarning("Woops, we don't have that rasterop, FIXME!!");
	r = XorROP;
    }
    rop = r;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	if ( !pdev->cmd( QPaintDevice::PdcSetROP, this, param ) || !hd )
	    return;
    }
    if ( penRef )
	updatePen();				// get non-cached pen GC
    if ( brushRef )
	updateBrush();				// get non-cached brush GC
}

void QPainter::setBrushOrigin( int x, int y )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::setBrushOrigin: Call begin() first" );
#endif
	return;
    }
    bro = QPoint(x,y);
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	if ( !pdev->cmd( QPaintDevice::PdcSetBrushOrigin, this, param ) ||
	     !hd )
	    return;
    }
    if ( brushRef )
	updateBrush();				// get non-cached brush GC
}

void QPainter::setClipping( bool b )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::setClipping: Call begin() first" );
#endif
	return;
    }

    initPaintDevice();
    QRegion reg;
    if(b) {
	setf(ClipOn);
	if(!crgn.isNull())
	    reg = crgn;
    } else {
	clearf(ClipOn);
    }

    if(!clippedreg.isNull()) {
	if(reg.isNull())
	    reg = clippedreg;
	else 
	    reg &= clippedreg;
    }
    SetClip((RgnHandle)reg.handle());
}


void QPainter::setClipRect( const QRect &r )
{
    setClipRegion(QRegion(r));
}

void QPainter::setClipRegion( const QRegion &r )
{
    if ( !isActive() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::setClipRegion: Call begin() first" );
#endif
	return;
    }

    initPaintDevice();
    QRegion rset(r);
    rset.translate(offx, offy);
    crgn = rset;

    if(!clippedreg.isNull()) {
	if(r.isNull())
	    rset = clippedreg;
	else 
	    rset &= clippedreg;
    }
    setf( ClipOn );
    SetClip((RgnHandle)rset.handle());
}

void QPainter::drawPolyInternal( const QPointArray &a, bool close )
{
    initPaintDevice();

    RgnHandle polyRegion = NewRgn();
    OpenRgn();
    uint loopc;
    MoveTo( a[0].x()+offx, a[0].y()+offy );
    for ( loopc = 1; loopc < a.size(); loopc++ ) {
	LineTo( a[loopc].x()+offx, a[loopc].y()+offy );
	MoveTo( a[loopc].x()+offx, a[loopc].y()+offy );
    }
    LineTo( a[0].x()+offx, a[0].y()+offy );
    CloseRgn( polyRegion );
    if( close && this->brush().style() != NoBrush) {
	updateBrush();
	if( this->brush().style() == SolidPattern ) {
	    PaintRgn( polyRegion );
	} else {
	    //save the clip
	    bool clipon = testf(ClipOn);
	    QRegion clip = crgn;

	    //create the region
	    QPointArray offa = a;
	    offa.translate(offx, offy);
	    QRegion newclip(offa);
	    if(clipon && !clip.isNull())
		newclip &= clip;
	    newclip.translate(-offx, -offy);
	    setClipRegion(newclip);

	    //draw the brush
	    QRect r(offa.boundingRect());
	    //turn off translation flags
	    uint save_flags = flags;
	    flags = IsActive | ClipOn;

	    //draw the brush
	    QPixmap *pm = cbrush.data->pixmap;
	    if(pm && !pm->isNull()) 
		drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), *pm, 
				r.x() - bro.x(), r.y() - bro.y());
	    if(brush_style_pix) 
		drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), *brush_style_pix, 0, 0 );

	    //restore translation flags
	    flags = save_flags;

	    //restore the clip
	    if(clipon) 
		setClipRegion(clip);
	    else
		setClipping(FALSE);
	}
    }

    if ( cpen.style() != NoPen ) {
	updatePen();
	FrameRgn( polyRegion );
    }
    DisposeRgn( polyRegion );
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
            if ( !pdev->cmd( QPaintDevice::PdcDrawPoint, this, param ) )
                return;
        }
        map( x, y, &x, &y );
    }


    if ( cpen.style() != NoPen ) {
	initPaintDevice();
	updatePen();
	::RGBColor f;
	f.red = cpen.color().red()*256;
	f.green = cpen.color().green()*256;
	f.blue = cpen.color().blue()*256;
	SetCPixel(x+offx,y+offy,&f);
    }
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
		if ( !pdev->cmd(QPaintDevice::PdcDrawPoint,this,param))
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
	initPaintDevice();
	updatePen();

	::RGBColor f;
	f.red = cpen.color().red()*256;
	f.green = cpen.color().green()*256;
	f.blue = cpen.color().blue()*256;
	for (int i=0; i<npoints; i++) 
	    SetCPixel(pa[index+i].x()+offx,pa[index+i].y()+offy,&f);
    }
}


void QPainter::moveTo( int x, int y )
{
  if ( !isActive() )
    return;
  if ( testf(ExtDev|VxF|WxF) ) {
    if ( testf(ExtDev) ) {
      QPDevCmdParam param[1];
      QPoint p( x, y );
      param[0].point = &p;
      if ( !pdev->cmd(QPaintDevice::PdcMoveTo,this,param) /*|| !hdc*/ )
	return;
    }
    map( x, y, &x, &y );
  }

  penx = x;
  peny = y;
}

void QPainter::lineTo( int x, int y )
{
  if ( !isActive() )
    return;
  if ( testf(ExtDev|VxF|WxF) ) {
    if ( testf(ExtDev) ) {
      QPDevCmdParam param[1];
      QPoint p( x, y );
      param[0].point = &p;
      if ( !pdev->cmd( QPaintDevice::PdcLineTo, this, param ) /*|| !hdc*/ )
	return;
    }
    map( x, y, &x, &y );
  }

  initPaintDevice();
  updateBrush();
  updatePen();
  MoveTo(penx+offx,peny+offy);
  LineTo(x+offx,y+offy);
  penx = x;
  peny = y;
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
      if ( !pdev->cmd( QPaintDevice::PdcDrawLine, this, param ) /*|| !hdc*/ )
	return;
    }
    map( x1, y1, &x1, &y1 );
    map( x2, y2, &x2, &y2 );
  }

  initPaintDevice();
  updatePen();
  MoveTo(x1+offx,y1+offy);
  LineTo(x2+offx,y2+offy);
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
            if ( !pdev->cmd( QPaintDevice::PdcDrawRect, this, param ) /*|| !hdc*/ )
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
#if 0 //FIXME, this was causing an off by one, I'm not sure if every paint is going to have this
    //problem or if this calculation is unnecesary, I need to revisit (canonball on t14 exhibited a problem)
    if ( cpen.style() == NoPen ) {
	w++;
        h++;
    }
#endif

    initPaintDevice();
    Rect rect;
    SetRect( &rect, x+offx, y+offy, x + w+offx, y + h+offy);
    if( this->brush().style() != NoBrush) {
	updateBrush();
	if( this->brush().style() == SolidPattern ) {
	    PaintRect( &rect );
	} else {
	    //save the clip
	    bool clipon = testf(ClipOn);
	    QRegion clip = crgn;

	    //create the region
	    QRect qr(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top);
	    QRegion newclip(qr);
	    if(clipon && !clip.isNull())
		newclip &= clip;
	    newclip.translate(-offx, -offy);
	    setClipRegion(newclip);

	    //turn off translation flags
	    uint save_flags = flags;
	    flags = IsActive | ClipOn;

	    //draw the brush
	    QPixmap *pm = cbrush.data->pixmap;
	    if(pm && !pm->isNull()) 
		drawTiledPixmap(x, y, w, h, *pm, x - bro.x(), y - bro.y());
	    if(brush_style_pix) 
		drawTiledPixmap(x, y, w, h, *brush_style_pix, 0, 0 );

	    //restore translation flags
	    flags = save_flags;

	    //restore the clip
	    if(clipon) 
		setClipRegion(clip);
	    else
		setClipping(FALSE);
	}
    }

    if( cpen.style() != NoPen ) {
	updatePen();
	FrameRect(&rect);
    }
}

void QPainter::drawWinFocusRect( int x, int y, int w, int h )
{
    drawWinFocusRect( x, y, w, h, TRUE, color0 );
}

void QPainter::drawWinFocusRect( int x, int y, int w, int h,
				 const QColor &bgColor )
{
    drawWinFocusRect( x, y, w, h, FALSE, bgColor);
}

void QPainter::drawWinFocusRect( int x, int y, int w, int h,
                                 bool xorPaint, const QColor &bgColor )
{
    if ( !isActive() )
        return;
#ifndef QT_NO_TRANSFORMATIONS
    if ( txop == TxRotShear )
        return;
#endif
    QPen    old_pen = cpen;
    QBrush  old_brush = cbrush;
    RasterOp old_rop = (RasterOp)rop;

    setBrush( QBrush() );

    if ( xorPaint ) {
        if ( QColor::numBitPlanes() <= 8 )
            setPen( color1 );
        else
            setPen( white );
        setRasterOp( XorROP );
    } else {
        if ( qGray( bgColor.rgb() ) < 128 )
            setPen( white );
        else
            setPen( black );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }

    cpen.setStyle( DashLine );
    updatePen();
    if ( cpen.style() != NoPen ) {
	drawLine(x,y,x+(w-1),y);
	drawLine(x+(w-1),y,x+(w-1),y+(h-1));
	drawLine(x,y+(h-1),x+(w-1),y+(h-1));
	drawLine(x,y,x,y+(h-1));
	x++;
	y++;
	w -= 2;
	h -= 2;
    }

    setRasterOp( old_rop );
    setPen( old_pen );
    setBrush( old_brush );
}

void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd)
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
            if ( !pdev->cmd( QPaintDevice::PdcDrawRoundRect, this, param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
	    if ( w <= 0 || h <= 0 )
		fix_neg_rect( &x, &y, &w, &h );
	    w--;
	    h--;
	    int rxx = w*xRnd/200;
	    int ryy = h*yRnd/200;
	    // were there overflows?
	    if ( rxx < 0 )
		rxx = w/200*xRnd;
	    if ( ryy < 0 )
		ryy = h/200*yRnd;
	    int rxx2 = 2*rxx;
	    int ryy2 = 2*ryy;
	    QPointArray a[4];
	    a[0].makeArc( x, y, rxx2, ryy2, 1*16*90, 16*90, xmat );
	    a[1].makeArc( x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, xmat );
	    a[2].makeArc( x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, xmat );
	    a[3].makeArc( x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, xmat );
	    // ### is there a better way to join QPointArrays?
	    QPointArray aa;
	    aa.resize( a[0].size() + a[1].size() + a[2].size() + a[3].size() );
	    uint j = 0;
	    for ( int k=0; k<4; k++ ) {
		for ( uint i=0; i<a[k].size(); i++ ) {
		    aa.setPoint( j, a[k].point(i) );
		    j++;
		}
	    }
	    drawPolyInternal( aa );
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

    initPaintDevice();
    Rect rect;
    SetRect( &rect, x+offx, y+offy, x + w+offx, y + h+offy );
    if( this->brush().style() == SolidPattern ) {
	updateBrush();
	PaintRoundRect( &rect, w*xRnd/100, h*yRnd/100 );
    }
    if( cpen.style() != NoPen ) {
	updatePen();
	FrameRoundRect( &rect, w*xRnd/100, h*yRnd/100 );
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
            if ( !pdev->cmd( QPaintDevice::PdcDrawEllipse, this, param ) /*|| !hdc*/ )
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

    initPaintDevice();
    Rect r;
    SetRect( &r, x+offx, y+offy, x + w+offx, y + h+offy );

    if( this->brush().style() != NoBrush) {
	updateBrush();

	if( this->brush().style() == SolidPattern ) {
	    PaintOval( &r );
	} else {

	    //save the clip
	    bool clipon = testf(ClipOn);
	    QRegion clip = crgn;

	    //create the region
	    QRect qr(r.left, r.top, r.right-r.left, r.bottom-r.top);
	    QRegion newclip(qr, QRegion::Ellipse);
	    if(clipon && !clip.isNull())
		newclip &= clip;
	    newclip.translate(-offx, -offy);
	    setClipRegion(newclip);

	    //turn off translation flags
	    uint save_flags = flags;
	    flags = IsActive | ClipOn;

	    //draw the brush
	    QPixmap *pm = cbrush.data->pixmap;
	    if(pm && !pm->isNull()) 
		drawTiledPixmap(x, y, w, h, *pm, x - bro.x(), y - bro.y());
	    if(brush_style_pix) 
		drawTiledPixmap(x, y, w, h, *brush_style_pix, 0, 0 );

	    //restore translation flags
	    flags = save_flags;

	    //restore the clip
	    if(clipon) 
		setClipRegion(clip);
	    else
		setClipping(FALSE);
	}
    }

    updatePen();
    FrameOval( &r );
}


void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
    // FIXME transformation is broken
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawArc, this, param ) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat );    // arc polyline
            drawPolyInternal( pa, TRUE );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }

    initPaintDevice();
    Rect bounds;
    SetRect(&bounds,x+offx,y+offy,x+w+offx,y+h+offy);
    updatePen();
    FrameArc(&bounds,a/16,alen/16);
}


void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{
    if ( a > (360*16) ) {
      a = a % (360*16);
    } else if ( a < 0 ) {
      a = a % (360*16);
      if ( a < 0 ) a += (360*16);
    }
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPie, this, param) /*|| !hdc*/ )
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

    initPaintDevice();
    Rect bounds;
    SetRect(&bounds,x+offx,y+offy,x+w+offx,y+h+offy);
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


// FIXME: Implement this
void QPainter::drawChord( int, int, int, int, int, int )
{
}


// FIXME: Support dash lines?
void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{
    if ( nlines < 0 )
        nlines = a.size()/2 - index/2;
    if ( index + nlines*2 > (int)a.size() )
        nlines = (a.size() - index)/2;
    if ( !isActive() || nlines < 1 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf( ExtDev|VxF|WxF ) ) {
        if ( testf( ExtDev ) ) {
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

    initPaintDevice();
    updatePen();
    while ( nlines-- ) {
        pa.point( i++, &x1, &y1 );
        pa.point( i++, &x2, &y2 );
        MoveTo(x1 + offx, y1 + offy);
        LineTo(x2 + offx, y2 + offy);
    }
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
    if ( testf( ExtDev|VxF|WxF ) ) {
        if ( testf( ExtDev ) ) {
            if ( npoints != (int)pa.size() ) {
                pa = QPointArray( npoints );
                for ( int i=0; i<npoints; i++ )
                    pa.setPoint( i, a.point(index+i) );
                index = 0;
            }
            QPDevCmdParam param[1];
            param[0].ptarr = (QPointArray*)&pa;
            if ( !pdev->cmd(QPaintDevice::PdcDrawPolyline,this,param) /*|| !hdc*/ )
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
    initPaintDevice();
    updateBrush();
    updatePen();
    MoveTo(pa[0].x() + offx, pa[0].y() + offy );
    for( loopc = 1; loopc < (int)pa.size(); loopc++ ) {
	LineTo( pa[loopc].x() + offx ,pa[loopc].y() + offy );
    }
}


void QPainter::drawConvexPolygon( const QPointArray &pa,
			     int index, int npoints )
{
    // Any efficient way?
    drawPolygon(pa,FALSE,index,npoints);
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
            if ( !pdev->cmd( QPaintDevice::PdcDrawPolygon, this, param ) /*|!hdc*/ )
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
    drawPolyInternal(pa,true);
}


/*!
  Draws a cubic Bezier curve defined by the control points in \a a,
  starting at \a a[index].  (\a index defaults to 0.)

  Control points after \a a[index+3] are ignored.  Nothing happens if
  there aren't enough control points.
*/

// FIXME: Implement this
void QPainter::drawCubicBezier( const QPointArray &, int )
{
}

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh )
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

    initPaintDevice();
    if ( testf(ExtDev|VxF|WxF) ) {
	if(txop == TxScale ) {
	    // Plain scaling, then unclippedScaledBitBlt is fastest
	    int w, h;
	    map( x, y, sw, sh, &x, &y, &w, &h );
	    unclippedScaledBitBlt( pdev, x, y, w, h, &pixmap, sx, sy, sw, sh, (RasterOp)rop, FALSE );
	    return;
        } else if ( testf(ExtDev) || txop == TxRotShear ) {
            if ( sx != 0 || sy != 0 ||
                 sw != pixmap.width() || sh != pixmap.height() ) {
                QPixmap tmp( sw, sh, pixmap.depth() );
                unclippedBitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, TRUE );
                if ( pixmap.mask() ) {
                    QBitmap mask( sw, sh );
                    unclippedBitBlt( &mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
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
                if ( !pdev->cmd(QPaintDevice::PdcDrawPixmap,this,param) /* || !hdc */ ) {
                    return;
		}
            }
	    if ( txop == TxScale || txop == TxRotShear ) {
		QWMatrix mat( m11(), m12(),
			      m21(), m22(),
			      dx(),  dy() );
		mat = QPixmap::trueMatrix( mat, sw, sh );
		QPixmap pm = pixmap.xForm( mat );
		if ( !pm.mask() && txop == TxRotShear ) {
		    QBitmap bm_clip( sw, sh, TRUE );
		    bm_clip.fill( color1 );
		    pm.setMask( bm_clip.xForm(mat) );
		}
		map( x, y, &x, &y );		// compute position of pixmap
		int dx, dy;
		mat.map( 0, 0, &dx, &dy );
		unclippedBitBlt( pdev, x-dx, y-dy, &pm, 0, 0, pm.width(), pm.height(), (RasterOp)rop, FALSE );
		return;
	    }
	}

	if ( txop == TxTranslate ) 
	    map( x, y, &x, &y );
    }
    unclippedBitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop, FALSE );
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
    drawTile( this, x, y, w, h, pixmap, sx, sy );
}

void QPainter::drawText( int x, int y, const QString &str, int from, int len, QPainter::TextDirection dir) 
{
    drawText(x, y, str.mid(from), len);
}

void QPainter::drawText( int x, int y, const QString &str, int len, QPainter::TextDirection dir)
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = str.length();
    if ( len == 0 )                             // empty string
	return;

    updateBrush();

    if ( testf(DirtyFont) ) 
	updateFont();

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p( x, y );
	QString newstr = str.left(len);
	param[0].point = &p;
	param[1].str = &newstr;
	if ( !pdev->cmd(QPaintDevice::PdcDrawText2,this,param) || !hd )
	    return;
    }

    if ( testf(VxF|WxF) ) {
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
	    if ( txop <= TxScale && pdev->devType() != QInternal::Printer ) {
		int newSize = qRound( m22() * (double)cfont.pointSize() ) - 1;
		newSize = QMAX( 6, QMIN( newSize, 72 ) ); // empirical values
		dfont.setPointSize( newSize );
		QFontMetrics fm2( dfont );
		QRect abbox = fm2.boundingRect( str, len );
		aw = abbox.width();
		ah = abbox.height();
		tx = -abbox.x();
		ty = -abbox.y();	// text position - off-by-one?
		if ( aw == 0 || ah == 0 )
		    return;
		double rx = (double)bbox.width() * mat1.m11() / (double)aw;
		double ry = (double)bbox.height() * mat1.m22() /(double)ah;
		mat2 = QWMatrix( rx, 0, 0, ry, 0, 0 );
	    } else {
		mat2 = QPixmap::trueMatrix( mat1, w, h );
		aw = w;
		ah = h;
	    }
	    bool empty = aw == 0 || ah == 0;
	    QString bm_key = gen_text_bitmap_key( mat2, dfont, str, len );
	    QBitmap *wx_bm = get_text_bitmap( bm_key );
	    bool create_new_bm = wx_bm == 0;
	    if ( create_new_bm && !empty ) {    // no such cached bitmap
		QBitmap bm( aw, ah, TRUE );	// create bitmap
		QPainter paint(&bm); 		// draw text in bitmap
		paint.setPen( color1 );
		paint.setFont( dfont );
#if 1
		paint.drawText( tx, ty, str, len );
#else
		paint.drawPoint( aw - 4, ah - 1 );
		paint.drawPoint( aw - 5, ah - 1 );
		qDebug( "aw %d, ah %d", aw - 1 , ah - 1 );
#endif
		paint.end();

		wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
		if ( wx_bm->isNull() ) {
		    delete wx_bm;		// nothing to draw
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
	    unclippedBitBlt(pdev, x, y, wx_bm, 0, 0, wx_bm->width(), 
			    wx_bm->height(), CopyROP, TRUE );
	    if(create_new_bm)
		ins_text_bitmap( bm_key, wx_bm );
	    return;
	}
	if ( txop == TxTranslate )
	    map( x, y, &x, &y );
    }

    QTextCodec *mapper = QTextCodec::codecForMib( 106 ); //utf8 always, this should do unicode FIXME
    QCString mapped = mapper->fromUnicode(str,len);

    if(!mapped) {
	char * soo=new char[2];
	soo[0]='?';
	soo[1]='\0';
	mapped=soo;
    }

    initPaintDevice();
    if ( !cfont.handle() ) {
	if ( mapped.isNull() )
	    warning("Fontsets only apply to mapped encodings");
	else {
	    MoveTo(x+offx,y+offy);
	    updatePen();
	    DrawText(mapped, 0, mapped.length());
	}
    } else {
	if ( !mapped.isNull() ) {
	    MoveTo(x+offx,y+offy);
	    updatePen();
	    DrawText(mapped, 0, mapped.length());
	} else {
	    // Unicode font

	    QString v = str;
#ifdef QT_BIDI
	    v.compose();  // apply ligatures (for arabic, etc...)
	    v = v.visual(); // visual ordering
	    len = v.length();
#endif

	    MoveTo(x+offx,y+offy);
	    updatePen();
	    DrawText(mapped, 0, mapped.length());
	}
    }

#if 0
    if ( cfont.underline() || cfont.strikeOut() ) {
	const QFontMetrics & fm = fontMetrics();
	int lw = fm.lineWidth();
	int tw = fm.width( str, len );
    }
#endif
}

QPoint QPainter::pos() const
{
    return QPoint(penx, peny);
}

//#define TRY_CACHE 
#ifdef TRY_CACHE
CGrafPtr lgraf = NULL;
#endif
void QPainter::initPaintDevice(bool force) {
#ifdef TRY_CACHE
    if(!force && lgraf == GetQDGlobalsThePort()) 
	return;
#endif
    
    clippedreg = QRegion(); //empty
    if( pdev->devType() == QInternal::Printer ) {
	if(pdev->handle()) {
	    SetGWorld((GrafPtr)pdev->handle(), 0); //set the gworld
	    clippedreg = QRegion(0, 0, pdev->metric(QPaintDeviceMetrics::PdmWidth), 
				 pdev->metric(QPaintDeviceMetrics::PdmHeight));
	}
    } else if ( pdev->devType() == QInternal::Widget ) {                    // device is a widget
        QWidget *w = (QWidget*)pdev;

	//set the correct window prot
	SetPortWindowPort((WindowPtr)w->handle());

	//offset painting in widget relative the tld
	QPoint wp(posInWindow(w));
	offx = wp.x();
	offy = wp.y();

	if(!w->isVisible()) 
	    clippedreg = QRegion(0, 0, 0, 0); //make the clipped reg empty if its not visible, this is hacky FIXME!!!
        else if ( unclipped ) 
	    clippedreg = w->clippedRegion(FALSE);	    //just clip my bounding rect
	else if(!paintevents.isEmpty() && (*paintevents.current()) == pdev) 
	    clippedreg = paintevents.current()->region();
	else 
	    clippedreg = w->clippedRegion();

    } else if ( pdev->devType() == QInternal::Pixmap ) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)pdev;

	//setup the gworld
	SetGWorld((GWorldPtr)pm->handle(),0);
#ifndef ONE_PIXEL_LOCK
	Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)pm->handle())));
#endif

	//clip out my bounding rect
	clippedreg = QRegion(0, 0, pm->width(), pm->height());
    } 

    QRegion reg; 
    if(testf(ClipOn) && !crgn.isNull())
	reg = crgn;
    if(!clippedreg.isNull()) {
	if(reg.isNull())
	    reg = clippedreg;
	else
	    reg &= clippedreg;
    }
    SetClip((RgnHandle)reg.handle());

#ifdef TRY_CACHE
    //save it
    lgraf = GetQDGlobalsThePort();
#endif
}

