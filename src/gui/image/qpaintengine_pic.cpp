/****************************************************************************
**
** Implementation of the QPicturePaintEngine class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "private/qpaintengine_p.h"
#include "private/qpainter_p.h"
#include "qbuffer.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qpaintengine_pic_p.h"
#include "qrect.h"

static const char  *mfhdr_tag = "QPIC";		// header tag
static const Q_UINT16 mfhdr_maj = 6;		// major version #
static const Q_UINT16 mfhdr_min = 0;		// minor version #

class QPicturePaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECL_PUBLIC(QPicturePaintEngine);

public:
    QBuffer *pictb;
    QDataStream s;

    int trecs;
    QRect brect;
    bool formatOk;
    int	formatMajor;
    int	formatMinor;
    QPainter *pt;
};

#define d d_func()
#define q q_func()

QPicturePaintEngine::QPicturePaintEngine(QBuffer *buf)
    : QPaintEngine(*(new QPicturePaintEnginePrivate), CanRenderText)
{
    Q_ASSERT(buf);
    d->pictb = buf;
    d->formatMajor = mfhdr_maj;
    d->formatMinor = mfhdr_min;
    d->formatOk = false;
    d->pt = 0;
    d->trecs = 0;
}

QPicturePaintEngine::QPicturePaintEngine(QPaintEnginePrivate &dptr, QBuffer *buf)
    : QPaintEngine(dptr, CanRenderText)
{
    Q_ASSERT(buf);
    d->pictb = buf;
    d->formatMajor = mfhdr_maj;
    d->formatMinor = mfhdr_min;
    d->formatOk = false;
    d->pt = 0;
    d->trecs = 0;
}

QPicturePaintEngine::~QPicturePaintEngine()
{
}

bool QPicturePaintEngine::begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    Q_ASSERT(state->painter);
    d->pt = state->painter;
    d->s.setDevice(d->pictb);
    d->s.setVersion(d->formatMajor);

    QByteArray empty;
    d->pictb->setBuffer( empty );		// reset byte array in buffer
    d->pictb->open( IO_WriteOnly );
    d->s.writeRawBytes( mfhdr_tag, 4 );
    d->s << (Q_UINT16)0 << (Q_UINT16)d->formatMajor << (Q_UINT16)d->formatMinor;
    d->s << (Q_UINT8)PdcBegin << (Q_UINT8)sizeof(Q_INT32);
    d->brect = QRect();
    if ( d->formatMajor >= 4 ) {
	d->s << (Q_INT32)d->brect.left() << (Q_INT32)d->brect.top()
	  << (Q_INT32)d->brect.width() << (Q_INT32)d->brect.height();
    }
    d->trecs = 0;
    d->s << (Q_UINT32)d->trecs;			// total number of records
    d->formatOk = false;
    setActive(true);
    return true;
}

bool QPicturePaintEngine::end()
{
    d->trecs++;
    d->s << (Q_UINT8)PdcEnd << (Q_UINT8)0;
    int cs_start = sizeof(Q_UINT32);		// pos of checksum word
    int data_start = cs_start + sizeof(Q_UINT16);
    int brect_start = data_start + 2*sizeof(Q_INT16) + 2*sizeof(Q_UINT8);
    int pos = d->pictb->at();
    d->pictb->at( brect_start );
    if ( d->formatMajor >= 4 ) { // bounding rectangle
	d->s << (Q_INT32)d->brect.left() << (Q_INT32)d->brect.top()
	  << (Q_INT32)d->brect.width() << (Q_INT32)d->brect.height();
    }
    d->s << (Q_UINT32)d->trecs;			// write number of records
    d->pictb->at( cs_start );
    QByteArray buf = d->pictb->buffer();
    Q_UINT16 cs = (Q_UINT16)qChecksum( buf.constData()+data_start, pos-data_start );
    d->s << cs;				// write checksum
    d->pictb->close();
    setActive(false);
    return true;
}

#define SERIALIZE_CMD(c) \
    d->trecs++; \
    d->s << (Q_UINT8) c; \
    d->s << (Q_UINT8) 0; \
    pos = d->pictb->at()

void QPicturePaintEngine::updatePen(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetPen);
    d->s << ps->pen;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBrush(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetBrush);
    d->s << ps->brush;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateFont(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetFont);
    d->s << ps->font;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateRasterOp(QPainterState *ps)
{
    d->trecs++;
    d->s << (Q_UINT8) PdcSetROP;
    d->s << (Q_UINT8) 0;
    int pos = d->pictb->at();
    d->s << (Q_INT8) ps->rasterOp;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateBackground(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetBrush);
    d->s << ps->bgBrush;
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(PdcSetBkMode);
    d->s << (Q_INT8) ps->bgMode;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateXForm(QPainterState *ps)
{
//     int pos;
//     SERIALIZE_CMD(PdcSetWMatrix);
//     d->s << ps->matrix << (Q_INT8) true; // ### fix combine param
//     writeCmdLength(pos, QRect(), false);

//     SERIALIZE_CMD(PdcSetWXform);
//     d->s << (Q_INT8) ps->WxF;
//     writeCmdLength(pos, QRect(), false);

//     SERIALIZE_CMD(PdcSetVXform);
//     d->s << (Q_INT8) ps->VxF;
//     writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::updateClipRegion(QPainterState *ps)
{
    int pos;
    SERIALIZE_CMD(PdcSetClipRegion);
    d->s << ps->clipRegion << (Q_INT8) QPainter::CoordPainter; // ### fix coord mode
    writeCmdLength(pos, QRect(), false);

    SERIALIZE_CMD(PdcSetClip);
    d->s << (Q_INT8) ps->clipEnabled;
    writeCmdLength(pos, QRect(), false);
}

void QPicturePaintEngine::writeCmdLength(int pos, const QRect &r, bool corr)
{
    int newpos = d->pictb->at();		// new position
    int length = newpos - pos;
    QRect br(r);

    if ( length < 255 ) {			// write 8-bit length
	d->pictb->at(pos - 1);			// position to right index
	d->s << (Q_UINT8)length;
    } else {					// write 32-bit length
	d->s << (Q_UINT32)0;				// extend the buffer
	d->pictb->at(pos - 1);			// position to right index
	d->s << (Q_UINT8)255;			// indicate 32-bit length
	char *p = d->pictb->buffer().data();
	memmove( p+pos+4, p+pos, length );	// make room for 4 byte
	d->s << (Q_UINT32)length;
	newpos += 4;
    }
    d->pictb->at( newpos );				// set to new position

    if ( br.isValid() ) {
	if ( corr ) {				// widen bounding rect
	    int w2 = d->pt->pen().width() / 2;
	    br.setCoords( br.left() - w2, br.top() - w2,
			  br.right() + w2, br.bottom() + w2 );
	}
#ifndef QT_NO_TRANSFORMATIONS
	br = d->pt->worldMatrix().map( br );
#endif
	if ( d->pt->hasClipping() ) {
	    QRect cr = d->pt->clipRegion().boundingRect();
	    br &= cr;
	}
	if ( br.isValid() )
	    d->brect |= br;		     	// merge with existing rect
    }
}

void QPicturePaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    int pos;
    SERIALIZE_CMD(PdcDrawLine);
    d->s << p1 << p2;
    writeCmdLength(pos, QRect(p1, p2).normalize(), true);
}

void QPicturePaintEngine::drawRect(const QRect &r)
{
    int pos;
    SERIALIZE_CMD(PdcDrawRect);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPoint(const QPoint &p)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPoint);
    d->s << p;
    writeCmdLength(pos, QRect(p,p), true);
}

void QPicturePaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
}

void QPicturePaintEngine::drawWinFocusRect(const QRect &r, bool xorPaint, const QColor &bgColor)
{
}

void QPicturePaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    int pos;
    SERIALIZE_CMD(PdcDrawRoundRect);
    d->s << r << (Q_INT16)xRnd << (Q_INT16)yRnd;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawEllipse(const QRect &r)
{
    int pos;
    SERIALIZE_CMD(PdcDrawEllipse);
    d->s << r;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawArc(const QRect &r, int a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawArc);
    d->s << r << (Q_INT16)a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawPie(const QRect &r, int _a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPie);
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawChord(const QRect &r, int _a, int alen)
{
    int pos;
    SERIALIZE_CMD(PdcDrawChord);
    d->s << r << (Q_INT16)_a << (Q_INT16)alen;
    writeCmdLength(pos, r, true);
}

void QPicturePaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    int pos;
    SERIALIZE_CMD(PdcDrawLineSegments);
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPolyline);
    d->s << a;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawPolygon(const QPointArray &a, bool winding, int, int)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPolygon);
    d->s << a << (Q_INT8) winding;
    writeCmdLength(pos, a.boundingRect(), true);
}

void QPicturePaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{
    drawPolygon(a, false, index, npoints);
}

void QPicturePaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
#ifndef QT_NO_BEZIER
    int pos;
    SERIALIZE_CMD(PdcDrawCubicBezier);
    d->s << a;
    writeCmdLength(pos, a.cubicBezier().boundingRect(), true);
#endif
}

void QPicturePaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{
    int pos;
    SERIALIZE_CMD(PdcDrawPixmap);
    d->s << r << pm;
    writeCmdLength(pos, r, false);
}

void QPicturePaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{
}

void QPicturePaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
//     int pos;
//     SERIALIZE_CMD(PdcDrawText2);
//     d->s << p << QString(ti.chars, ti.num_chars);
//     writeCmdLength(pos, QRect(p, p), true);
}

/*!
  \internal

  Sets formatOk to false and resets the format version numbers to default
*/

