/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture.cpp#10 $
**
** Implementation of QMetaFile class
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmetafil.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qwxfmat.h"
#include "qfile.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpicture.cpp#10 $";
#endif


static const UINT32 mfhdr_tag = 0x11140d06;	// header tag
static const UINT16 mfhdr_maj = 1;		// major version #
static const UINT16 mfhdr_min = 0;		// minor version #


bool QMetaFile::load( const char *fileName )	// read from file
{
    QByteArray a;
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;
    a.resize( (uint)f.size() );
    f.readBlock( a.data(), (uint)f.size() );	// read file into byte array
    f.close();
    mfbuf.setBuffer( a );			// set byte array in buffer
    return TRUE;
}

bool QMetaFile::save( const char *fileName )	// write to file
{
    QFile f( fileName );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    f.writeBlock( mfbuf.buffer().data(), mfbuf.buffer().size() );
    f.close();
    return TRUE;
}


bool QMetaFile::play( QPainter *painter )
{
    if ( mfbuf.size() == 0 )			// nothing recorded
	return FALSE;

    mfbuf.open( IO_ReadOnly );			// open buffer device
    QDataStream s;
    s.setDevice( &mfbuf );			// attach data stream to buffer

    UINT32 mf_id;				// metafile id (tag)
    s >> mf_id;					// read tag
    if ( mf_id != mfhdr_tag ) {			// wrong header id
#if defined(CHECK_RANGE)
	warning( "QMetaFile::play: Incorrect header" );
#endif
	mfbuf.close();
	return FALSE;
    }

    int cs_start = sizeof(UINT32);		// pos of checksum word
    int data_start = cs_start + sizeof(UINT16);
    UINT16 cs,ccs;
    QByteArray buf = mfbuf.buffer();		// pointer to data
    s >> cs;					// read checksum
    ccs = qchecksum( buf.data() + data_start, buf.size() - data_start );
    if ( ccs != cs ) {
#if defined(CHECK_STATE)
	warning( "QMetaFile::play: Invalid checksum %x, %x expected",
		 ccs, cs );
#endif
#if !defined(DEBUG)
	mfbuf.close();		// NOTE!!! PASS THROUGH
	return FALSE;
#endif
    }

    UINT16 major, minor;
    s >> major >> minor;			// read version number
    if ( major > mfhdr_maj ) {			// new, incompatible version
#if defined(CHECK_RANGE)
	warning( "QMetaFile::play: Incompatible version %d.%d",
		 major, minor);
#endif
	mfbuf.close();
	return FALSE;
    }
    UINT8  c, clen;
    UINT32 nrecords;
    s >> c >> clen;
    if ( c == PDC_BEGIN ) {
	s >> nrecords;
	if ( !exec( painter, s, nrecords ) )
	    c = 0;	
    }
    if ( c !=  PDC_BEGIN ) {
#if defined(CHECK_RANGE)
	warning( "QMetaFile::play: Format error" );
#endif
	mfbuf.close();
	return FALSE;
    }    
    mfbuf.close();
    return TRUE;				// no end-command
}


