#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

#include <qstring.h>

class QTextString;

class QComplexText {
public:
    enum Shape {
	XIsolated,
	XFinal,
	XInitial,
	XMedial
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    static const QChar *shapedString( const QString &str, int from, int len, int *lenOut );
    static QChar shapedCharacter(const QString &str, int pos);
    static void glyphPositions( QTextString *str );
};


#endif
