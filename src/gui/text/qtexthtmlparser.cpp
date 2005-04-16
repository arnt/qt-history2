/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtexthtmlparser_p.h"

#include <qbytearray.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdebug.h>

#include "qtextdocument.h"
#include "qtextformat_p.h"
#include "qtextdocument_p.h"
#include "qtextcursor.h"

const int DefaultFontSize = 12;

#define MAX_ENTITY 259
static const struct QTextHtmlEntity { const char *name; quint16 code; } entities[MAX_ENTITY]= {
    { "AElig", 0x00c6 },
    { "Aacute", 0x00c1 },
    { "Acirc", 0x00c2 },
    { "Agrave", 0x00c0 },
    { "Alpha", 0x0391 },
    { "AMP", 38 },
    { "Aring", 0x00c5 },
    { "Atilde", 0x00c3 },
    { "Auml", 0x00c4 },
    { "Beta", 0x0392 },
    { "Ccedil", 0x00c7 },
    { "Chi", 0x03a7 },
    { "Dagger", 0x2021 },
    { "Delta", 0x0394 },
    { "ETH", 0x00d0 },
    { "Eacute", 0x00c9 },
    { "Ecirc", 0x00ca },
    { "Egrave", 0x00c8 },
    { "Epsilon", 0x0395 },
    { "Eta", 0x0397 },
    { "Euml", 0x00cb },
    { "Gamma", 0x0393 },
    { "GT", 62 },
    { "Iacute", 0x00cd },
    { "Icirc", 0x00ce },
    { "Igrave", 0x00cc },
    { "Iota", 0x0399 },
    { "Iuml", 0x00cf },
    { "Kappa", 0x039a },
    { "Lambda", 0x039b },
    { "LT", 60 },
    { "Mu", 0x039c },
    { "Ntilde", 0x00d1 },
    { "Nu", 0x039d },
    { "OElig", 0x0152 },
    { "Oacute", 0x00d3 },
    { "Ocirc", 0x00d4 },
    { "Ograve", 0x00d2 },
    { "Omega", 0x03a9 },
    { "Omicron", 0x039f },
    { "Oslash", 0x00d8 },
    { "Otilde", 0x00d5 },
    { "Ouml", 0x00d6 },
    { "Phi", 0x03a6 },
    { "Pi", 0x03a0 },
    { "Prime", 0x2033 },
    { "Psi", 0x03a8 },
    { "QUOT", 34 },
    { "Rho", 0x03a1 },
    { "Scaron", 0x0160 },
    { "Sigma", 0x03a3 },
    { "THORN", 0x00de },
    { "Tau", 0x03a4 },
    { "Theta", 0x0398 },
    { "Uacute", 0x00da },
    { "Ucirc", 0x00db },
    { "Ugrave", 0x00d9 },
    { "Upsilon", 0x03a5 },
    { "Uuml", 0x00dc },
    { "Xi", 0x039e },
    { "Yacute", 0x00dd },
    { "Yuml", 0x0178 },
    { "Zeta", 0x0396 },
    { "aacute", 0x00e1 },
    { "acirc", 0x00e2 },
    { "acute", 0x00b4 },
    { "aelig", 0x00e6 },
    { "agrave", 0x00e0 },
    { "alefsym", 0x2135 },
    { "alpha", 0x03b1 },
    { "amp", 38 },
    { "and", 0x22a5 },
    { "ang", 0x2220 },
    { "apos", 0x0027 },
    { "aring", 0x00e5 },
    { "asymp", 0x2248 },
    { "atilde", 0x00e3 },
    { "auml", 0x00e4 },
    { "bdquo", 0x201e },
    { "beta", 0x03b2 },
    { "brvbar", 0x00a6 },
    { "bull", 0x2022 },
    { "cap", 0x2229 },
    { "ccedil", 0x00e7 },
    { "cedil", 0x00b8 },
    { "cent", 0x00a2 },
    { "chi", 0x03c7 },
    { "circ", 0x02c6 },
    { "clubs", 0x2663 },
    { "cong", 0x2245 },
    { "copy", 0x00a9 },
    { "crarr", 0x21b5 },
    { "cup", 0x222a },
    { "curren", 0x00a4 },
    { "dArr", 0x21d3 },
    { "dagger", 0x2020 },
    { "darr", 0x2193 },
    { "deg", 0x00b0 },
    { "delta", 0x03b4 },
    { "diams", 0x2666 },
    { "divide", 0x00f7 },
    { "eacute", 0x00e9 },
    { "ecirc", 0x00ea },
    { "egrave", 0x00e8 },
    { "empty", 0x2205 },
    { "emsp", 0x2003 },
    { "ensp", 0x2002 },
    { "epsilon", 0x03b5 },
    { "equiv", 0x2261 },
    { "eta", 0x03b7 },
    { "eth", 0x00f0 },
    { "euml", 0x00eb },
    { "euro", 0x20ac },
    { "exist", 0x2203 },
    { "fnof", 0x0192 },
    { "forall", 0x2200 },
    { "frac12", 0x00bd },
    { "frac14", 0x00bc },
    { "frac34", 0x00be },
    { "frasl", 0x2044 },
    { "gamma", 0x03b3 },
    { "ge", 0x2265 },
    { "gt", 62 },
    { "hArr", 0x21d4 },
    { "harr", 0x2194 },
    { "hearts", 0x2665 },
    { "hellip", 0x2026 },
    { "iacute", 0x00ed },
    { "icirc", 0x00ee },
    { "iexcl", 0x00a1 },
    { "igrave", 0x00ec },
    { "image", 0x2111 },
    { "infin", 0x221e },
    { "int", 0x222b },
    { "iota", 0x03b9 },
    { "iquest", 0x00bf },
    { "isin", 0x2208 },
    { "iuml", 0x00ef },
    { "kappa", 0x03ba },
    { "lArr", 0x21d0 },
    { "lambda", 0x03bb },
    { "lang", 0x2329 },
    { "laquo", 0x00ab },
    { "larr", 0x2190 },
    { "lceil", 0x2308 },
    { "ldquo", 0x201c },
    { "le", 0x2264 },
    { "lfloor", 0x230a },
    { "lowast", 0x2217 },
    { "loz", 0x25ca },
    { "lrm", 0x200e },
    { "lsaquo", 0x2039 },
    { "lsquo", 0x2018 },
    { "lt", 60 },
    { "macr", 0x00af },
    { "mdash", 0x2014 },
    { "micro", 0x00b5 },
    { "middot", 0x00b7 },
    { "minus", 0x2212 },
    { "mu", 0x03bc },
    { "nabla", 0x2207 },
    { "nbsp", 0x00a0 },
    { "ndash", 0x2013 },
    { "ne", 0x2260 },
    { "ni", 0x220b },
    { "not", 0x00ac },
    { "notin", 0x2209 },
    { "nsub", 0x2284 },
    { "ntilde", 0x00f1 },
    { "nu", 0x03bd },
    { "oacute", 0x00f3 },
    { "ocirc", 0x00f4 },
    { "oelig", 0x0153 },
    { "ograve", 0x00f2 },
    { "oline", 0x203e },
    { "omega", 0x03c9 },
    { "omicron", 0x03bf },
    { "oplus", 0x2295 },
    { "or", 0x22a6 },
    { "ordf", 0x00aa },
    { "ordm", 0x00ba },
    { "oslash", 0x00f8 },
    { "otilde", 0x00f5 },
    { "otimes", 0x2297 },
    { "ouml", 0x00f6 },
    { "para", 0x00b6 },
    { "part", 0x2202 },
    { "percnt", 0x0025 },
    { "permil", 0x2030 },
    { "perp", 0x22a5 },
    { "phi", 0x03c6 },
    { "pi", 0x03c0 },
    { "piv", 0x03d6 },
    { "plusmn", 0x00b1 },
    { "pound", 0x00a3 },
    { "prime", 0x2032 },
    { "prod", 0x220f },
    { "prop", 0x221d },
    { "psi", 0x03c8 },
    { "quot", 34 },
    { "rArr", 0x21d2 },
    { "radic", 0x221a },
    { "rang", 0x232a },
    { "raquo", 0x00bb },
    { "rarr", 0x2192 },
    { "rceil", 0x2309 },
    { "rdquo", 0x201d },
    { "real", 0x211c },
    { "reg", 0x00ae },
    { "rfloor", 0x230b },
    { "rho", 0x03c1 },
    { "rlm", 0x200f },
    { "rsaquo", 0x203a },
    { "rsquo", 0x2019 },
    { "sbquo", 0x201a },
    { "scaron", 0x0161 },
    { "sdot", 0x22c5 },
    { "sect", 0x00a7 },
    { "shy", 0x00ad },
    { "sigma", 0x03c3 },
    { "sigmaf", 0x03c2 },
    { "sim", 0x223c },
    { "spades", 0x2660 },
    { "sub", 0x2282 },
    { "sube", 0x2286 },
    { "sum", 0x2211 },
    { "sup1", 0x00b9 },
    { "sup2", 0x00b2 },
    { "sup3", 0x00b3 },
    { "sup", 0x2283 },
    { "supe", 0x2287 },
    { "szlig", 0x00df },
    { "tau", 0x03c4 },
    { "there4", 0x2234 },
    { "theta", 0x03b8 },
    { "thetasym", 0x03d1 },
    { "thinsp", 0x2009 },
    { "thorn", 0x00fe },
    { "tilde", 0x02dc },
    { "times", 0x00d7 },
    { "trade", 0x2122 },
    { "uArr", 0x21d1 },
    { "uacute", 0x00fa },
    { "uarr", 0x2191 },
    { "ucirc", 0x00fb },
    { "ugrave", 0x00f9 },
    { "uml", 0x00a8 },
    { "upsih", 0x03d2 },
    { "upsilon", 0x03c5 },
    { "uuml", 0x00fc },
    { "weierp", 0x2118 },
    { "xi", 0x03be },
    { "yacute", 0x00fd },
    { "yen", 0x00a5 },
    { "yuml", 0x00ff },
    { "zeta", 0x03b6 },
    { "zwj", 0x200d },
    { "zwnj", 0x200c },
    { 0, 0 }
};

