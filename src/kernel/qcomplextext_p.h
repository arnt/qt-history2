#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

#include <qstring.h>
#include <qpointarray.h>
#include <qfont.h>
#include <qpainter.h>

class QTextString;
class QFontPrivate;

class Q_EXPORT QComplexText {
public:
    enum Shape {
        XIsolated,
        XFinal,
        XInitial,
        XMedial
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    static QString shapedString( const QString &str, int from = 0, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
    static QChar shapedCharacter(const QString &str, int pos);
    static void glyphPositions( QTextString *str );

    // positions non spacing marks relative to the base character at position pos.
    static QPointArray positionMarks( QFontPrivate *f, const QString &str, int pos, QRect *boundingRect = 0 );
};


#endif
