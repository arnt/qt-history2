/****************************************************************************
** $Id: $
**
** Implementation of QCursor class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qnamespace.h"
#include "qt_mac.h"
#ifdef Q_WS_MACX
#include <CGRemoteOperation.h>
#endif
#include <stdlib.h>

// NOT REVISED


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static QCursor cursorTable[Qt::LastCursor+1];
static const int arrowCursorIdx = 0;
QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[0];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[1];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[2];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[3];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[4];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[5];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[6];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[7];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[8];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[9];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[10];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[11];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[12];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[13];
QT_STATIC_CONST_IMPL QCursor & Qt::forbiddenCursor = cursorTable[14];
QT_STATIC_CONST_IMPL QCursor & Qt::whatsThisCursor = cursorTable[15];

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

#ifndef QMAC_NO_FAKECURSOR
class QMacCursorWidget : public QWidget
{
    Q_OBJECT
    QPixmap *pm;
public:
    QMacCursorWidget(QBitmap *mask, QPixmap *pix) :
	QWidget(0, "fake_cursor", WType_Dialog | WStyle_Customize | WStyle_NoBorder)
	{
	    pm = pix;
	    hide();
	    resize(pm->width(), pm->height());
	    setMask(*mask);
	    ChangeWindowAttributes((WindowPtr)handle(), kWindowNoShadowAttribute, 0);
	}
    ~QMacCursorWidget() { }
protected:
    void paintEvent(QPaintEvent *) { bitBlt(this, 0, 0, pm); }
};
#include "qcursor_mac.moc"
#endif

struct QCursorData : public QShared
{
    QCursorData( int s = 0 );
   ~QCursorData();

    int id;
    int	      cshape;
    int hx, hy;
    QBitmap  *bm, *bmm;

    enum { TYPE_None, TYPE_CursorImage, TYPE_CursPtr, TYPE_ThemeCursor, TYPE_FakeCursor } type;
    union {
	struct {
	    uint my_cursor:1;
	    CursPtr   hcurs;
	} cp;
#ifndef QMAC_NO_FAKECURSOR
	struct {
	    QMacCursorWidget *widget;
	    CursPtr empty_curs;
	} fc;
#endif
	CursorImageRec *ci;
	ThemeCursor tc;
    } curs;
};

static QCursorData *currentCursor = NULL; //current cursor
void qt_mac_set_cursor(const QCursor *c, const Point *p)
{
    (void)c->handle(); //force the cursor to get loaded, if it's not

#ifndef QMAC_NO_FAKECURSOR
    if(c->data->type == QCursorData::TYPE_FakeCursor) {
	/* That's right folks, I want nice big cursors - if apple won't give them to me, why
	   I'll just take them!!! */
	c->data->curs.fc.widget->move(p->h - c->data->curs.fc.empty_curs->hotSpot.h,
				      p->v - c->data->curs.fc.empty_curs->hotSpot.v);
	SetCursor(c->data->curs.fc.empty_curs);
	if(!c->data->curs.fc.widget->isVisible())
	    c->data->curs.fc.widget->show();
    } else
#else
    Q_UNUSED(p);
#endif
    if(currentCursor != c->data) {
#ifndef QMAC_NO_FAKECURSOR
	if(currentCursor && currentCursor->type == QCursorData::TYPE_FakeCursor)
	    currentCursor->curs.fc.widget->hide();
#endif
	if(c->data->type == QCursorData::TYPE_CursPtr) {
	    SetCursor(c->data->curs.cp.hcurs);
	} else if(c->data->type == QCursorData::TYPE_CursorImage) {

	} else if(c->data->type == QCursorData::TYPE_ThemeCursor) {
	    switch(c->data->curs.tc) {
	    case kThemeWatchCursor:
		SetAnimatedThemeCursor(c->data->curs.tc, 1);
		break;
	    default:
		SetThemeCursor(c->data->curs.tc);
		break;
	    }
	} else {
	    qDebug("whoa! that shouldn't happen!");
	}
    }
    currentCursor = c->data;
}