static bool operator<(const QString &entityStr, const QTextHtmlEntity &entity)
{
    return entityStr < QLatin1String(entity.name);
}

static bool operator<(const QTextHtmlEntity &entity, const QString &entityStr)
{
    return QLatin1String(entity.name) < entityStr;
}

static QChar resolveEntity(const QString &entity)
{
    const QTextHtmlEntity *start = &entities[0];
    const QTextHtmlEntity *end = &entities[MAX_ENTITY-1];
    const QTextHtmlEntity *e = qBinaryFind(start, end, entity);
    if (!e->name)
        return QChar();
    return e->code;
}

// the displayMode value is according to the what are blocks in the piecetable, not
// what the w3c defines.
static const struct QTextHtmlElement
{
    const char *name;
    int id;
    enum DisplayMode { DisplayBlock, DisplayInline, DisplayNone } displayMode;
} elements[Html_NumElements+1]= {
    { "a", Html_a, QTextHtmlElement::DisplayInline },
    { "b", Html_b, QTextHtmlElement::DisplayInline },
    { "big", Html_big, QTextHtmlElement::DisplayInline },
    { "blockquote", Html_blockquote, QTextHtmlElement::DisplayBlock },
    { "body", Html_body, QTextHtmlElement::DisplayBlock },
    { "br", Html_br, QTextHtmlElement::DisplayInline },
    { "center", Html_center, QTextHtmlElement::DisplayBlock },
    { "code", Html_code, QTextHtmlElement::DisplayInline },
    { "dd", Html_dd, QTextHtmlElement::DisplayBlock },
    { "div", Html_div, QTextHtmlElement::DisplayBlock },
    { "dl", Html_dl, QTextHtmlElement::DisplayBlock },
    { "dt", Html_dt, QTextHtmlElement::DisplayBlock },
    { "em", Html_em, QTextHtmlElement::DisplayInline },
    { "font", Html_font, QTextHtmlElement::DisplayInline },
    { "h1", Html_h1, QTextHtmlElement::DisplayBlock },
    { "h2", Html_h2, QTextHtmlElement::DisplayBlock },
    { "h3", Html_h3, QTextHtmlElement::DisplayBlock },
    { "h4", Html_h4, QTextHtmlElement::DisplayBlock },
    { "h5", Html_h5, QTextHtmlElement::DisplayBlock },
    { "h6", Html_h6, QTextHtmlElement::DisplayBlock },
    { "head", Html_head, QTextHtmlElement::DisplayNone },
    { "hr", Html_hr, QTextHtmlElement::DisplayInline },
    { "html", Html_html, QTextHtmlElement::DisplayInline },
    { "i", Html_i, QTextHtmlElement::DisplayInline },
    { "img", Html_img, QTextHtmlElement::DisplayInline },
    { "li", Html_li, QTextHtmlElement::DisplayBlock },
    { "meta", Html_meta, QTextHtmlElement::DisplayNone },
    { "nobr", Html_nobr, QTextHtmlElement::DisplayInline },
    { "ol", Html_ol, QTextHtmlElement::DisplayBlock },
    { "p", Html_p, QTextHtmlElement::DisplayBlock },
    { "pre", Html_pre, QTextHtmlElement::DisplayBlock },
    { "qt", Html_qt, QTextHtmlElement::DisplayBlock },
    { "s", Html_s, QTextHtmlElement::DisplayInline },
    { "small", Html_small, QTextHtmlElement::DisplayInline },
    { "span", Html_span, QTextHtmlElement::DisplayInline },
    { "strong", Html_strong, QTextHtmlElement::DisplayInline },
    { "style", Html_style, QTextHtmlElement::DisplayNone },
    { "sub", Html_sub, QTextHtmlElement::DisplayInline },
    { "sup", Html_sup, QTextHtmlElement::DisplayInline },
    { "table", Html_table, QTextHtmlElement::DisplayBlock },
    { "td", Html_td, QTextHtmlElement::DisplayBlock },
    { "th", Html_th, QTextHtmlElement::DisplayBlock },
    { "title", Html_title, QTextHtmlElement::DisplayNone },
    { "tr", Html_tr, QTextHtmlElement::DisplayBlock },
    { "tt", Html_tt, QTextHtmlElement::DisplayInline },
    { "u", Html_u, QTextHtmlElement::DisplayInline },
    { "ul", Html_ul, QTextHtmlElement::DisplayBlock },
    { 0, 0, QTextHtmlElement::DisplayNone }
};


