/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture.cpp#1 $
**
** Implementation of QMetaFile class
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qmetafil.h"
#include "qpainter.h"
#include "qpntarry.h"
#include "qfile.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpicture.cpp#1 $";
#endif


static const UINT32 mfhdr_tag = 0x11140d06;	// header tag
static const int    mfhdr_maj = 1;		// major version #
static const int    mfhdr_min = 0;		// minor version #


bool QMetaFile::load( const char *fileName )	// read from file
{
    QByteArray a;
    QFile f( fileName );
    if ( !f.open( Stream_ReadOnly ) )
	return FALSE;
    a.resize( (uint)f.size() );
    f.readBytes( a.data(), (uint)f.size() );
    f.close();
    s.setBuffer( a );
    return TRUE;
}

bool QMetaFile::save( const char *fileName )	// write to file
{
    QFile f( fileName );
    if ( !f.open( Stream_WriteOnly ) )
	return FALSE;
    f.writeBytes( s.buffer().data(), s.buffer().size() );
    f.close();
    return TRUE;
}


bool QMetaFile::play( QPainter *painter )
{
    if ( s.size() == 0 )			// nothing recorded
	return FALSE;
    s.open( Stream_ReadOnly );			// open stream
    QByteArray buf = s.buffer();
    int c, len, i1, i2;				// parameters
    ulong ul;
    char *str;
    QPoint p, p2;
    QRect r;
    QPointArray a;
    QColor color;
    ulong mf_id;				// metafile id (tag)
    int major, minor;
    s >> mf_id;					// read tag
    if ( mf_id != mfhdr_tag ) {			// wrong header id
#if defined(CHECK_RANGE)
	warning( "QMetaFile::play: Incorrect header" );
#endif
	s.close();
	return FALSE;
    }
    int cs_start = sizeof(UINT32);		// pos of checksum word
    int data_start = cs_start + sizeof(UINT16);
    UINT16 cs;
    s >> cs;					// read checksum
    if ( qchecksum( buf.data() + data_start, buf.size() - data_start) != cs ) {
#if defined(CHECK_STATE)
	warning( "QMetaFile::play: Invalid checksum" );
#endif
	s.close();
	return FALSE;
    }
    s >> major >> minor;			// read version number
    if ( major > mfhdr_maj ) {			// new, incompatible version
#if defined(CHECK_RANGE)
	warning( "QMetaFile::play: Incompatible version %d.%d",
		 major, minor);
#endif
	s.close();
	return FALSE;
    }
    while ( !s.atEnd() ) {
	s >> c;					// read cmd
	s >> len;				// read param length
	switch ( c ) {				// exec cmd
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
		s >> p >> p2;
		painter->drawLine( p, p2 );
		break;
	    case PDC_DRAWRECT:
		s >> r;
		painter->drawRect( r );
		break;
	    case PDC_DRAWROUNDRECT:
		s >> r >> i1 >> i2;
		painter->drawRoundRect( r, i1, i2 );
		break;
	    case PDC_DRAWELLIPSE:
		s >> r;
		painter->drawEllipse( r );
		break;
	    case PDC_DRAWARC:
		s >> r >> i1 >> i2;
		painter->drawArc( r, i1, i2 );
		break;
	    case PDC_DRAWPIE:
		s >> r >> i1 >> i2;
		painter->drawPie( r, i1, i2 );
		break;
	    case PDC_DRAWCHORD:
		s >> r >> i1 >> i2;
		painter->drawChord( r, i1, i2 );
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
		s >> a >> i1;
		painter->drawPolygon( a, i1 );
		break;
	    case PDC_DRAWTEXT:
		s >> p >> str;
		painter->drawText( p, str );
		delete str;
		break;
	    case PDC_SETBKCOLOR:
		s >> ul;
		color.setRGB( ul );
		painter->setBackgroundColor( color );
		break;
	    case PDC_SETBKMODE:
		s >> i1;
		painter->setBackgroundMode( (BGMode)i1 );
		break;
	    case PDC_SETROP:
		s >> i1;
		painter->setRasterOp( (RasterOp)i1 );
		break;
	    case PDC_SETPEN:
		s >> i1 >> i2 >> ul;
		color.setRGB( ul );
		painter->setPen( QPen( color, i2, (PenStyle)i1 ) );
		break;
	    case PDC_SETBRUSH:
		s >> i1 >> ul;
		color.setRGB( ul );
		painter->setBrush( QBrush( color, (BrushStyle)i1 ) );
		break;
	    case PDC_SETXFORM:
		s >> i1;
		painter->setXForm( i1 );
		break;
	    case PDC_SETSOURCEVIEW:
		s >> r;
		painter->setSourceView( r );
		break;
	    case PDC_SETTARGETVIEW:
		s >> r;
		painter->setTargetView( r );
		break;
	    case PDC_END:
		s.close();
		return TRUE;
	    default:
		if ( len )			// skip unknown command
		    s.at( s.at()+len );
	}
    }
    return FALSE;				// no end-command
}