QCursorData::QCursorData( int s )
{
    cshape = s;
    bm = bmm = 0;
    hx = hy  = -1;
    type = TYPE_None;

    static int static_id = 121578; //the holy day
    id = static_id++;
}

QCursorData::~QCursorData()
{
    if(type == TYPE_CursPtr) {
	if ( curs.cp.hcurs && curs.cp.my_cursor )
	    free(curs.cp.hcurs);
    } else if(type == TYPE_CursorImage) {
	free(curs.ci);
    } else if(type == TYPE_FakeCursor) {
#ifndef QMAC_NO_FAKECURSOR
	free(curs.fc.empty_curs);
	delete curs.fc.widget;
#endif
    }
    type = TYPE_None;

    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
    if(currentCursor == this)
	currentCursor = NULL;
}

QCursor *QCursor::find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= LastCursor ? &cursorTable[shape] : 0;
}


static bool initialized = FALSE;
void QCursor::cleanup()
{
    if ( !initialized )
	return;

    int shape;
    for( shape = 0; shape <= LastCursor; shape++ ) {
	delete cursorTable[shape].data;
	cursorTable[shape].data = 0;
    }
    initialized = FALSE;
}

void QCursor::initialize()
{
    InitCursor();
    int shape;
    for( shape = 0; shape <= LastCursor; shape++ )
	cursorTable[shape].data = new QCursorData( shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
}

QCursor::QCursor()
{
    if ( !initialized ) {
	if ( qApp->startingUp() ) {
	    data = 0;
	    return;
	}
	initialize();
    }
    QCursor* c = &cursorTable[arrowCursorIdx];
    c->data->ref();
    data = c->data;
}

QCursor::QCursor(int shape)
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
    c->data->ref();
    data = c->data;
}

void QCursor::setBitmap( const QBitmap &bitmap, const QBitmap &mask,
			 int hotX, int hotY )
{
    if ( !initialized )
	initialize();
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
	 bitmap.size() != mask.size() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif
	QCursor *c = &cursorTable[arrowCursorIdx];
	c->data->ref();
	data = c->data;
	return;
    }
    data = new QCursorData;
    Q_CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;
}

QCursor::QCursor( const QCursor &c )
{
    if ( !initialized )
	initialize();
    data = c.data;				// shallow copy
    data->ref();
}

QCursor::~QCursor()
{
    if ( data && data->deref() )
	delete data;
}

QCursor &QCursor::operator=( const QCursor &c )
{
    if ( !initialized )
	initialize();
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}

int QCursor::shape() const
{
    if ( !initialized )
	initialize();
    return data->cshape;
}

void QCursor::setShape( int shape )
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
    c->data->ref();
    data = c->data;
}

const QBitmap *QCursor::bitmap() const
{
    if ( !initialized )
	initialize();
    return data->bm;
}


const QBitmap *QCursor::mask() const
{
    if ( !initialized )
	initialize();
    return data->bmm;
}

QPoint QCursor::hotSpot() const
{
    if ( !initialized )
	initialize();
    return QPoint( data->hx, data->hy );
}

Qt::HANDLE QCursor::handle() const
{
    if ( !initialized )
	initialize();
    if( data->type == QCursorData::TYPE_None )
	update();
    return (Qt::HANDLE)data->id;
}

QPoint QCursor::pos()
{
    Point p;
    GetGlobalMouse(&p);
    return QPoint(p.h, p.v);
}


void QCursor::setPos( int x, int y)
{
#ifdef Q_WS_MACX
    CGPoint p;
    p.x = x;
    p.y = y;
    CGWarpMouseCursorPosition( p );
#else
// some kruft I found on the web.. it doesn't work, but I want to test more FIXME
#   define MTemp 0x828
#   define RawMouse 0x82c
#   define CrsrNewCouple 0x8ce
    HideCursor();
    Point where;
    where.h = x;
    where.v = y;
    *((Point *) RawMouse) = where ;
    *((Point *) MTemp) = where ;
    *((short *) CrsrNewCouple) = -1 ;
    ShowCursor ( ) ;
#endif
}

