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


void ScriptEngineArabic::shape( const FontEngineIface &f, const QString &text, int from, int len,
			const ScriptAnalysis &analysis, ShapedItem *result )
{
    QPainter::TextDirection dir = analysis.bidiLevel % 2 ? QPainter::RTL : QPainter::LTR;
    QString shaped = QComplexText::shapedString( text, from, len, dir );
    len = shaped.length();

    result->d->num_glyphs = len;
    result->d->glyphs = (int *)realloc( result->d->glyphs, result->d->num_glyphs*sizeof( int ) );
    int error = f.stringToCMap( shaped.unicode(), len, result->d->glyphs, &result->d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	result->d->glyphs = (int *)realloc( result->d->glyphs, result->d->num_glyphs*sizeof( int ) );
	f.stringToCMap( shaped.unicode(), len, result->d->glyphs, &result->d->num_glyphs );
    }
    result->d->offsets = new Offset[result->d->num_glyphs];
}

int ScriptEngineArabic::cursorToX( int cPos, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}

int ScriptEngineArabic::xToCursor( int x, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}