void QPicturePaintEngine::resetFormat()
{
    d->formatOk = false;
    d->formatMajor = mfhdr_maj;
    d->formatMinor = mfhdr_min;
}


/*!
  \internal

  Checks data integrity and format version number. Set formatOk to
  true on success, to false otherwise. Returns the resulting formatOk
  value.
*/
bool QPicturePaintEngine::checkFormat()
{
    resetFormat();

    // can't check anything in an empty buffer
    if ( d->pictb->size() == 0 )
	return false;

    d->pictb->open( IO_ReadOnly );			// open buffer device
    QDataStream s;
    s.setDevice( d->pictb );			// attach data stream to buffer

    char mf_id[4];				// picture header tag
    s.readRawBytes( mf_id, 4 );			// read actual tag
    if ( memcmp(mf_id, mfhdr_tag, 4) != 0 ) { 	// wrong header id
	qWarning( "QPicturePaintEngine::checkFormat: Incorrect header" );
	d->pictb->close();
	return false;
    }

    int cs_start = sizeof(Q_UINT32);		// pos of checksum word
    int data_start = cs_start + sizeof(Q_UINT16);
    Q_UINT16 cs,ccs;
    QByteArray buf = d->pictb->buffer();	// pointer to data

    s >> cs;				// read checksum
    ccs = (Q_UINT16) qChecksum( buf.constData() + data_start, buf.size() - data_start );
    if ( ccs != cs ) {
	qWarning( "QPicturePaintEngine::checkFormat: Invalid checksum %x, %x expected",
		  ccs, cs );
	d->pictb->close();
	return false;
    }

    Q_UINT16 major, minor;
    s >> major >> minor;			// read version number
    if ( major > mfhdr_maj ) {		// new, incompatible version
	qWarning( "QPicturePaintEngine::checkFormat: Incompatible version %d.%d",
		  major, minor);
	d->pictb->close();
	return false;
    }
    s.setVersion( major != 4 ? major : 3 );

    Q_UINT8  c, clen;
    s >> c >> clen;
    if ( c == PdcBegin ) {
	if ( !( major >= 1 && major <= 3 )) {
	    Q_INT32 l, t, w, h;
	    s >> l >> t >> w >> h;
	    d->brect = QRect( l, t, w, h );
	}
    } else {
	qWarning( "QPicturePaintEngine::checkFormat: Format error" );
	d->pictb->close();
	return false;
    }
    d->pictb->close();

    d->formatOk = true;			// picture seems to be ok
    d->formatMajor = major;
    d->formatMinor = minor;
    return true;
}