static bool operator<(const QString &str, const QTextHtmlElement &e)
{
    return str < QLatin1String(e.name);
}

static bool operator<(const QTextHtmlElement &e, const QString &str)
{
    return QLatin1String(e.name) < str;
}

static const QTextHtmlElement *lookupElement(const QString &element)
{
    const QTextHtmlElement *start = &elements[0];
    const QTextHtmlElement *end = &elements[Html_NumElements];
    const QTextHtmlElement *e = qBinaryFind(start, end, element);
    Q_ASSERT(!e->name || e->name == element);
    return e;
}

int QTextHtmlParser::lookupElement(const QString &element)
{
    const QTextHtmlElement *e = ::lookupElement(element);
    if (!e->name)
        return -1;
    return e->id;
}

static int scaleFontPointSize(int fontPointSize, int logicalFontSize, int logicalFontSizeStep = 0)
{
    if (logicalFontSize != -1 || logicalFontSizeStep) {
        int logical = logicalFontSize;
        if (logical < 0)
            logical = 3;
        logical += logicalFontSizeStep;
        if (logical < 0)
            logical = 0;
        else if (logical > 7)
            logical = 8;
        switch (logical) {
        case 1:
            fontPointSize =  (7 * fontPointSize) / 10;
            break;
        case 2:
            fontPointSize = (8 * fontPointSize) / 10;
            break;
        case 4:
            fontPointSize =  (12 * fontPointSize) / 10;
            break;
        case 5:
            fontPointSize = (15 * fontPointSize) / 10;
            break;
        case 6:
            fontPointSize = 2 * fontPointSize;
            break;
        case 7:
            fontPointSize = (24 * fontPointSize) / 10;
            break;
        };
    }
    return fontPointSize;
}

// quotes newlines as "\\n"
static QString quoteNewline(const QString &s)
{
    QString n = s;
    n.replace('\n', "\\n");
    return n;
}

QTextHtmlParserNode::QTextHtmlParserNode()
    : parent(0), id(-1), isBlock(false), isListItem(false), isListStart(false), isTableCell(false), isAnchor(false),
      fontItalic(false), fontUnderline(false), fontOverline(false), fontStrikeOut(false), fontFixedPitch(false),
      cssFloat(QTextFrameFormat::InFlow), hasOwnListStyle(false), hasFontPointSize(false),
      hasCssBlockIndent(false), hasCssListIndent(false), isEmptyParagraph(false), direction(3),
      fontPointSize(DefaultFontSize),
      fontWeight(QFont::Normal), alignment(0), verticalAlignment(QTextCharFormat::AlignNormal),
      listStyle(QTextListFormat::ListStyleUndefined), imageWidth(-1), imageHeight(-1), tableBorder(0),
      tableCellRowSpan(1), tableCellColSpan(1), tableCellSpacing(2), tableCellPadding(0), cssBlockIndent(0),
      cssListIndent(0), text_indent(0), wsm(WhiteSpaceModeUndefined)
{
    margin[QTextHtmlParser::MarginLeft] = 0;
    margin[QTextHtmlParser::MarginRight] = 0;
    margin[QTextHtmlParser::MarginTop] = 0;
    margin[QTextHtmlParser::MarginBottom] = 0;
}

