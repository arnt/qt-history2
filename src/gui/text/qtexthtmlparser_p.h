#ifndef QTEXTHTMLPARSER_P_H
#define QTEXTHTMLPARSER_P_H

#include <qvector.h>
#include <qcolor.h>
#include <qfont.h>
#include <private/qtextformat_p.h>
#include <qstylesheet.h>

#include "qtextdocument.h"
#include "qtextcursor.h"

//idea for QString / QChar. QLatin1String is only useful if QString
//supports it natively for operator== et. al.

struct QLatin1String
{
    inline QLatin1String(const char *s):latin1(s){}
    const char *latin1;
    inline operator QString() const { return QString::fromLatin1(latin1); }
};

struct QLatin1Char
{
    inline QLatin1Char(const char c):latin1(c){}
    char latin1;
    inline operator QChar () const { return (ushort)latin1; }
};

inline bool operator==(const QString &a, const QLatin1String &b)
{
    const unsigned short *uc = a.ucs2();
    const unsigned char *c = (unsigned char *) b.latin1;

    while (*uc == *c) {
	if (!*uc)
	    return true;
	++uc;
	++c;
    }
    return false;
}
inline bool operator==(const QLatin1String &b, const QString &a)
{
    return operator==(a, b);
}
inline bool operator!=(const QLatin1String &b, const QString &a)
{
    return !operator==(a, b);
}
inline bool operator!=(const QString &a, const QLatin1String &b)
{
    return !operator==(a, b);
}
inline bool operator>(const QString &a, const QLatin1String &b)
{
    const unsigned short *uc = a.ucs2();
    const unsigned char *c = (unsigned char *) b.latin1;

    while (*uc == *c) {
	if (!*uc)
	    return false;
	++uc;
	++c;
    }
    return *uc > *c;

}
inline bool operator<(const QString &a, const QLatin1String &b)
{
    const unsigned short *uc = a.ucs2();
    const unsigned char *c = (unsigned char *) b.latin1;

    while (*uc == *c) {
	if (!*uc)
	    return false;
	++uc;
	++c;
    }
    return *uc < *c;

}
inline bool operator>(const QLatin1String &b, const QString &a)
{
    return operator<(a, b);
}
inline bool operator<(const QLatin1String &b, const QString &a)
{
    return operator>(a, b);
}
inline bool operator>=(const QString &a, const QLatin1String &b)
{
    return !operator<(a, b);
}
inline bool operator<=(const QString &a, const QLatin1String &b)
{
    return !operator>(a, b);
}
inline bool operator>=(const QLatin1String &b, const QString &a)
{
    return !operator>(a, b);
}
inline bool operator<=(const QLatin1String &b, const QString &a)
{
    return !operator<(a, b);
}

struct QTextHtmlParser;
struct QTextHtmlParserAttribute {
    enum {
	Add,
	Attributes,
	And,
	Styles,
	Here,
	That,
	The,
	Parser,
	Does,
	Not,
	Resolve,
	Eg,
	Tables
    } id;
    QString value;
};
Q_DECLARE_TYPEINFO(QTextHtmlParserAttribute, Q_MOVABLE_TYPE);

struct QTextHtmlParserNode {
    QTextHtmlParserNode():parent(0), isBlock(0), isListItem(0), isAnchor(false), isImage(false), fontItalic(0), fontUnderline(0), fontOverline(0),
    			fontStrikeOut(0), fontFixedPitch(0), fontPointSize(12), fontWeight(QFont::Normal),
			alignment(Qt::AlignAuto),listStyle(QTextListFormat::ListStyleUndefined), listIndex(-1), indent(0),
			imageWidth(-1), imageHeight(-1), tableIndex(-1), cellIndex(-1),
			wsm(QStyleSheetItem::WhiteSpaceModeUndefined), style(0)
    { margin[0] = margin[1] = margin[2] = margin[3] = margin[4] = 0; formatIndex = formatReference = 0; }
    QString tag;
    QString text;
    QVector<QTextHtmlParserAttribute> attributes;
    int parent;
    uint isBlock : 1;
    uint isListItem : 1;
    uint isAnchor : 1;
    uint isImage : 1;
    uint fontItalic : 1;
    uint fontUnderline : 1;
    uint fontOverline : 1;
    uint fontStrikeOut : 1;
    uint fontFixedPitch : 1;
    QString fontFamily;
    int fontPointSize;
    int fontWeight;
    QColor color;
    QColor bgColor;
    Qt::Alignment alignment;
    QTextListFormat::Style listStyle;
    int listIndex; // ### maybe it's better to do the list allocation in ::load (Simon)
    int indent;
    QString anchorHref;
    QString anchorName;
    QString imageName;
    int imageWidth;
    int imageHeight;
    int tableIndex;
    int cellIndex;

    // for the xml import
    union {
	struct {
	    int formatIndex;
	    int formatReference;
	};
	struct {
	    int propertyId;
	};
    };
    // ###
    QString propertyType;

    QStyleSheetItem::WhiteSpaceMode wsm;
private:
    int margin[5];
    friend class QTextHtmlParser;
    const QStyleSheetItem *style; // will go away
};
Q_DECLARE_TYPEINFO(QTextHtmlParserNode, Q_MOVABLE_TYPE);


struct QTextHtmlParser
{
public:
    QTextHtmlParser(QTextFormatCollection *formatCollection)
	: formats(formatCollection) {}

    inline const QTextHtmlParserNode &at(int i) const { return nodes.at(i); }
    inline QTextHtmlParserNode &operator[](int i) { return nodes[i]; }
    inline int count() const { return nodes.count(); }
    inline int last() const { return nodes.count()-1; }
    int depth(int i) const;
    int topMargin(int i) const;
    int bottomMargin(int i) const;
    inline int leftMargin(int i) const { return margin(i, QStyleSheetItem::MarginLeft); }
    inline int rightMargin(int i) const { return margin(i, QStyleSheetItem::MarginRight); }
    inline int firstLineMargin(int i) const { return margin(i, QStyleSheetItem::MarginFirstLine); }
    int findChild(int i, const char *tag) const;
    int findNextChild(int parent, int currentChild) const;

    void dumpHtml();

    void parse(const QString &text);

    static QTextCodec *codecForStream(const QByteArray &ba);
protected:
    QTextHtmlParserNode *newNode(int parent);
    QVector<QTextHtmlParserNode> nodes;
    QString txt;
    int pos, len;

    void parse();
    void parseTag();
    void parseCloseTag();
    void parseExclamationTag();
    QChar parseEntity();
    QString parseWord();
    void resolveParent();
    void resolveNode();
    void parseAttributes();
    void eatSpace();
    inline bool hasPrefix(QChar c, int lookahead = 0)
	{return pos + lookahead < len && txt.at(pos) == c; }
    int margin(int i, int mar) const;

    QTextFormatCollection *formats;
};

#endif // QTEXTHTMLPARSER_P_H
