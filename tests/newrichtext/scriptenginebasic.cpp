#include "scriptenginebasic.h"
#include <stdlib.h>

#include <qstring.h>

static void positionMarks( ShapedItem *item )
{
#if 0

    int len = str.length();
    int nmarks = 0;
    while ( pos + nmarks < len && combiningClass( str.unicode()[pos+nmarks +1] ) > 0 )
	nmarks++;

    if ( !nmarks )
	return QPointArray();

    QChar baseChar = QComplexText::shapedCharacter( str, pos );
    QRect baseRect = f->boundingRect( baseChar );
    int baseOffset = f->textWidth( str, pos, 1 );

    //qDebug( "base char: bounding rect at %d/%d (%d/%d)", baseRect.x(), baseRect.y(), baseRect.width(), baseRect.height() );
    int offset = f->actual.pixelSize / 10 + 1;
    //qDebug("offset = %d", offset );
    QPointArray pa( nmarks );
    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;
    if ( boundingRect )
	*boundingRect = baseRect;
    for( i = 0; i < nmarks; i++ ) {
	QChar mark = str.unicode()[pos+i+1];
	unsigned char cmb = combiningClass( mark );
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
	QRect markRect = f->boundingRect( mark );
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
	//qDebug( "char=%x combiningClass = %d offset=%d/%d", mark.unicode(), cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	p += QPoint( -baseOffset, 0 );
	attachmentRect |= markRect;
	if ( boundingRect )
	    *boundingRect |= markRect;
	lastCmb = cmb;
	pa.setPoint( i, p );
    }
    return pa;
#endif
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
    d->offsets = new Offset[d->num_glyphs];
}


void ScriptEngineBasic::position( ShapedItem *shaped )
{
    positionMarks( shaped );
}

int ScriptEngineBasic::cursorToX( int cPos, const ShapedItem &shaped )
{

}

int ScriptEngineBasic::xToCursor( int x, const ShapedItem &shaped )
{

}
