#include "scriptenginebasic.h"
#include <stdlib.h>

#include <qstring.h>
#include "qtextdata.h"

static inline void positionCluster( ShapedItem *shaped, int gfrom,  int glast )
{
    int nmarks = glast - gfrom;
    if ( nmarks <= 0 ) {
	qWarning( "positionCluster: no marks to position!" );
	return;
    }

    FontEngineIface *f = shaped->d->fontEngine;
    QGlyphInfo baseInfo = f->boundingBox( shaped->d->glyphs[gfrom] );
    QRect baseRect( baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height );

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast );
//     qDebug( "baseInfo: %d/%d (%d/%d) off=%d/%d", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff );

    int offset = f->ascent() / 10 + 1;
//     qDebug("offset = %d", offset );

    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    QPoint penpos = QPoint( baseInfo.xoff, baseInfo.yoff );

    for( i = 1; i <= nmarks; i++ ) {
	GlyphIndex mark = shaped->d->glyphs[gfrom+i];
	unsigned char cmb = shaped->d->glyphAttributes[gfrom+i].combiningClass;
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew and thai.

	    // ### add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 )
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
	    else if ( cmb == 103 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 )
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

	QPoint p;
	QGlyphInfo markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x, markInfo.y, markInfo.width, markInfo.height );
// 	qDebug( "markRect: %d/%d (%d/%d) offset=%d/%d", markRect.x(), markRect.y(), markRect.width(), markRect.height(), markInfo.xoff, markInfo.yoff );
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
	shaped->d->offsets[gfrom+i].x = -penpos.x() + p.x();
	shaped->d->offsets[gfrom+i].y = -penpos.y() + p.y();
	penpos += QPoint( shaped->d->offsets[gfrom+i].x, shaped->d->offsets[gfrom+i].y );
	penpos += QPoint( markInfo.xoff, markInfo.yoff );
// 	qDebug("positionCluster penpos=%d/%d", penpos.x(), penpos.y() );
    }
    if ( glast < shaped->d->num_glyphs-1 ) {
// 	qDebug("positionCluster:end penpos=%d/%d", penpos.x(), penpos.y() );
	shaped->d->offsets[glast+1].x = -penpos.x() + baseInfo.xoff;
	shaped->d->offsets[glast+1].y = -penpos.y() + baseInfo.yoff;
    }
}


void ScriptEngine::heuristicPositionMarks( ShapedItem *shaped )
{
    ShapedItemPrivate *d = shaped->d;

    int cEnd = -1;
    int i = d->num_glyphs;
    while ( i-- ) {
	if ( cEnd == -1 && d->glyphAttributes[i].mark ) {
	    cEnd = i;
	} else if ( cEnd != -1 && !d->glyphAttributes[i].mark ) {
	    positionCluster( shaped, i, cEnd );
	    cEnd = -1;
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
// and no reordering.
// also computes logClusters heuristically
void ScriptEngine::heuristicSetGlyphAttributes( ShapedItem *shaped )
{
    // ### zeroWidth and justification are missing here!!!!!
    ShapedItemPrivate *d = shaped->d;

    if ( d->length != d->num_glyphs )
	qWarning("ScriptEngine::heuristicSetGlyphAttributes: char length and num glyphs disagree" );

    qDebug("ScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", d->num_glyphs);

    d->glyphAttributes = (GlyphAttributes *)realloc( d->glyphAttributes, d->num_glyphs * sizeof( GlyphAttributes ) );

    d->logClusters = (unsigned short *) realloc( d->logClusters, d->num_glyphs * sizeof( unsigned short ) );

    // honour the logClusters array if it exists.
    const QChar *uc = d->string.unicode() + d->from;

    for ( int i = 0; i < d->num_glyphs; i++ )
	d->logClusters[i] = i;

    int pos = 1;

    // first char in a run is never (treated as) a mark
    int cStart = 0;
    d->glyphAttributes[0].mark = FALSE;
    d->glyphAttributes[0].clusterStart = TRUE;

    while ( pos < d->length ) {
	if ( isMark( uc[pos] ) ) {
	    d->glyphAttributes[pos].mark = TRUE;
	    d->glyphAttributes[pos].clusterStart = FALSE;
	    d->glyphAttributes[pos].combiningClass = combiningClass( uc[pos] );
	    // 		qDebug("found a mark at position %d", pos );
	    d->logClusters[pos] = cStart;
	} else {
	    d->glyphAttributes[pos].mark = FALSE;
	    d->glyphAttributes[pos].clusterStart = TRUE;
	    cStart = pos;
	}
	pos++;
    }
}

void ScriptEngineBasic::charAttributes( const QString &text, int from, int len, CharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	attributes[i].softBreak = FALSE;
	// ### remove nbsp?
	attributes[i].whiteSpace = uc[i].isSpace();
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = attributes[i].whiteSpace;
    }
}


void ScriptEngineBasic::shape( ShapedItem *result )
{
    const QString &text = result->d->string;
    int from = result->d->from;
    int len = result->d->length;

    ShapedItemPrivate *d = result->d;
    d->num_glyphs = len;
    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs );
    }

    heuristicSetGlyphAttributes( result );

    d->offsets = (Offset *) malloc( d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );

    // we have a simple 1 to 1 mapping from chars to glyphs, so we leave logClusters initialized to 0
}


void ScriptEngineBasic::position( ShapedItem *shaped )
{
    heuristicPositionMarks( shaped );
}

int ScriptEngineBasic::cursorToX( int cPos, const ShapedItem &shaped )
{
    Q_UNUSED(cPos);
    Q_UNUSED(shaped);
    return 0;
}

int ScriptEngineBasic::xToCursor( int x, const ShapedItem &shaped )
{
    Q_UNUSED(x);
    Q_UNUSED(shaped);
    return 0;
}
