#include "scriptengine.h"
#include <stdlib.h>

#include <qstring.h>
#include <qrect.h>
#include <private/qunicodetables_p.h>
#include "qtextengine.h"

static inline void positionCluster( QScriptItem *item, int gfrom,  int glast )
{
    int nmarks = glast - gfrom;
    if ( nmarks <= 0 ) {
	qWarning( "positionCluster: no marks to position!" );
	return;
    }

    QShapedItem *shaped = item->shaped;
    QFontEngine *f = item->fontEngine;
    QGlyphMetrics baseInfo = f->boundingBox( shaped->glyphs[gfrom] );
    QRect baseRect( baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height );

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast );
//     qDebug( "baseInfo: %d/%d (%d/%d) off=%d/%d", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff );

    int size = f->ascent()/10;
    int offsetBase = (size - 4) / 4 + QMIN( size, 4 ) + 1;
//     qDebug("offset = %d", offsetBase );

    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;

    for( i = 1; i <= nmarks; i++ ) {
	glyph_t mark = shaped->glyphs[gfrom+i];
	QPoint p;
	QGlyphMetrics markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x, markInfo.y, markInfo.width, markInfo.height );

	int offset = offsetBase;
	unsigned char cmb = shaped->glyphAttributes[gfrom+i].combiningClass;

	// ### maybe the whole position determination should move down to heuristicSetGlyphAttributes. Would save some
	// bits  in the glyphAttributes structure.
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew, lao and thai.

	    // for Lao and Thai marks with class 0, see below ( heuristicSetGlyphAttributes )

	    // add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 && offset < 3 )
		offset +=1;
 	    // below
	    if ( (cmb >= 10 && cmb <= 18) ||
		 cmb == 20 || cmb == 22 ||
		 cmb == 29 || cmb == 32 )
		cmb = QChar::Combining_Below;
	    // above
	    else if ( cmb == 23 || cmb == 27 || cmb == 28 ||
		      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36 ) )
		cmb = QChar::Combining_Above;
	    //below-right
	    else if ( cmb == 9 || cmb == 103 || cmb == 118 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 || cmb == 122 )
		cmb = QChar::Combining_AboveRight;
	    else if ( cmb == 25 )
		cmb = QChar::Combining_AboveLeft;
	    // fixed:
	    //  19 21

	}

	// combining marks of different class don't interact. Reset the rectangle.
	if ( cmb != lastCmb ) {
	    //qDebug( "resetting rect" );
	    attachmentRect = baseRect;
	}

	switch( cmb ) {
	case QChar::Combining_DoubleBelow:
		// ### wrong in rtl context!
	case QChar::Combining_BelowLeft:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowLeftAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    break;
	case QChar::Combining_Below:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_BelowRight:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowRightAttached:
	    p += attachmentRect.bottomRight() - markRect.topRight();
	    break;
	    case QChar::Combining_Left:
	    p += QPoint( -offset, 0 );
	case QChar::Combining_LeftAttached:
	    break;
	    case QChar::Combining_Right:
	    p += QPoint( offset, 0 );
	case QChar::Combining_RightAttached:
	    break;
	case QChar::Combining_DoubleAbove:
	    // ### wrong in RTL context!
	case QChar::Combining_AboveLeft:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveLeftAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    break;
	    case QChar::Combining_Above:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_AboveRight:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveRightAttached:
	    p += attachmentRect.topRight() - markRect.bottomRight();
	    break;

	case QChar::Combining_IotaSubscript:
	    default:
		break;
	}
// 	qDebug( "char=%x combiningClass = %d offset=%d/%d", mark, cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	attachmentRect |= markRect;
	lastCmb = cmb;
	shaped->offsets[gfrom+i].x = p.x() - baseInfo.xoff;
	shaped->offsets[gfrom+i].y = p.y() - baseInfo.yoff;
	shaped->advances[gfrom+i].x = 0;
	shaped->advances[gfrom+i].y = 0;
    }
}


