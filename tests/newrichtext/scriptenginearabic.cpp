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


void ScriptEngineArabic::shape( const QString &text, int from, int len, ShapedItem *result )
{
    ShapedItemPrivate *d = result->d;
    QPainter::TextDirection dir = d->analysis.bidiLevel % 2 ? QPainter::RTL : QPainter::LTR;
    QString shaped = QComplexText::shapedString( text, from, len, dir );
    len = shaped.length();

    d->num_glyphs = len;
    d->glyphs = (int *)realloc( d->glyphs, d->num_glyphs*sizeof( int ) );
    int error = d->fontEngine->stringToCMap( shaped.unicode(), len,
						 d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (int *)realloc( d->glyphs, d->num_glyphs*sizeof( int ) );
	d->fontEngine->stringToCMap( shaped.unicode(), len, d->glyphs, &d->num_glyphs );
    }
    d->offsets = new Offset[d->num_glyphs];
}

int ScriptEngineArabic::cursorToX( int cPos, const QString &text, int from, int len, const ShapedItem &shaped )
{

}

int ScriptEngineArabic::xToCursor( int x, const QString &text, int from, int len, const ShapedItem &shaped )
{

}