QTextCharFormat QTextHtmlParserNode::charFormat() const
{
    QTextCharFormat format;

    format.setFontItalic(fontItalic);
    format.setFontUnderline(fontUnderline);
    format.setFontOverline(fontOverline);
    format.setFontStrikeOut(fontStrikeOut);
    format.setFontFixedPitch(fontFixedPitch);
    if (fontFamily.size())
        format.setFontFamily(fontFamily);
    if (hasFontPointSize)
        format.setFontPointSize(fontPointSize);
    format.setFontWeight(fontWeight);
    if (color.isValid())
        format.setForeground(QBrush(color));
    if (verticalAlignment != QTextCharFormat::AlignNormal)
        format.setVerticalAlignment(verticalAlignment);
    if (isAnchor) {
        format.setAnchor(true);
        format.setAnchorHref(anchorHref);
        format.setAnchorName(anchorName);
    }

    return format;
}

void QTextHtmlParser::dumpHtml()
{
    for (int i = 0; i < count(); ++i) {
        qDebug().nospace() << QString(depth(i)*4, ' ')
                           << at(i).tag << ":"
                           << "\"" << quoteNewline(at(i).text) << "\" "
            ;
    }
}

QTextHtmlParserNode *QTextHtmlParser::newNode(int parent)
{
    QTextHtmlParserNode *node = &nodes.last();
    // create new new, or reuse the last one
    if (nodes.count() == 1 || node->tag.size()
        || (node->text.size() && node->text != QLatin1String(" "))) {
        nodes.resize(nodes.size() + 1);
        node = &nodes.last();
    } else {
        node->tag.clear();
        node->text.clear();
        node->id = -1;
    }
    node->parent = parent;
    return node;
}

void QTextHtmlParser::parse(const QString &text)
{
    nodes.clear();
    nodes.resize(1);
    txt = text;
    pos = 0;
    len = txt.length();
    textEditMode = false;
    parse();
    //dumpHtml();
}

int QTextHtmlParser::depth(int i) const
{
    int depth = 0;
    while (i) {
        i = at(i).parent;
        ++depth;
    }
    return depth;
}

int QTextHtmlParser::margin(int i, int mar) const {
    int m = 0;
    const QTextHtmlParserNode *node;
    if (mar == MarginLeft
        || mar == MarginRight) {
        while (i) {
            node = &at(i);
            if (!node->isBlock)
                return 0;
            m += node->margin[mar];
            i = node->parent;
        }
    }
    return m;
}

int QTextHtmlParser::topMargin(int i) const
{
    int m = 0;
    const QTextHtmlParserNode *node;
    while (i) {
        node = &at(i);
        if (!node->isBlock)
            return 0;
        m = qMax(m, node->margin[MarginTop]);

        // collapsing margins across table cells makes no sense
        if (node->isTableCell)
            break;

        // don't collapse margins across list items
        // (the top margin of the list is merged as part of the block
        // merging in documentfragment.cpp)
        if (node->isListItem)
            break;

        // <ul>
        //  ..
        //  <ul> <-- this one should not take the first <ul>'s margin into account
        if (node->isNestedList(this))
            break;

        // get previous block
        while (i-1 && !at(i-1).isBlock)
            --i;
        if (i && node->parent == at(i).parent)
            break;
        i = node->parent;
    }
    return m;
}

int QTextHtmlParser::bottomMargin(int i) const
{
    int m = 0;
    const QTextHtmlParserNode *node;
    while (i) {
        node = &at(i);
        if (!node->isBlock)
            return 0;
        m = qMax(m, node->margin[MarginBottom]);

        // collapsing margins across table cells makes no sense
        if (node->isTableCell)
            break;

        // don't collapse margins across list items
        if (node->isListItem)
            break;

        // <ul>
        //  ..
        //  <ul> <-- this one should not take the first <ul>'s margin into account
        if (node->isNestedList(this))
            break;

        // get next block
        while (i+1 < count() && !at(i+1).isBlock)
            ++i;
        if (i && node->parent == at(i).parent)
            break;
        i = node->parent;
    }
    return m;
}

void QTextHtmlParser::eatSpace()
{
    while (pos < len && txt.at(pos).isSpace() && txt.at(pos) != QChar::ParagraphSeparator)
        pos++;
}

void QTextHtmlParser::parse() {
    QTextHtmlParserNode::WhiteSpaceMode wsm = QTextHtmlParserNode::WhiteSpaceNormal;
    while (pos < len) {
        QChar c = txt.at(pos++);
        if (c == QLatin1Char('<')) {
            parseTag();
            wsm = nodes.last().wsm;
        } else if (c == QLatin1Char('&')) {
            nodes.last().text += parseEntity();
        } else {
            if (c.isSpace() && c != QChar(QChar::Nbsp) && c != QChar::ParagraphSeparator) {
                if (wsm == QTextHtmlParserNode::WhiteSpacePre
                    || textEditMode) {
                    if (c == QLatin1Char('\n'))
                        c = QChar::LineSeparator;
                    else if (c == QLatin1Char('\r'))
                        continue;

                    if (textEditMode
                        && c == QChar::LineSeparator)
                        continue;
                } else if (wsm != QTextHtmlParserNode::WhiteSpacePreWrap) { // non-pre mode: collapse whitespace except nbsp
                    while (pos < len && txt.at(pos).isSpace()
                           && txt.at(pos) != QChar::Nbsp)
                        pos++;
                    c = QLatin1Char(' ');
                }
            }
            nodes.last().text += c;
        }
    }
}

