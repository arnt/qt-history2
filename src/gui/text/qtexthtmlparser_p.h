#ifndef QTEXTHTMLPARSER_P_H
#define QTEXTHTMLPARSER_P_H

#ifndef QT_H
#include <qvector.h>
#include <qcolor.h>
#include <qfont.h>
#include <private/qtextformat_p.h>
#include <private/qtextdocument_p.h>
#include <qstylesheet.h>

#include "qtextdocument.h"
#include "qtextcursor.h"
#endif // QT_H


class QTextHtmlParser;
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
    inline QTextHtmlParserNode():parent(0), isBlock(0), isListItem(0), isListStart(false), isTableCell(false), isAnchor(false),
                          isImage(false), fontItalic(0), fontUnderline(0), fontOverline(0),
                          fontStrikeOut(0), fontFixedPitch(0), fontPointSize(12), fontWeight(QFont::Normal),
                          alignment(Qt::AlignAuto),listStyle(QTextListFormat::ListStyleUndefined),
                          imageWidth(-1), imageHeight(-1),
                          wsm(QStyleSheetItem::WhiteSpaceModeUndefined), style(0)
    { margin[0] = margin[1] = margin[2] = margin[3] = margin[4] = 0; }
    QString tag;
    QString text;
    QVector<QTextHtmlParserAttribute> attributes;
    int parent;
    uint isBlock : 1;
    uint isListItem : 1;
    uint isListStart : 1;
    uint isTableCell : 1;
    uint isAnchor : 1;
    uint isImage : 1;
    uint fontItalic : 1;
    uint fontUnderline : 1;
    uint fontOverline : 1;
    uint fontStrikeOut : 1;
    uint fontFixedPitch : 1;
    QTextFrameFormat::Position cssFloat : 2;
    QString fontFamily;
    int fontPointSize;
    int fontWeight;
    QColor color;
    QColor bgColor;
    Qt::Alignment alignment;
    QTextListFormat::Style listStyle;
    QString anchorHref;
    QString anchorName;
    QString imageName;
    int imageWidth;
    int imageHeight;

    QStyleSheetItem::WhiteSpaceMode wsm;
private:
    int margin[5];
    friend class QTextHtmlParser;
    const QStyleSheetItem *style; // will go away
};
Q_DECLARE_TYPEINFO(QTextHtmlParserNode, Q_MOVABLE_TYPE);


class QTextHtmlParser
{
public:
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
};

#endif // QTEXTHTMLPARSER_P_H