void QCursor::update() const
{
    if ( !initialized )
	initialize();
    register QCursorData *d = data;		// cheat const!
    if ( d->type != QCursorData::TYPE_None )				// already loaded
	return;

    /* Note to self... ***
     * mask x data
     * 0xFF x 0x00 == fully opaque white
     * 0x00 x 0xFF == xor'd black
     * 0xFF x 0xFF == fully opaque black
     * 0x00 x 0x00 == fully transparent
     */

    switch ( d->cshape ) {			// map Q cursor to MAC cursor
    case BitmapCursor: {
	if(d->bm->width() == 16 && d->bm->height() == 16) {
	    d->type = QCursorData::TYPE_CursPtr;
	    d->curs.cp.my_cursor = TRUE;
	    d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	    QImage bmi, bmmi;
	    bmi = *d->bm;
	    bmmi = *d->bmm;

	    memset(d->curs.cp.hcurs->mask, 0, 32);
	    memset(d->curs.cp.hcurs->data, 0, 32);
	    for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 16; x++) {
		    if(!bmmi.pixel(x,y)) {
			if(bmi.pixel(x,y)) {
			    *(((uchar*)d->curs.cp.hcurs->mask) + (y*2) + (x / 8)) |= (1 << (7 - (x % 8)));
			    *(((uchar*)d->curs.cp.hcurs->data) + (y*2) + (x / 8)) |= (1 << (7 - (x % 8)));
			} else {
			    *(((uchar*)d->curs.cp.hcurs->mask) + (y*2) + (x / 8)) |= (1 << (7 - (x % 8)));
			}
		    }
		}
	    }
	} else {
#ifndef QMAC_NO_FAKECURSOR
	    d->type = QCursorData::TYPE_FakeCursor;
	    d->curs.fc.widget = new QMacCursorWidget(d->bmm, d->bm);
	    //make an empty cursor
	    d->curs.fc.empty_curs = (CursPtr)malloc(sizeof(Cursor));
	    memset(d->curs.fc.empty_curs->data, 0x00, sizeof(d->curs.fc.empty_curs->data));
	    memset(d->curs.fc.empty_curs->mask, 0x00, sizeof(d->curs.fc.empty_curs->mask));
	    d->curs.fc.empty_curs->hotSpot.h = data->hx >= 0 ? data->hx : 8;
	    d->curs.fc.empty_curs->hotSpot.v = data->hy >= 0 ? data->hy : 8;
#else
	    d->type = QCursorData::TYPE_CursorImage;
	    d->curs.ci = (CursorImageRec*)malloc(sizeof(CursorImageRec));
	    d->curs.ci->majorVersion = kCursorImageMajorVersion;
	    d->curs.ci->minorVersion = kCursorImageMinorVersion;
	    d->curs.ci->cursorPixMap = GetGWorldPixMap((GWorldPtr)d->bm->handle());
	    d->curs.ci->cursorBitMask = (BitMap **)GetGWorldPixMap((GWorldPtr)d->bmm->handle());
#endif
	}
	break; }
    case ArrowCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemeArrowCursor;
	break;
    case CrossCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemeCrossCursor;
	break;
    case WaitCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemeWatchCursor;
	break;
    case IbeamCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemeIBeamCursor;
	break;
    case SizeAllCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemePlusCursor;
	break;
    case WhatsThisCursor: //for now jus tuse the pointing hand
    case PointingHandCursor:
	d->type = QCursorData::TYPE_ThemeCursor;
	d->curs.tc = kThemePointingHandCursor;
	break;