bool QMetaFile::exec( QPainter *painter, QDataStream &s, long nrecords )
{
    UINT8  c;					// command id
    UINT8  tiny_len;				// 8-bit length descriptor
    INT32  len;					// 32-bit length descriptor
    INT16  i1_16, i2_16;			// parameters...
    INT8   i_8, i1_8, i2_8;
    UINT32 ul;
    long   strm_pos;
    char  *str;
    QPoint p, p1, p2;
    QRect  r;
    QPointArray a;
    QColor color;
    QFont  font;
    QPen   pen;
    QBrush brush;
    QRegion rgn;
    QWXFMatrix matrix;

    while ( nrecords-- && !s.eos() ) {
	s >> c;					// read cmd
	s >> tiny_len;				// read param length
	if ( tiny_len == 255 )			// longer than 254 bytes
	    s >> len;
	else
	    len = tiny_len;
#if defined(DEBUG)
	strm_pos = s.device()->at();
#endif
	switch ( c ) {				// exec cmd
	    case PDC_NOP:
		break;
	    case PDC_DRAWPOINT:
		s >> p;
		painter->drawPoint( p );
		break;
	    case PDC_MOVETO:
		s >> p;
		painter->moveTo( p );
		break;
	    case PDC_LINETO:
		s >> p;
		painter->lineTo( p );
		break;
	    case PDC_DRAWLINE:
		s >> p1 >> p2;
		painter->drawLine( p1, p2 );
		break;
	    case PDC_DRAWRECT:
		s >> r;
		painter->drawRect( r );
		break;
	    case PDC_DRAWROUNDRECT:
		s >> r >> i1_16 >> i2_16;
		painter->drawRoundRect( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWELLIPSE:
		s >> r;
		painter->drawEllipse( r );
		break;
	    case PDC_DRAWARC:
		s >> r >> i1_16 >> i2_16;
		painter->drawArc( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWPIE:
		s >> r >> i1_16 >> i2_16;
		painter->drawPie( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWCHORD:
		s >> r >> i1_16 >> i2_16;
		painter->drawChord( r, i1_16, i2_16 );
		break;
	    case PDC_DRAWLINESEGS:
		s >> a;
		painter->drawLineSegments( a );
		break;
	    case PDC_DRAWPOLYLINE:
		s >> a;
		painter->drawPolyline( a );
		break;
	    case PDC_DRAWPOLYGON:
		s >> a >> i_8;
		painter->drawPolygon( a, i_8 );
		break;
	    case PDC_DRAWTEXT:
		s >> p >> str;
		painter->drawText( p, str );
		delete str;
		break;
	    case PDC_DRAWTEXTALIGN:
	        debug( "QMetaFile: DRAWTEXTALIGN not implemented" );
	        break;
	    case PDC_DRAWPIXMAP:
	        debug( "QMetaFile: DRAWPIXMAP not implemented" );
	        break;
	    case PDC_BEGIN:
		s >> ul;			// number of records
	        if ( !exec( painter, s, ul ) )
		    return FALSE;
	        break;
	    case PDC_END:
		if ( nrecords == 0 )
		    return TRUE;
		break;
	    case PDC_SAVE:
		painter->save();
		break;
	    case PDC_RESTORE:
		painter->restore();
		break;
	    case PDC_SETBKCOLOR:
		s >> color;
		painter->setBackgroundColor( color );
		break;
	    case PDC_SETBKMODE:
		s >> i_8;
		painter->setBackgroundMode( (BGMode)i_8 );
		break;
	    case PDC_SETROP:
		s >> i_8;
		painter->setRasterOp( (RasterOp)i_8 );
		break;
	    case PDC_SETBRUSHORIGIN:
		s >> p;
		painter->setBrushOrigin( p );
		break;
	    case PDC_SETFONT:
	        s >> font;
	        painter->setFont( font );
		break;
	    case PDC_SETPEN:
		s >> pen;
		painter->setPen( pen );
		break;
	    case PDC_SETBRUSH:
		s >> brush;
		painter->setBrush( brush );
		break;
	    case PDC_SETVXFORM:
		s >> i_8;
		painter->setViewXForm( i_8 );
		break;
	    case PDC_SETSOURCEVIEW:
		s >> r;
		painter->setSourceView( r );
		break;
	    case PDC_SETTARGETVIEW:
		s >> r;
		painter->setTargetView( r );
		break;
	    case PDC_SETWXFORM:
	        s >> i_8;
	        painter->setWorldXForm( i_8 );
	        break;
	    case PDC_SETWXFMATRIX:
	        s >> matrix >> i_8;
	        painter->setWxfMatrix( matrix, i_8 );
	        break;
	    case PDC_SETCLIP:
	        s >> i_8;
	        painter->setClipping( i_8 );
	        break;
	    case PDC_SETCLIPRGN:
		s >> rgn;
		painter->setClipRegion( rgn );
		break;
	    default:
#if defined(CHECK_RANGE)
		warning( "QMetaFile::play: Invalid command %d", c );
#endif
		if ( len )			// skip unknown command
		    s.device()->at( s.device()->at()+len );
	}
#if defined(DEBUG)
	ASSERT( s.device()->at() - strm_pos == len );
#endif
    }
    return FALSE;
}


bool QMetaFile::cmd( int c, QPDevCmdParam *p )
{
    QDataStream s;
    s.setDevice( &mfbuf );
    if ( c ==  PDC_BEGIN ) {			// begin; write header
	QByteArray empty( 0 );
	mfbuf.setBuffer( empty );		// reset byte array in buffer
	mfbuf.open( IO_WriteOnly );
	s << mfhdr_tag << (UINT16)0 << mfhdr_maj << mfhdr_min;
	s << (UINT8)c << (UINT8)sizeof(INT32);
	trecs = 0;
	s << trecs;				// total number of records
	return TRUE;
    }
    else if ( c == PDC_END ) {			// end; calc checksum and close
	trecs++;
	s << (UINT8)c << (UINT8)0;
	QByteArray buf = mfbuf.buffer();
	int cs_start = sizeof(UINT32);		// pos of checksum word
	int data_start = cs_start + sizeof(UINT16);
	int nrecs_start = data_start + 2*sizeof(INT16) + 2*sizeof(UINT8);
	long pos = mfbuf.at();
	mfbuf.at( nrecs_start );
	s << trecs;				// write number of records
	mfbuf.at( cs_start );
	UINT16 cs = (UINT16)qchecksum( buf.data()+data_start, pos-data_start );
	s << cs;				// write checksum
	mfbuf.close();
	return TRUE;
    }
    trecs++;
    s << (UINT8)c;				// write cmd to stream
    s << (UINT8)0;				// write dummy length info
    int pos = (int)mfbuf.at();			// save position
    switch ( c ) {
	case PDC_DRAWPOINT:
	case PDC_MOVETO:
	case PDC_LINETO:
	case PDC_SETBRUSHORIGIN:
	    s << *p[0].point;
	    break;
	case PDC_DRAWLINE:
	    s << *p[0].point << *p[1].point;
	    break;
	case PDC_DRAWRECT:
	case PDC_DRAWELLIPSE:
	    s << *p[0].rect;
	    break;
	case PDC_DRAWROUNDRECT:
	case PDC_DRAWARC:
	case PDC_DRAWPIE:
	case PDC_DRAWCHORD:
	    s << *p[0].rect << (INT16)p[1].ival << (INT16)p[2].ival;
	    break;
	case PDC_DRAWLINESEGS:
	case PDC_DRAWPOLYLINE:
	    s << *p[0].ptarr;
	    break;
	case PDC_DRAWPOLYGON:
	    s << *p[0].ptarr << (INT8)p[1].ival;
	    break;
	case PDC_DRAWTEXT:
	    s << *p[0].point << p[1].str;
	    break;
	case PDC_DRAWTEXTALIGN:
	    debug( "QMetaFile::cmd: DRAWTEXTALIGN not implemented" );
	    break;
	case PDC_DRAWPIXMAP:
	    debug( "QMetaFile::cmd: DRAWPIXMAP not implemented" );
	    break;
	case PDC_SAVE:
	case PDC_RESTORE:
	    break;
	case PDC_SETBKCOLOR:
	    s << *p[0].color;
	    break;
	case PDC_SETBKMODE:
	case PDC_SETROP:
	    s << (INT8)p[0].ival;
	    break;
	case PDC_SETFONT:
	    s << *p[0].font;
	    break;
	case PDC_SETPEN:
	    s << *p[0].pen;
	    break;
	case PDC_SETBRUSH:
	    s << *p[0].brush;
	    break;
	case PDC_SETUNIT:
	case PDC_SETVXFORM:
	case PDC_SETWXFORM:
	case PDC_SETCLIP:
	    s << (INT8)p[0].ival;
	    break;
	case PDC_SETSOURCEVIEW:
	case PDC_SETTARGETVIEW:
	    s << *p[0].rect;
	    break;
	case PDC_SETWXFMATRIX:
	    s << *p[0].matrix << (INT8)p[1].ival;
	    break;
	case PDC_SETCLIPRGN:
	    s << *p[0].rgn;
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QMetaFile::cmd: Command %d not recognized", c );
#endif
    }
    int newpos = (int)mfbuf.at();		// new position
    int length = newpos - pos;
    mfbuf.at(pos - 1);				// set back and
    if ( length < 255 )				// write 8-bit length
	s << (UINT8)length;
    else					// write 32-bit length
	s << (UINT8)255 << (UINT32)length;
    mfbuf.at( newpos );				// set to new position
    return TRUE;
}


// --------------------------------------------------------------------------
// QPainter member functions
//

void QPainter::drawMetaFile( const QMetaFile &mf )
{
    ((QMetaFile*)&mf)->play( (QPainter*)this );
}
