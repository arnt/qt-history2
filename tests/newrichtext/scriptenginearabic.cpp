#include "scriptenginearabic.h"

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


void ScriptEngineArabic::shape( const FontEngine &f, const QString &text, int from, int len,
			const ScriptAnalysis &analysis, ShapedItem *result )
{

}

int ScriptEngineArabic::cursorToX( int cursorPos, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}

int ScriptEngineArabic::xToCursor( int x, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}