// parses a tag after "<"
void QTextHtmlParser::parseTag()
{
    eatSpace();

    // handle comments and other exclamation mark declarations
    if (hasPrefix(QLatin1Char('!'))) {
        parseExclamationTag();
        eatSpace();
        return;
    }

    // if close tag just close
    if (hasPrefix(QLatin1Char('/'))) {
        parseCloseTag();
        return;
    }

    int p = last();
    while (p && at(p).tag.size() == 0)
        p = at(p).parent;

    QTextHtmlParserNode *node = newNode(p);

    // parse tag name
    node->tag = parseWord().toLower();

    const QTextHtmlElement *elem = ::lookupElement(node->tag);
    if (elem->name) {
        node->id = elem->id;
        node->isBlock = (elem->displayMode == QTextHtmlElement::DisplayBlock);
    } else {
        node->id = -1;
    }

    node->isListItem = (node->id == Html_li);
    node->isListStart = (node->id == Html_ol || node->id == Html_ul);
    node->isTableCell = (node->id == Html_td || node->id == Html_th);

    resolveParent();
    resolveNode();
    // _need_ at least one space after the tag name, otherwise there can't be attributes
    if (pos < len && txt.at(pos).isSpace())
        parseAttributes();

    // special handling for anchors with href attribute (hyperlinks)
    if (node->isAnchor && !node->anchorHref.isEmpty()) {
        node->fontUnderline = true; // ####
        node->color = Qt::blue; // ####
    }

    // finish tag
    while (pos < len && txt.at(pos++) != QLatin1Char('>'))
        ;

    if (node->wsm != QTextHtmlParserNode::WhiteSpacePre
        && node->wsm != QTextHtmlParserNode::WhiteSpacePreWrap)
        eatSpace();

    if (node->mayNotHaveChildren()) {
        newNode(node->parent);
        resolveNode();
    }
}

// parses a tag beginning with "/"
void QTextHtmlParser::parseCloseTag()
{
    ++pos;
    QString tag = parseWord().toLower().trimmed();
    while (pos < len) {
        QChar c = txt.at(pos++);
        if (c == QLatin1Char('>'))
            break;
    }

    // find corresponding open node
    int p = last();
    while (p && at(p).tag != tag)
        p = at(p).parent;

    newNode(at(p).parent);
    resolveNode();
}

// parses a tag beginning with "!"
void QTextHtmlParser::parseExclamationTag()
{
    ++pos;
    if (hasPrefix(QLatin1Char('-'),1) && hasPrefix(QLatin1Char('-'),2)) {
        pos += 3;
        // eat comments
        int end = txt.indexOf(QLatin1String("-->"), pos);
        pos = (end >= 0 ? end + 3 : len);
    } else {
        // eat internal tags
        while (pos < len) {
            QChar c = txt.at(pos++);
            if (c == QLatin1Char('>'))
                break;
        }
    }
}

// parses an entity after "&", and returns it
QString QTextHtmlParser::parseEntity()
{
    int recover = pos;
    QString entity;
    while (pos < len) {
        QChar c = txt.at(pos++);
        if (c.isSpace() || pos - recover > 8) {
            goto error;
        }
        if (c == QLatin1Char(';'))
            break;
        entity += c;
    }
    {
        QChar resolved = resolveEntity(entity);
        if (!resolved.isNull())
            return QString(resolved);
    }
    if (entity.length() > 1 && entity.at(0) == QLatin1Char('#')) {
        entity.remove(0, 1); // removing leading #

        int base = 10;
        bool ok = false;

        if (entity.at(0).toLower() == QLatin1Char('x')) { // hex entity?
            entity.remove(0, 1);
            base = 16;
        }

        int uc = entity.toInt(&ok, base);
        if (ok) {
            if (uc == 151) // ### hack for designer manual
                uc = '-';
            QString str;
            if (uc > 0xffff) {
                // surrogate pair
                uc -= 0x10000;
                ushort high = uc/0x400 + 0xd800;
                ushort low = uc%0x400 + 0xdc00;
                str.append(QChar(high));
                str.append(QChar(low));
            } else {
                str.append(QChar(uc));
            }
            return str;
        }
    }
error:
    pos = recover;
    return QLatin1String("&");
}

// parses one word, possibly quoted, and returns it
QString QTextHtmlParser::parseWord()
{
    QString word;
    if (hasPrefix(QLatin1Char('\"'))) { // double quotes
        ++pos;
        while (pos < len) {
            QChar c = txt.at(pos++);
            if (c == QLatin1Char('\"'))
                break;
            else if (c == QLatin1Char('&'))
                word += parseEntity();
            else
                word += c;
        }
    } else if (hasPrefix(QLatin1Char('\''))) { // single quotes
        ++pos;
        while (pos < len) {
            QChar c = txt.at(pos++);
            if (c == QLatin1Char('\''))
                break;
            else
                word += c;
        }
    } else { // normal text
        while (pos < len) {
            QChar c = txt.at(pos++);
            if (c == QLatin1Char('>')
                || (c == '/' && hasPrefix(QLatin1Char('>'), 1))
                || c == QLatin1Char('<')
                || c == QLatin1Char('=')
                || c.isSpace()) {
                --pos;
                break;
            }
            if (c == QLatin1Char('&'))
                word += parseEntity();
            else
                word += c;
        }
    }
    return word;
}

// gives the new node the right parent
void QTextHtmlParser::resolveParent()
{
    QTextHtmlParserNode *node = &nodes.last();
    int p = node->parent;

    // block elements close inline elements
    // ... with the exception of the font element ... grmbl ...
    if (node->isBlock)
        while (p
               && !at(p).isBlock
               && at(p).id != Html_font) {
            p = at(p).parent;
        }

    // some elements are not self nesting
    if (node->tag == at(p).tag) {
        if (node->isNotSelfNesting())
            p = at(p).parent;
    }

    // some elements are not allowed in certain contexts
    while (p && !node->allowedInContext(at(p).id)
           // ### make new styles aware of empty tags
           || at(p).id == Html_hr
           || at(p).id == Html_br
           || at(p).id == Html_img
       ) {
        p = at(p).parent;
    }

    node->parent = p;

    // makes it easier to traverse the tree, later
    nodes[p].children.append(nodes.count() - 1);
}

// sets all properties on the new node
void QTextHtmlParser::resolveNode()
{
    QTextHtmlParserNode *node = &nodes.last();
    const QTextHtmlParserNode *parent = &nodes.at(node->parent);
    node->initializeProperties(parent, this);
}

