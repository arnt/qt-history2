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
    QGlyphMetrics baseInfo = f->boundingBox( shaped->d->glyphs[gfrom] );
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
	GlyphIndex mark = shaped->d->glyphs[gfrom+i];
	unsigned char cmb = shaped->d->glyphAttributes[gfrom+i].combiningClass;
	int offset = offsetBase;
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew and thai.

	    // ### add a bit more offset to arabic, a bit hacky
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
	QGlyphMetrics markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x, markInfo.y, markInfo.width, markInfo.height );
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
	shaped->d->offsets[gfrom+i].x = p.x() - baseInfo.xoff;
	shaped->d->offsets[gfrom+i].y = p.y() - baseInfo.yoff;
	shaped->d->advances[gfrom+i].x = 0;
	shaped->d->advances[gfrom+i].y = 0;
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

//     qDebug("ScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", d->num_glyphs);

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
	// ### remove nbsp?
	attributes[i].whiteSpace = ::isSpace( uc[i] );
	attributes[i].softBreak = attributes[i].whiteSpace;
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
    d->isShaped = TRUE;
}


void ScriptEngineBasic::calculateAdvances( ShapedItem *shaped )
{
    ShapedItemPrivate *d = shaped->d;
    d->offsets = (Offset *) realloc( d->offsets, d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );
    d->advances = (Offset *) realloc( d->advances, d->num_glyphs * sizeof( Offset ) );
    d->ascent = d->fontEngine->ascent();
    d->descent = d->fontEngine->descent();
    for ( int i = 0; i < d->num_glyphs; i++ ) {
	QGlyphMetrics gi = d->fontEngine->boundingBox( d->glyphs[i] );
	d->advances[i].x = gi.xoff;
	d->advances[i].y = gi.yoff;
// 	qDebug("setting advance of glyph %d to %d", i, gi.xoff );
	int y = d->offsets[i].y + gi.y;
	d->ascent = QMAX( d->ascent, -y );
	d->descent = QMAX( d->descent, y + gi.height );
    }
}

void ScriptEngineBasic::position( ShapedItem *shaped )
{
    if ( shaped->d->isPositioned )
	return;

    calculateAdvances( shaped );
    heuristicPositionMarks( shaped );
    shaped->d->isPositioned = TRUE;
}
