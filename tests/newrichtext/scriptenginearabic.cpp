#include "scriptenginearabic.h"

#include "private/qcomplextext_p.h"

#include <stdlib.h>

void ScriptEngineArabic::charAttributes( const QString &text, int from, int len, CharAttributes *attributes )
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


void ScriptEngineArabic::shape( ShapedItem *result )
{
    const QString &text = result->d->string;
    int from = result->d->from;
    int len = result->d->length;

    ShapedItemPrivate *d = result->d;
    QPainter::TextDirection dir = d->analysis.bidiLevel % 2 ? QPainter::RTL : QPainter::LTR;
    // ### this is hacky and should work on glyphs
    QString shaped = QComplexText::shapedString( text, from, len, dir );
    len = shaped.length();

    d->num_glyphs = len;
    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( shaped.unicode(), len, d->glyphs, &d->num_glyphs, FALSE );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( shaped.unicode(), len, d->glyphs, &d->num_glyphs, FALSE );
    }
    d->offsets = new Offset[d->num_glyphs];
}
