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
    QGlyphInfo base = f->boundingBox( shaped->d->glyphs[gfrom] );
    QRect baseRect( base.x, base.y, base.width, base.height );

    qDebug( "base char: bounding rect at %d/%d (%d/%d)", baseRect.x(), baseRect.y(), baseRect.width(), baseRect.height() );
    int offset = f->ascent() / 10 + 1;
    qDebug("offset = %d", offset );

    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    for( i = 0; i < nmarks; i++ ) {
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
	qDebug( "char=%x combiningClass = %d offset=%d/%d", mark, cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	p += QPoint( -base.xoff + base.x, -base.yoff + base.y );
	p -= QPoint( -markInfo.xoff + markInfo.x, -markInfo.yoff + markInfo.y );
	attachmentRect |= markRect;
	lastCmb = cmb;
	shaped->d->offsets[gfrom+i].x = p.x();
	shaped->d->offsets[gfrom+i].y = p.y();
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
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
// and no reordering (except for reversing if (bidiLevel % 2 ) )
// also computes logClusters heuristically
void ScriptEngine::heuristicSetGlyphAttributes( ShapedItem *shaped )
{
    // ### zeroWidth and justification are missing here!!!!!
    ShapedItemPrivate *d = shaped->d;

    if ( d->length != d->num_glyphs )
	qWarning("ScriptEngine::heuristicSetGlyphAttributes: char length and num glyphs disagree" );

    d->glyphAttributes = (GlyphAttributes *)realloc( d->glyphAttributes, d->num_glyphs * sizeof( GlyphAttributes ) );

    d->logClusters = (unsigned short *) realloc( d->logClusters, d->num_glyphs * sizeof( unsigned short ) );
    for ( int i = 0; i < d->num_glyphs; i++ )
	d->logClusters[i] = i;

    // honour the logClusters array if it exists.
    const QChar *uc = d->string.unicode() + d->from;
    if ( d->analysis.bidiLevel % 2 ) {
	// reversed
	int gpos = d->length - 2;
	int cpos = 1;

	// first char in a run is never (treated as) a mark
	int cStart = 0;
	d->glyphAttributes[0].mark = FALSE;
	d->glyphAttributes[0].clusterStart = TRUE;

	bool hasMark = FALSE;
	while ( gpos >= 0 ) {
	    if ( isMark( uc[cpos] ) ) {
		d->glyphAttributes[gpos].mark = TRUE;
		d->glyphAttributes[gpos].combiningClass = combiningClass( uc[cpos] );
		d->glyphAttributes[gpos].clusterStart = FALSE;
		d->logClusters[cpos] = cpos;
		hasMark = TRUE;
	    } else {
		d->glyphAttributes[gpos].mark = FALSE;
		d->glyphAttributes[gpos].clusterStart = TRUE;
		cStart = gpos;
	    }
	    cpos++;
	    gpos--;
	}


	if ( hasMark ) {
	    // reorder marks to follow the base char. Also brings the clusterstar flag into the correct position
	    int gpos = 0;
	    int clusterstart = -1;
	    while ( gpos < d->length ) {
		// since the order is still reversed, FALSE indicates the first glyph in a cluster, the next true will be the last one
		if ( clusterstart == -1 && !d->glyphAttributes[gpos].clusterStart ) {
		    clusterstart = gpos;
		} else if ( clusterstart != -1 && d->glyphAttributes[gpos].clusterStart ) {
		    // reverse from clusterstart to gpos
		    {
			GlyphIndex *cp = d->glyphs + clusterstart;
			GlyphIndex *ch = d->glyphs + cpos;
			while ( ch > cp ) {
			    GlyphIndex tmpGlyph = *cp;
			    *cp = *ch;
			    *ch = tmpGlyph;
			    cp++;
			    ch--;
			}
		    }
		    GlyphAttributes *cp = d->glyphAttributes + clusterstart;
		    GlyphAttributes *ch = d->glyphAttributes + cpos;
		    while ( ch > cp ) {
			GlyphAttributes tmp = *cp;
			*cp = *ch;
			*ch = tmp;
			cp++;
			ch--;
		    }
		}
		gpos++;
	    }

	}
    } else {
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
		d->logClusters[pos] = cStart;
	    } else {
		d->glyphAttributes[pos].mark = FALSE;
		d->glyphAttributes[pos].clusterStart = TRUE;
		cStart = pos;
	    }
	    pos++;
	}
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
    bool reverse = d->analysis.bidiLevel % 2;
    int error = d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs, reverse );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs, reverse );
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

}

int ScriptEngineBasic::xToCursor( int x, const ShapedItem &shaped )
{

}
