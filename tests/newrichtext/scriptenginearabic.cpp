#include "scriptenginearabic.h"

#include "private/qcomplextext_p.h"

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
    QPainter::TextDirection dir = analysis.bidiLevel % 2 ? QPainter::RTL : QPainter::LTR;
    QString shaped = QComplexText::shapedString( text, from, len, dir );

    result->d = (ShapedItemPrivate *)(new QString(shaped));
}

int ScriptEngineArabic::cursorToX( int cursorPos, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}

int ScriptEngineArabic::xToCursor( int x, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped )
{

}
