#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

#include <qstring.h>

class QComplexText {
public:
    enum Shape {
	XIsolated,
	XRight,
	XLeft,
	XMedial,
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    static const QChar *shapedString( const QString &str, int from, int len, int *lenOut );
    static QChar shapedCharacter(const QString &str, int pos);
};


#endif
