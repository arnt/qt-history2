#include "scriptenginelatin.h"
#include <stdlib.h>

#include <qstring.h>

void ScriptEngineLatin::charAttributes( const QString &text, int from, int len, CharAttributes *attributes )
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


void ScriptEngineLatin::shape( const QString &text, int from, int len, ShapedItem *result )
{
    ShapedItemPrivate *d = result->d;
    d->num_glyphs = len;
    d->glyphs = (int *)realloc( d->glyphs, d->num_glyphs*sizeof( int ) );
    bool reverse = d->analysis.bidiLevel % 2;
    int error = d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs, reverse );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (int *)realloc( d->glyphs, d->num_glyphs*sizeof( int ) );
	d->fontEngine->stringToCMap( text.unicode() + from, len, d->glyphs, &d->num_glyphs, reverse );
    }
    d->offsets = new Offset[d->num_glyphs];
}

int ScriptEngineLatin::cursorToX( int cPos, const QString &text, int from, int len, const ShapedItem &shaped )
{

}

int ScriptEngineLatin::xToCursor( int x, const QString &text, int from, int len, const ShapedItem &shaped )
{

}