bool QTextHtmlParserNode::isNestedList(const QTextHtmlParser *parser) const
{
    if (!isListStart)
        return false;

    int p = parent;
    while (p) {
        if (parser->at(p).isListStart)
            return true;
        p = parser->at(p).parent;
    }
    return false;
}

void QTextHtmlParserNode::initializeProperties(const QTextHtmlParserNode *parent, const QTextHtmlParser *parser)
{
    // inherit properties from parent element
    isAnchor = parent->isAnchor;
    fontItalic = parent->fontItalic;
    fontUnderline = parent->fontUnderline;
    fontOverline = parent->fontOverline;
    fontStrikeOut = parent->fontStrikeOut;
    fontFixedPitch = parent->fontFixedPitch;
    fontFamily = parent->fontFamily;
    hasFontPointSize = parent->hasFontPointSize;
    fontPointSize = parent->fontPointSize;
    fontWeight = parent->fontWeight;
    color = parent->color;
    verticalAlignment = parent->verticalAlignment;

    if (parent->id != Html_table) {
        alignment = parent->alignment;
        bgColor = parent->bgColor;
    }

    listStyle = parent->listStyle;
    anchorHref = parent->anchorHref;
    // makes no sense to inherit that property, a named anchor is a single point
    // in the document, which is set by the DocumentFragment
    //anchorName = parent->anchorName;
    wsm = parent->wsm;

    // initialize remaining properties
    margin[QTextHtmlParser::MarginLeft] = 0;
    margin[QTextHtmlParser::MarginRight] = 0;
    margin[QTextHtmlParser::MarginTop] = 0;
    margin[QTextHtmlParser::MarginBottom] = 0;
    cssFloat = QTextFrameFormat::InFlow;

    const int oldFontPointSize = fontPointSize;

    // set element specific attributes
    switch (id) {
        case Html_a:
            isAnchor = true;
            break;
        case Html_em:
        case Html_i:
            fontItalic = true;
            break;
        case Html_big:
            fontPointSize = scaleFontPointSize(fontPointSize, -1 /*logical*/, 1 /*step*/);
            break;
        case Html_small:
            fontPointSize = scaleFontPointSize(fontPointSize, -1 /*logical*/, -1 /*step*/);
            break;
        case Html_strong:
        case Html_b:
            fontWeight = QFont::Bold;
            break;
        case Html_h1:
            fontWeight = QFont::Bold;
            fontPointSize = scaleFontPointSize(DefaultFontSize, 6);
            margin[QTextHtmlParser::MarginTop] = 18;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h2:
            fontWeight = QFont::Bold;
            fontPointSize = scaleFontPointSize(DefaultFontSize, 5);
            margin[QTextHtmlParser::MarginTop] = 16;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h3:
            fontWeight = QFont::Bold;
            fontPointSize = scaleFontPointSize(DefaultFontSize, 4);
            margin[QTextHtmlParser::MarginTop] = 14;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h4:
            fontWeight = QFont::Bold;
            fontPointSize = scaleFontPointSize(DefaultFontSize, 3);
            margin[QTextHtmlParser::MarginTop] = 12;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h5:
            fontWeight = QFont::Bold;
            fontPointSize = scaleFontPointSize(DefaultFontSize, 2);
            margin[QTextHtmlParser::MarginTop] = 12;
            margin[QTextHtmlParser::MarginBottom] = 4;
            break;
        case Html_p:
            margin[QTextHtmlParser::MarginTop] = 12;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_center:
            alignment = Qt::AlignCenter;
            break;
        case Html_ul:
            listStyle = QTextListFormat::ListDisc;
            // nested lists don't have margins, except for the toplevel one
            if (!isNestedList(parser)) {
                margin[QTextHtmlParser::MarginTop] = 12;
                margin[QTextHtmlParser::MarginBottom] = 12;
            }
            // no left margin as we use indenting instead
            break;
        case Html_ol:
            listStyle = QTextListFormat::ListDecimal;
            // nested lists don't have margins, except for the toplevel one
            if (!isNestedList(parser)) {
                margin[QTextHtmlParser::MarginTop] = 12;
                margin[QTextHtmlParser::MarginBottom] = 12;
            }
            // no left margin as we use indenting instead
            break;
        case Html_code:
        case Html_tt:
            fontFamily = QString::fromLatin1("Courier New,courier");
            break;
        case Html_br:
            text = QChar(QChar::LineSeparator);
            break;
        // ##### sub / sup
        case Html_pre:
            fontFamily = QString::fromLatin1("Courier New,courier");
            wsm = WhiteSpacePre;
            margin[QTextHtmlParser::MarginTop] = 12;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_blockquote:
            margin[QTextHtmlParser::MarginLeft] = 40;
            margin[QTextHtmlParser::MarginRight] = 40;
            break;
        case Html_dl:
            margin[QTextHtmlParser::MarginTop] = 8;
            margin[QTextHtmlParser::MarginBottom] = 8;
            break;
        case Html_dd:
            margin[QTextHtmlParser::MarginLeft] = 30;
            break;
        case Html_u:
            fontUnderline = true;
            break;
        case Html_s:
            fontStrikeOut = true;
            break;
        case Html_nobr:
            wsm = WhiteSpaceNoWrap;
            break;
        case Html_th:
            fontWeight = QFont::Bold;
            alignment = Qt::AlignCenter;
            break;
        case Html_td:
            alignment = Qt::AlignLeft;
            break;
        case Html_sub:
            verticalAlignment = QTextCharFormat::AlignSubScript;
            break;
        case Html_sup:
            verticalAlignment = QTextCharFormat::AlignSuperScript;
            break;
        default: break;
    }

    if (fontPointSize != oldFontPointSize)
        hasFontPointSize = true;
}

static bool setIntAttribute(int *destination, const QString &value)
{
    bool ok = false;
    int val = value.toInt(&ok);
    if (ok)
        *destination = val;

    return ok;
}