bool QPicturePaintEngine::play(QPainter *painter)
{
    if ( d->pictb->size() == 0 )			// nothing recorded
	return true;

    if ( !d->formatOk && !checkFormat() )
	return false;

    d->pictb->open( IO_ReadOnly );		// open buffer device
    QDataStream s;
    s.setDevice( d->pictb );			// attach data stream to buffer
    s.device()->at( 10 );			// go directly to the data
    s.setVersion( d->formatMajor == 4 ? 3 : d->formatMajor );

    Q_UINT8  c, clen;
    Q_UINT32 nrecords;
    s >> c >> clen;
    Q_ASSERT( c == PdcBegin );
    // bounding rect was introduced in ver 4. Read in checkFormat().
    if ( d->formatMajor >= 4 ) {
	Q_INT32 dummy;
	s >> dummy >> dummy >> dummy >> dummy;
    }
    s >> nrecords;
    if ( !exec( painter, s, nrecords ) ) {
	qWarning( "QPicturePaintEngine::play: Format error" );
	d->pictb->close();
	return false;
    }
    d->pictb->close();
    return true;				// no end-command
}


/*!
  \internal
  Iterates over the internal picture data and draws the picture using
  \a painter.
*/

bool QPicturePaintEngine::exec(QPainter *painter, QDataStream &s, int nrecords)
{
#if defined(QT_DEBUG)
    int		strm_pos;
#endif
    Q_UINT8	c;				// command id
    Q_UINT8	tiny_len;			// 8-bit length descriptor
    Q_INT32	len;				// 32-bit length descriptor
    Q_INT16	i_16, i1_16, i2_16;		// parameters...
    Q_INT8	i_8;
    Q_UINT32	ul;
    QByteArray	str1;
    QString	str;
    QPoint	p, p1, p2;
    QRect	r;
    QPointArray a;
    QColor	color;
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QRegion	rgn;
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix	matrix;
#endif

    while ( nrecords-- && !s.eof() ) {
	s >> c;					// read cmd
	s >> tiny_len;				// read param length
	if ( tiny_len == 255 )			// longer than 254 bytes
	    s >> len;
	else
	    len = tiny_len;
#if defined(QT_DEBUG)
	strm_pos = s.device()->at();
#endif
	switch ( c ) {				// exec cmd
	    case PdcNOP:
		break;
	    case PdcDrawPoint:
		s >> p;
		painter->drawPoint( p );
		break;
	    case PdcDrawLine:
		s >> p1 >> p2;
		painter->drawLine( p1, p2 );
		break;
	    case PdcDrawRect:
		s >> r;
		painter->drawRect( r );
		break;
	    case PdcDrawRoundRect:
		s >> r >> i1_16 >> i2_16;
		painter->drawRoundRect( r, i1_16, i2_16 );
		break;
	    case PdcDrawEllipse:
		s >> r;
		painter->drawEllipse( r );
		break;
	    case PdcDrawArc:
		s >> r >> i1_16 >> i2_16;
		painter->drawArc( r, i1_16, i2_16 );
		break;
	    case PdcDrawPie:
		s >> r >> i1_16 >> i2_16;
		painter->drawPie( r, i1_16, i2_16 );
		break;
	    case PdcDrawChord:
		s >> r >> i1_16 >> i2_16;
		painter->drawChord( r, i1_16, i2_16 );
		break;
	    case PdcDrawLineSegments:
		s >> a;
		painter->drawLineSegments( a );
		break;
	    case PdcDrawPolyline:
		s >> a;
		painter->drawPolyline( a );
		break;
	    case PdcDrawPolygon:
		s >> a >> i_8;
		painter->drawPolygon( a, i_8 );
		break;
	    case PdcDrawCubicBezier:
		s >> a;
#ifndef QT_NO_BEZIER
		painter->drawCubicBezier( a );
#endif
		break;
	    case PdcDrawText:
		s >> p >> str1;
		painter->drawText( p, str1 );
		break;
	    case PdcDrawTextFormatted:
		s >> r >> i_16 >> str1;
		painter->drawText( r, i_16, str1 );
		break;
	    case PdcDrawText2:
		s >> p >> str;
		painter->drawText( p, str );
		break;
	    case PdcDrawText2Formatted:
		s >> r >> i_16 >> str;
		painter->drawText( r, i_16, str );
		break;
	    case PdcDrawPixmap: {
		QPixmap pixmap;
		if ( d->formatMajor < 4 ) {
		    s >> p >> pixmap;
		    painter->drawPixmap( p, pixmap );
		} else {
		    s >> r >> pixmap;
		    painter->drawPixmap( r, pixmap );
		}
	                }
		break;
	    case PdcDrawImage: {
		QImage image;
		if ( d->formatMajor < 4 ) {
		    s >> p >> image;
		    painter->drawImage( p, image );
		} else {
		    s >> r >> image;
		    painter->drawImage( r, image );
		}
		}
		break;
	    case PdcBegin:
		s >> ul;			// number of records
		if ( !exec( painter, s, ul ) )
		    return FALSE;
		break;
	    case PdcEnd:
		if ( nrecords == 0 )
		    return TRUE;
		break;
	    case PdcSave:
		painter->save();
		break;
	    case PdcRestore:
		painter->restore();
		break;
	    case PdcSetBkColor:
		s >> color;
		painter->setBackgroundColor( color );
		break;
	    case PdcSetBkMode:
		s >> i_8;
		painter->setBackgroundMode( (Qt::BGMode)i_8 );
		break;
	    case PdcSetROP:
		s >> i_8;
		painter->setRasterOp( (Qt::RasterOp)i_8 );
		break;
	    case PdcSetBrushOrigin:
		s >> p;
		painter->setBrushOrigin( p );
		break;
	    case PdcSetFont:
		s >> font;
		painter->setFont( font );
		break;
	    case PdcSetPen:
		s >> pen;
		painter->setPen( pen );
		break;
	    case PdcSetBrush:
		s >> brush;
		painter->setBrush( brush );
		break;
// #ifdef Q_Q3PAINTER
// 	case PdcSetTabStops:
// 		s >> i_16;
// 		painter->setTabStops( i_16 );
// 		break;
// 	    case PdcSetTabArray:
// 		s >> i_16;
// 		if ( i_16 == 0 ) {
// 		    painter->setTabArray( 0 );
// 		} else {
// 		    int *ta = new int[i_16];
// 		    for ( int i=0; i<i_16; i++ ) {
// 			s >> i1_16;
// 			ta[i] = i1_16;
// 		    }
// 		    painter->setTabArray( ta );
// 		    delete [] ta;
// 		}
// 		break;
// #endif
	    case PdcSetVXform:
		s >> i_8;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setViewXForm( i_8 );
#endif
		break;
	    case PdcSetWindow:
		s >> r;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setWindow( r );
#endif
		break;
	    case PdcSetViewport:
		s >> r;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setViewport( r );
#endif
		break;
	    case PdcSetWXform:
		s >> i_8;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setWorldXForm( i_8 );
#endif
		break;
	    case PdcSetWMatrix:
#ifndef QT_NO_TRANSFORMATIONS	// #### fix me!
		s >> matrix >> i_8;
		painter->setWorldMatrix( matrix, i_8 );
#endif
		break;
#ifndef QT_NO_TRANSFORMATIONS
// #ifdef Q_Q3PAINTER
// 	    case PdcSaveWMatrix:
// 		painter->saveWorldMatrix();
// 		break;
// 	    case PdcRestoreWMatrix:
// 		painter->restoreWorldMatrix();
// 		break;
// #endif
#endif
	    case PdcSetClip:
		s >> i_8;
		painter->setClipping( i_8 );
		break;
	    case PdcSetClipRegion:
		s >> rgn >> i_8;
		painter->setClipRegion( rgn, (QPainter::CoordinateMode)i_8 );
		break;
	    default:
		qWarning( "QPicture::play: Invalid command %d", c );
		if ( len )			// skip unknown command
		    s.device()->at( s.device()->at()+len );
	}
#if defined(QT_DEBUG)
	//qDebug( "device->at(): %i, strm_pos: %i len: %i", s.device()->at(), strm_pos, len );
	Q_ASSERT( Q_INT32(s.device()->at() - strm_pos) == len );
#endif
    }
    return false;
}