#define QT_USE_APPROXIMATE_CURSORS
#ifdef QT_USE_APPROXIMATE_CURSORS
    case SizeVerCursor:
    {
	static const uchar cur_ver_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
	    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0f, 0xf0,
	    0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00 };
	static const uchar mcur_ver_bits[] = {
	    0x00, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8,
	    0x7f, 0xfc, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x7f, 0xfc, 0x3f, 0xf8,
	    0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
	break;
    }

    case SizeHorCursor:
    {
	static const uchar cur_hor_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x18, 0x30,
	    0x38, 0x38, 0x7f, 0xfc, 0x7f, 0xfc, 0x38, 0x38, 0x18, 0x30, 0x08, 0x20,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_hor_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x0c, 0x60, 0x1c, 0x70, 0x3c, 0x78,
	    0x7f, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0x7f, 0xfc, 0x3c, 0x78,
	    0x1c, 0x70, 0x0c, 0x60, 0x04, 0x40, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_hor_bits, sizeof(cur_hor_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_hor_bits, sizeof(mcur_hor_bits));
	break;
    }

    case SizeBDiagCursor:
    {
	static const uchar cur_fdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0xf8, 0x00, 0x78,
	    0x00, 0xf8, 0x01, 0xd8, 0x23, 0x88, 0x37, 0x00, 0x3e, 0x00, 0x3c, 0x00,
	    0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_fdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0xfc,
	    0x41, 0xfc, 0x63, 0xfc, 0x77, 0xdc, 0x7f, 0x8c, 0x7f, 0x04, 0x7e, 0x00,
	    0x7f, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_fdiag_bits, sizeof(cur_fdiag_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_fdiag_bits, sizeof(mcur_fdiag_bits));
	break;
    }
    case SizeFDiagCursor:
    {
	static const uchar cur_bdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e, 0x00,
	    0x37, 0x00, 0x23, 0x88, 0x01, 0xd8, 0x00, 0xf8, 0x00, 0x78, 0x00, 0xf8,
	    0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_bdiag_bits[] = {
	    0x00, 0x00, 0x7f, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7e, 0x00, 0x7f, 0x04,
	    0x7f, 0x8c, 0x77, 0xdc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01, 0xfc,
	    0x03, 0xfc, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_bdiag_bits, sizeof(cur_bdiag_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_bdiag_bits, sizeof(mcur_bdiag_bits));
	break;
    }
    case ForbiddenCursor: //need a forbidden cursor! FIXME
    case BlankCursor:
    {
	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memset(d->curs.cp.hcurs->data, 0x00, sizeof(d->curs.cp.hcurs->data));
	memset(d->curs.cp.hcurs->mask, 0x00, sizeof(d->curs.cp.hcurs->data));
	break;
    }
    case UpArrowCursor:
    {
	static unsigned char cur_up_arrow_bits[] = {
	    0x00, 0x80, 0x01, 0x40, 0x01, 0x40, 0x02, 0x20, 0x02, 0x20, 0x04, 0x10,
	    0x04, 0x10, 0x08, 0x08, 0x0f, 0x78, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40,
	    0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0 };
	static unsigned char mcur_up_arrow_bits[] = {
	    0x00, 0x80, 0x01, 0xc0, 0x01, 0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x07, 0xf0,
	    0x07, 0xf0, 0x0f, 0xf8, 0x0f, 0xf8, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0,
	    0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_up_arrow_bits, sizeof(cur_up_arrow_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_up_arrow_bits, sizeof(mcur_up_arrow_bits));
	break;
    }
    case SplitVCursor:
    {
	static unsigned char cur_vsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };
	static unsigned char mcur_vsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80,
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_vsplit_bits, sizeof(cur_vsplit_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_vsplit_bits, sizeof(mcur_vsplit_bits));
	break;
    }
    case SplitHCursor:
    {
	static unsigned char cur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00,
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00,
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static unsigned char mcur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00,
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00,
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };

	d->type = QCursorData::TYPE_CursPtr;
	d->curs.cp.my_cursor = TRUE;
	d->curs.cp.hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->curs.cp.hcurs->data, cur_hsplit_bits, sizeof(cur_hsplit_bits));
	memcpy(d->curs.cp.hcurs->mask, mcur_hsplit_bits, sizeof(mcur_hsplit_bits));
	break;
    }
#endif
    default:
#if defined(QT_CHECK_RANGE)
	qWarning( "QCursor::update: Invalid cursor shape %d", d->cshape );
#endif
	return;
    }

    if(d->type == QCursorData::TYPE_CursPtr && d->curs.cp.hcurs && d->curs.cp.my_cursor) {
	d->curs.cp.hcurs->hotSpot.h = data->hx >= 0 ? data->hx : 8;
	d->curs.cp.hcurs->hotSpot.v = data->hy >= 0 ? data->hy : 8;
    }
}