static void setWidthAttribute(QTextLength *width, QString value)
{
    int intVal;
    bool ok = false;
    intVal = value.toInt(&ok);
    if (ok) {
        *width = QTextLength(QTextLength::FixedLength, intVal);
    } else {
        value = value.trimmed();
        if (!value.isEmpty() && value.at(value.length() - 1) == QLatin1Char('%')) {
            value.chop(1);
            intVal = value.toInt(&ok);
            if (ok)
                *width = QTextLength(QTextLength::PercentageLength, intVal);
        }
    }
}

static QTextHtmlParserNode::WhiteSpaceMode stringToWhiteSpaceMode(const QString &s)
{
    if (s == QLatin1String("normal"))
        return QTextHtmlParserNode::WhiteSpaceNormal;
    else if (s == QLatin1String("pre"))
        return QTextHtmlParserNode::WhiteSpacePre;
    else if (s == QLatin1String("nowrap"))
        return QTextHtmlParserNode::WhiteSpaceNoWrap;
    else if (s == QLatin1String("pre-wrap"))
        return QTextHtmlParserNode::WhiteSpacePreWrap;

    return QTextHtmlParserNode::WhiteSpaceModeUndefined;
}

void QTextHtmlParser::parseAttributes()
{
    // local state variable for qt3 textedit mode
    bool seenQt3Richtext = false;

    QTextHtmlParserNode *node = &nodes.last();
    while (pos < len) {
        eatSpace();
        if (hasPrefix(QLatin1Char('>')) || hasPrefix(QLatin1Char('/')))
            break;
        QString key = parseWord().toLower();
        QString value = QLatin1String("1");
        if (key.size() == 0)
            break;
        eatSpace();
        if (hasPrefix(QLatin1Char('='))){
            pos++;
            eatSpace();
            value = parseWord();
        }
        if (value.size() == 0)
            continue;
        if (node->id == Html_font) {
            // the infamous font tag
            if (key == QLatin1String("size") && value.size()) {
                int n = value.toInt();
                if (value.at(0) == QLatin1Char('+') || value.at(0) == QLatin1Char('-'))
                    n += 3;
                node->fontPointSize = scaleFontPointSize(DefaultFontSize, n);
                node->hasFontPointSize = true;
            } else if (key == QLatin1String("face")) {
                node->fontFamily = value;
            } else if (key == QLatin1String("color")) {
                node->color.setNamedColor(value);
            }
        } else if (node->id == Html_ol
                   || node->id == Html_ul) {
            if (key == QLatin1String("type")) {
                node->hasOwnListStyle = true;
                if (value == QLatin1String("1")) {
                    node->listStyle = QTextListFormat::ListDecimal;
                } else if (value == QLatin1String("a")) {
                    node->listStyle = QTextListFormat::ListLowerAlpha;
                } else if (value == QLatin1String("A")) {
                    node->listStyle = QTextListFormat::ListUpperAlpha;
                } else {
                    value = value.toLower();
                    if (value == QLatin1String("square"))
                        node->listStyle = QTextListFormat::ListSquare;
                    else if (value == QLatin1String("disc"))
                        node->listStyle = QTextListFormat::ListDisc;
                    else if (value == QLatin1String("circle"))
                        node->listStyle = QTextListFormat::ListCircle;
                }
            }
        } else if (node->isAnchor) {
            if (key == QLatin1String("href"))
                node->anchorHref = value;
            else if (key == QLatin1String("name"))
                node->anchorName = value;
        } else if (node->id == Html_img) {
            if (key == QLatin1String("src") || key == QLatin1String("source")) {
                node->imageName = value;
            } else if (key == QLatin1String("width")) {
                setIntAttribute(&node->imageWidth, value);
            } else if (key == QLatin1String("height")) {
                setIntAttribute(&node->imageHeight, value);
            }
        } else if (node->id == Html_tr || node->id == Html_body) {
            if (key == QLatin1String("bgcolor"))
                node->bgColor.setNamedColor(value);
        } else if (node->isTableCell) {
            if (key == QLatin1String("width")) {
                setWidthAttribute(&node->width, value);
            } else if (key == QLatin1String("bgcolor")) {
                node->bgColor.setNamedColor(value);
            } else if (key == QLatin1String("rowspan")) {
                setIntAttribute(&node->tableCellRowSpan, value);
            } else if (key == QLatin1String("colspan")) {
                setIntAttribute(&node->tableCellColSpan, value);
            }
        } else if (node->id == Html_table) {
            if (key == QLatin1String("border")) {
                setIntAttribute(&node->tableBorder, value);
            } else if (key == QLatin1String("bgcolor")) {
                node->bgColor.setNamedColor(value);
            } else if (key == QLatin1String("cellspacing")) {
                setIntAttribute(&node->tableCellSpacing, value);
            } else if (key == QLatin1String("cellpadding")) {
                setIntAttribute(&node->tableCellPadding, value);
            } else if (key == QLatin1String("width")) {
                setWidthAttribute(&node->width, value);
            }
        } else if (node->id == Html_meta) {
            if (key == QLatin1String("name")
                && value == QLatin1String("qrichtext")) {
                seenQt3Richtext = true;
            }

            if (key == QLatin1String("content")
                && value == QLatin1String("1")
                && seenQt3Richtext) {

                textEditMode = true;
            }
        }

        if (key == QLatin1String("style")) {
            // style parser taken from Qt 3
            QString a = value;
            int count = a.count(';')+1;
            for (int s = 0; s < count; s++) {
                QString style = a.section(';', s, s).trimmed();
                if (style.startsWith(QLatin1String("font-size:")) && style.endsWith(QLatin1String("pt"))) {
                    node->fontPointSize = int(style.mid(10, style.length() - 12).trimmed().toDouble());
                    node->hasFontPointSize = true;
                } if (style.startsWith(QLatin1String("font-style:"))) {
                    QString s = style.mid(11).trimmed();
                    if (s == QLatin1String("normal"))
                        node->fontItalic = false;
                    else if (s == QLatin1String("italic") || s == QLatin1String("oblique"))
                        node->fontItalic = true;
                } else if (style.startsWith(QLatin1String("font-weight:"))) {
                    QString s = style.mid(12);
                    bool ok = true;
                    int n = s.toInt(&ok);
                    if (ok)
                        node->fontWeight = n/8;
                } else if (style.startsWith(QLatin1String("font-family:"))) {
                    node->fontFamily = style.mid(12).trimmed();
                } else if (style.startsWith(QLatin1String("text-decoration:"))) {
                    QString s = style.mid(16);
                    node->fontUnderline = static_cast<bool>(s.contains("underline"));
                    node->fontOverline = static_cast<bool>(s.contains("overline"));
                    node->fontStrikeOut = static_cast<bool>(s.contains("line-through"));
#if 0
                } else if (style.startsWith(QLatin1String("vertical-align:"))) {
                    QString s = style.mid(15).trimmed();
                    if (s == QLatin1String("sub"))
                        format.setVAlign(QTextFormat::AlignSubScript);
                    else if (s == QLatin1String("super"))
                        format.setVAlign(QTextFormat::AlignSuperScript);
                    else
                        format.setVAlign(QTextFormat::AlignNormal);
#endif
                } else if (style.startsWith(QLatin1String("color:"))) {
                    QString s = style.mid(6).trimmed();
                    if (s.startsWith(QLatin1String("rgb("))
                        && s.at(s.length() - 1) == QLatin1Char(')')) {

                        s.chop(1);
                        s.remove(0, 4);

                        const QStringList rgb = s.split(',');
                        if (rgb.count() == 3)
                            node->color.setRgb(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt());
                        else
                            node->color = QColor();
                    } else {
                        node->color.setNamedColor(style.mid(6));
                    }
                } else if (style.startsWith(QLatin1String("float:"))) {
                    QString s = style.mid(6).trimmed();
                    node->cssFloat = QTextFrameFormat::InFlow;
                    if (s == QLatin1String("left"))
                        node->cssFloat = QTextFrameFormat::FloatLeft;
                    else if (s == QLatin1String("right"))
                        node->cssFloat = QTextFrameFormat::FloatRight;
                } else if (style.startsWith(QLatin1String("-qt-block-indent:"))) {
                    const QString s = style.mid(17).trimmed();
                    if (setIntAttribute(&node->cssBlockIndent, s))
                        node->hasCssBlockIndent = true;
                } else if (style.startsWith(QLatin1String("text-indent:")) && style.endsWith(QLatin1String("px"))) {
                    node->text_indent = style.mid(12, style.length() - 14).trimmed().toDouble();
                } else if (style.startsWith(QLatin1String("-qt-list-indent:"))) {
                    const QString s = style.mid(16).trimmed();
                    if (setIntAttribute(&node->cssListIndent, s)) {
                        node->hasCssListIndent = true;
                    }
                } else if (style.startsWith(QLatin1String("-qt-paragraph-type:"))) {
                    const QString s = style.mid(19).trimmed().toLower();
                    if (s == QLatin1String("empty"))
                        node->isEmptyParagraph = true;
                } else if (style.startsWith(QLatin1String("white-space:"))) {
                    const QString s = style.mid(12).trimmed().toLower();
                    QTextHtmlParserNode::WhiteSpaceMode ws = stringToWhiteSpaceMode(s);
                    if (ws != QTextHtmlParserNode::WhiteSpaceModeUndefined)
                        node->wsm = ws;
                } else if (style.startsWith(QLatin1String("margin-top:")) && style.endsWith("px")) {
                    const QString s = style.mid(11, style.length() - 13).trimmed();
                    setIntAttribute(&node->margin[MarginTop], s);
                } else if (style.startsWith(QLatin1String("margin-bottom:")) && style.endsWith("px")) {
                    const QString s = style.mid(14, style.length() - 16).trimmed();
                    setIntAttribute(&node->margin[MarginBottom], s);
                } else if (style.startsWith(QLatin1String("margin-left:")) && style.endsWith("px")) {
                    const QString s = style.mid(12, style.length() - 14).trimmed();
                    setIntAttribute(&node->margin[MarginLeft], s);
                } else if (style.startsWith(QLatin1String("margin-right:")) && style.endsWith("px")) {
                    const QString s = style.mid(13, style.length() - 15).trimmed();
                    setIntAttribute(&node->margin[MarginRight], s);
                } else if (style.startsWith(QLatin1String("vertical-align:"))) {
                    const QString s = style.mid(15, style.length() - 15).trimmed();
                    if (s == "sub")
                        node->verticalAlignment = QTextCharFormat::AlignSubScript;
                    else if (s == "super")
                        node->verticalAlignment = QTextCharFormat::AlignSuperScript;
                    else
                        node->verticalAlignment = QTextCharFormat::AlignNormal;
                }
            }
        } else if (key == QLatin1String("align")) {
            if (value == QLatin1String("left"))
                node->alignment = Qt::AlignLeft|Qt::AlignAbsolute;
            else if (value == QLatin1String("right"))
                node->alignment = Qt::AlignRight|Qt::AlignAbsolute;
            else if (value == QLatin1String("center"))
                node->alignment = Qt::AlignHCenter;
            else if (value == QLatin1String("justify"))
                node->alignment = Qt::AlignJustify;

            // HTML4 compat
            if (node->id == Html_img) {
                if (node->alignment == Qt::AlignLeft)
                    node->cssFloat = QTextFrameFormat::FloatLeft;
                else if (node->alignment == Qt::AlignRight)
                    node->cssFloat = QTextFrameFormat::FloatRight;
            }
        } else if (key == QLatin1String("dir")) {
            if (value == QLatin1String("ltr"))
                node->direction = Qt::LeftToRight;
            else if (value == QLatin1String("rtl"))
                node->direction = Qt::RightToLeft;
        }
    }
}