void QScriptEngine::heuristicPosition( QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;;

    int cEnd = -1;
    int i = shaped->num_glyphs;
    while ( i-- ) {
	if ( cEnd == -1 && shaped->glyphAttributes[i].mark ) {
	    cEnd = i;
	} else if ( cEnd != -1 && !shaped->glyphAttributes[i].mark ) {
	    positionCluster( item, i, cEnd );
	    cEnd = -1;
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
// and no reordering.
// also computes logClusters heuristically
void QScriptEngine::heuristicSetGlyphAttributes( const QString &string, int from, int len, QScriptItem *item )
{
    // ### zeroWidth and justification are missing here!!!!!
    QShapedItem *shaped = item->shaped;

    if ( shaped->num_glyphs != len )
	qWarning("QScriptEngine::heuristicSetGlyphAttributes: char length and num glyphs disagree" );

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", shaped->num_glyphs);

    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, shaped->num_glyphs * sizeof( GlyphAttributes ) );

    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, shaped->num_glyphs * sizeof( unsigned short ) );

    // honour the logClusters array if it exists.
    const QChar *uc = string.unicode() + from;

    for ( int i = 0; i < shaped->num_glyphs; i++ )
	shaped->logClusters[i] = i;

    int pos = 1;

    // first char in a run is never (treated as) a mark
    int cStart = 0;
    shaped->glyphAttributes[0].mark = FALSE;
    shaped->glyphAttributes[0].clusterStart = TRUE;

    while ( pos < len ) {
	if ( ::category( uc[pos] ) == QChar::Mark_NonSpacing ) {
	    shaped->glyphAttributes[pos].mark = TRUE;
	    shaped->glyphAttributes[pos].clusterStart = FALSE;
	    int cmb = combiningClass( uc[pos] );

	    if ( cmb == 0 ) {
		// Fix 0 combining classes
		if ( uc[pos].row() == 0x0e ) {
		    // thai or lao
		    unsigned char col = uc[pos].cell();
		    if ( col == 0x31 ||
			 col == 0x34 ||
			 col == 0x35 ||
			 col == 0x36 ||
			 col == 0x37 ||
			 col == 0x47 ||
			 col == 0x4c ||
			 col == 0x4d ||
			 col == 0x4e ) {
			cmb = QChar::Combining_AboveRight;
		    } else if ( col == 0xb1 ||
				col == 0xb4 ||
				col == 0xb5 ||
				col == 0xb6 ||
				col == 0xb7 ||
				col == 0xbb ||
				col == 0xcc ||
				col == 0xcd ) {
			cmb = QChar::Combining_Above;
		    } else if ( col == 0xbc ) {
			cmb = QChar::Combining_Below;
		    }
		}
	    }

	    shaped->glyphAttributes[pos].combiningClass = cmb;
	    // 		qDebug("found a mark at position %d", pos );
	    shaped->logClusters[pos] = cStart;
	} else {
	    shaped->glyphAttributes[pos].mark = FALSE;
	    shaped->glyphAttributes[pos].clusterStart = TRUE;
	    shaped->glyphAttributes[pos].combiningClass = 0;
	    cStart = pos;
	}
	pos++;
    }
}

void QScriptEngine::charAttributes( const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	// ### remove nbsp?
	attributes[i].whiteSpace = ::isSpace( uc[i] );
	attributes[i].softBreak = attributes[i].whiteSpace;
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = attributes[i].whiteSpace;
    }
}


void QScriptEngine::shape( const QString &string, int from, int len, QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;
    shaped->num_glyphs = len;
    shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
    int error = item->fontEngine->stringToCMap( string.unicode() + from, len, shaped->glyphs, &shaped->num_glyphs );
    if ( error == QFontEngine::OutOfMemory ) {
	shaped->glyphs = (glyph_t *)realloc( shaped->glyphs, shaped->num_glyphs*sizeof( glyph_t ) );
	item->fontEngine->stringToCMap( string.unicode() + from, len, shaped->glyphs, &shaped->num_glyphs );
    }

    heuristicSetGlyphAttributes( string, from, len, item );
    calculateAdvances( item );
    heuristicPosition( item );
}


void QScriptEngine::calculateAdvances( QScriptItem *item )
{
    QShapedItem *shaped = item->shaped;
    shaped->offsets = (offset_t *) realloc( shaped->offsets, shaped->num_glyphs * sizeof( offset_t ) );
    memset( shaped->offsets, 0, shaped->num_glyphs * sizeof( offset_t ) );
    shaped->advances = (offset_t *) realloc( shaped->advances, shaped->num_glyphs * sizeof( offset_t ) );
    item->ascent = item->fontEngine->ascent();
    item->descent = item->fontEngine->descent();
    item->width = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	QGlyphMetrics gi = item->fontEngine->boundingBox( shaped->glyphs[i] );
	shaped->advances[i].x = gi.xoff;
	shaped->advances[i].y = gi.yoff;
// 	qDebug("setting advance of glyph %d to %d", i, gi.xoff );
	int y = shaped->offsets[i].y + gi.y;
	item->ascent = QMAX( item->ascent, -y );
	item->descent = QMAX( item->descent, y + gi.height );
	item->width += gi.xoff;
    }
}
