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
static const QTextHtmlElement elements[Html_NumElements+1]= {
    { "a",          Html_a,          QTextHtmlElement::DisplayInline },
    { "address",    Html_address,    QTextHtmlElement::DisplayInline },
    { "b",          Html_b,          QTextHtmlElement::DisplayInline },
    { "big",        Html_big,        QTextHtmlElement::DisplayInline },
    { "blockquote", Html_blockquote, QTextHtmlElement::DisplayBlock },
    { "body",       Html_body,       QTextHtmlElement::DisplayBlock },
    { "br",         Html_br,         QTextHtmlElement::DisplayInline },
    { "center",     Html_center,     QTextHtmlElement::DisplayBlock },
    { "cite",       Html_cite,       QTextHtmlElement::DisplayInline },
    { "code",       Html_code,       QTextHtmlElement::DisplayInline },
    { "dd",         Html_dd,         QTextHtmlElement::DisplayBlock },
    { "dfn",        Html_dfn,        QTextHtmlElement::DisplayInline },
    { "div",        Html_div,        QTextHtmlElement::DisplayBlock },
    { "dl",         Html_dl,         QTextHtmlElement::DisplayBlock },
    { "dt",         Html_dt,         QTextHtmlElement::DisplayBlock },
    { "em",         Html_em,         QTextHtmlElement::DisplayInline },
    { "font",       Html_font,       QTextHtmlElement::DisplayInline },
    { "h1",         Html_h1,         QTextHtmlElement::DisplayBlock },
    { "h2",         Html_h2,         QTextHtmlElement::DisplayBlock },
    { "h3",         Html_h3,         QTextHtmlElement::DisplayBlock },
    { "h4",         Html_h4,         QTextHtmlElement::DisplayBlock },
    { "h5",         Html_h5,         QTextHtmlElement::DisplayBlock },
    { "h6",         Html_h6,         QTextHtmlElement::DisplayBlock },
    { "head",       Html_head,       QTextHtmlElement::DisplayNone },
    { "hr",         Html_hr,         QTextHtmlElement::DisplayBlock },
    { "html",       Html_html,       QTextHtmlElement::DisplayInline },
    { "i",          Html_i,          QTextHtmlElement::DisplayInline },
    { "img",        Html_img,        QTextHtmlElement::DisplayInline },
    { "kbd",        Html_kbd,        QTextHtmlElement::DisplayInline },
    { "li",         Html_li,         QTextHtmlElement::DisplayBlock },
    { "meta",       Html_meta,       QTextHtmlElement::DisplayNone },
    { "nobr",       Html_nobr,       QTextHtmlElement::DisplayInline },
    { "ol",         Html_ol,         QTextHtmlElement::DisplayBlock },
    { "p",          Html_p,          QTextHtmlElement::DisplayBlock },
    { "pre",        Html_pre,        QTextHtmlElement::DisplayBlock },
    { "qt",         Html_body /*deliberate mapping*/, QTextHtmlElement::DisplayBlock },
    { "s",          Html_s,          QTextHtmlElement::DisplayInline },
    { "samp",       Html_samp,       QTextHtmlElement::DisplayInline },
    { "small",      Html_small,      QTextHtmlElement::DisplayInline },
    { "span",       Html_span,       QTextHtmlElement::DisplayInline },
    { "strong",     Html_strong,     QTextHtmlElement::DisplayInline },
    { "style",      Html_style,      QTextHtmlElement::DisplayNone },
    { "sub",        Html_sub,        QTextHtmlElement::DisplayInline },
    { "sup",        Html_sup,        QTextHtmlElement::DisplayInline },
    { "table",      Html_table,      QTextHtmlElement::DisplayTable },
    { "tbody",      Html_tbody,      QTextHtmlElement::DisplayTable },
    { "td",         Html_td,         QTextHtmlElement::DisplayBlock },
    { "tfoot",      Html_tfoot,      QTextHtmlElement::DisplayTable },
    { "th",         Html_th,         QTextHtmlElement::DisplayBlock },
    { "thead",      Html_thead,      QTextHtmlElement::DisplayTable },
    { "title",      Html_title,      QTextHtmlElement::DisplayNone },
    { "tr",         Html_tr,         QTextHtmlElement::DisplayTable },
    { "tt",         Html_tt,         QTextHtmlElement::DisplayInline },
    { "u",          Html_u,          QTextHtmlElement::DisplayInline },
    { "ul",         Html_ul,         QTextHtmlElement::DisplayBlock },
    { "var",        Html_var,        QTextHtmlElement::DisplayInline },
    { 0, 0,         QTextHtmlElement::DisplayNone }
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

// quotes newlines as "\\n"
static QString quoteNewline(const QString &s)
{
    QString n = s;
    n.replace('\n', "\\n");
    return n;
}

QTextHtmlParserNode::QTextHtmlParserNode()
    : parent(0), id(-1), isBlock(false), isListItem(false), isListStart(false), isTableCell(false), isAnchor(false),
      fontItalic(Unspecified), fontUnderline(Unspecified), fontOverline(Unspecified), fontStrikeOut(Unspecified), fontFixedPitch(Unspecified),
      cssFloat(QTextFrameFormat::InFlow), hasOwnListStyle(false), hasFontPointSize(false), hasFontPixelSize(false), hasFontSizeAdjustment(false),
      hasCssBlockIndent(false), hasCssListIndent(false), isEmptyParagraph(false), isTextFrame(false), direction(3),
      displayMode(QTextHtmlElement::DisplayInline), fontPointSize(-1), fontPixelSize(-1), fontSizeAdjustment(0),
      fontWeight(-1), alignment(0), verticalAlignment(QTextCharFormat::AlignNormal),
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

    if (fontItalic != Unspecified) {
        format.setFontItalic(fontItalic == On);
    }
    if (fontUnderline != Unspecified) {
        format.setFontUnderline(fontUnderline == On);
    }
    if (fontOverline != Unspecified) {
        format.setFontOverline(fontOverline == On);
    }
    if (fontStrikeOut != Unspecified) {
        format.setFontStrikeOut(fontStrikeOut == On);
    }
    if (fontFixedPitch != Unspecified) {
        format.setFontFixedPitch(fontFixedPitch == On);
    }
    if (fontFamily.size())
        format.setFontFamily(fontFamily);

    if (hasFontPointSize)
        format.setFontPointSize(fontPointSize);
    else if (hasFontPixelSize)
        format.setProperty(QTextFormat::FontPixelSize, fontPixelSize);

    if (hasFontSizeAdjustment)
        format.setProperty(QTextFormat::FontSizeAdjustment, fontSizeAdjustment);
    if (fontWeight > 0)
        format.setFontWeight(fontWeight);
    if (color.isValid())
        format.setForeground(QBrush(color));
    if (bgColor.isValid())
        format.setBackground(QBrush(bgColor));
    if (verticalAlignment != QTextCharFormat::AlignNormal)
        format.setVerticalAlignment(verticalAlignment);
    if (isAnchor) {
        format.setAnchor(true);
        format.setAnchorHref(anchorHref);
        format.setAnchorName(anchorName);
    }

    return format;
}

QTextBlockFormat QTextHtmlParserNode::blockFormat() const
{
    QTextBlockFormat format;

    if (alignment)
        format.setAlignment(alignment);
    if (direction < 2)
        format.setLayoutDirection(Qt::LayoutDirection(direction));

    if (hasCssBlockIndent)
        format.setIndent(cssBlockIndent);
    if (text_indent != 0.)
        format.setTextIndent(text_indent);

    return format;
}

void QTextHtmlParser::dumpHtml()
{
    for (int i = 0; i < count(); ++i) {
        qDebug().nospace() << qPrintable(QString(depth(i)*4, ' '))
                           << qPrintable(at(i).tag) << ":"
                           << quoteNewline(at(i).text);
            ;
    }
}

QTextHtmlParserNode *QTextHtmlParser::newNode(int parent)
{
    QTextHtmlParserNode *lastNode = &nodes.last();
    QTextHtmlParserNode *newNode = 0;

    bool reuseLastNode = true;

    if (nodes.count() == 1) {
        reuseLastNode = false;
    } else if (lastNode->tag.isEmpty()) {

        if (lastNode->text.isEmpty()) {
            reuseLastNode = true;
        } else { // last node is a text node (empty tag) with some text

            if (lastNode->text == QLatin1String(" ")) {

                int lastSibling = count() - 2;
                while (lastSibling
                       && at(lastSibling).parent != lastNode->parent
                       && at(lastSibling).displayMode == QTextHtmlElement::DisplayInline) {
                    lastSibling = at(lastSibling).parent;
                }
                
                if (at(lastSibling).displayMode == QTextHtmlElement::DisplayInline) {
                    reuseLastNode = false;
                } else {
                    reuseLastNode = true;
                }
            } else {
                // text node with real (non-whitespace) text -> nothing to re-use
                reuseLastNode = false;
            }

        }

    } else {
        // last node had a proper tag -> nothing to re-use
        reuseLastNode = false;
    }

    if (reuseLastNode) {
        newNode = lastNode;
        newNode->tag.clear();
        newNode->text.clear();
        newNode->id = -1;
    } else {
        nodes.resize(nodes.size() + 1);
        newNode = &nodes.last();
    }

    newNode->parent = parent;
    return newNode;
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
            if (node->isTableCell)
                break;
            m += node->margin[mar];
            i = node->parent;
        }
    }
    return m;
}

int QTextHtmlParser::topMargin(int i) const
{
    if (!i)
        return 0;
    return at(i).margin[MarginTop];
    // we do margin collapsing in QTextDocumentFragment
#if 0
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
#endif
}

int QTextHtmlParser::bottomMargin(int i) const
{
    if (!i)
        return 0;
    return at(i).margin[MarginBottom];
#if 0
    // we do margin collapsing in QTextDocumentFragment
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
#endif
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
                    if (wsm == QTextHtmlParserNode::WhiteSpaceNoWrap)
                        c = QChar::Nbsp;
                    else
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
        if (nodes.last().wsm != QTextHtmlParserNode::WhiteSpacePre
            && nodes.last().wsm != QTextHtmlParserNode::WhiteSpacePreWrap
            && !textEditMode)
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
        node->displayMode = elem->displayMode;
    } else {
        node->id = -1;
    }

    node->isListItem = (node->id == Html_li);
    node->isListStart = (node->id == Html_ol || node->id == Html_ul);
    node->isTableCell = (node->id == Html_td || node->id == Html_th);

    resolveParent();
    resolveNode();
    QColor inheritedNodeColor = node->color;
    node->color = QColor();
    // _need_ at least one space after the tag name, otherwise there can't be attributes
    if (pos < len && txt.at(pos).isSpace())
        parseAttributes();

    // special handling for anchors with href attribute (hyperlinks)
    if (node->isAnchor && !node->anchorHref.isEmpty()) {
        node->fontUnderline = On; // #### remove 4.2
        if (!node->color.isValid())
            node->color = Qt::blue; // #### remove 4.2
    } else {
        if (!node->color.isValid())
            node->color = inheritedNodeColor;
    }

    // finish tag
    bool tagClosed = false;
    while (pos < len && txt.at(pos) != QLatin1Char('>')) {
        if (txt.at(pos) == QLatin1Char('/')) 
            tagClosed = true;
        

        pos++;
    }
    pos++;

    if (node->wsm != QTextHtmlParserNode::WhiteSpacePre
        && node->wsm != QTextHtmlParserNode::WhiteSpacePreWrap
        && !textEditMode)
        eatSpace();

    if (node->mayNotHaveChildren() || tagClosed) {
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
    if (p > 0
        && at(p - 1).tag == tag
        && at(p - 1).mayNotHaveChildren())
        p--;

    while (p && at(p).tag != tag)
        p = at(p).parent;

    // simply ignore the tag if we can't find
    // a corresponding open node, for broken
    // html such as <font>blah</font></font>
    if (!p)
        return;

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

    // permit invalid html by letting block elements be children
    // of inline elements with the exception of paragraphs:
    //
    // a new paragraph closes parent inline elements (while loop),
    // unless they themselves are children of a non-paragraph block
    // element (if statement)
    //
    // For example:
    //
    // <body><p><b>Foo<p>Bar <-- second <p> implicitly closes <b> that
    //                           belongs to the first <p>. The self-nesting
    //                           check further down prevents the second <p>
    //                           from nesting into the first one then.
    //                           so Bar is not bold.
    //
    // <body><b><p>Foo <-- Foo should be bold.
    //
    // <body><b><p>Foo<p>Bar <-- Foo and Bar should be bold.
    //
    if (node->id == Html_p) {
        while (p && !at(p).isBlock)
            p = at(p).parent;

        if (!p || at(p).id != Html_p)
            p = node->parent;
    }

    // some elements are not self nesting
    if (node->id == at(p).id
        && node->isNotSelfNesting())
        p = at(p).parent;

    // some elements are not allowed in certain contexts
    while (p && !node->allowedInContext(at(p).id)
           // ### make new styles aware of empty tags
           || at(p).mayNotHaveChildren()
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
    fontSizeAdjustment = parent->fontSizeAdjustment;
    hasFontSizeAdjustment = parent->hasFontSizeAdjustment;
    fontWeight = parent->fontWeight;
    color = parent->color;
    verticalAlignment = parent->verticalAlignment;

    if (parent->displayMode == QTextHtmlElement::DisplayNone)
        displayMode = QTextHtmlElement::DisplayNone;

    if (parent->id != Html_table) {
        alignment = parent->alignment;
    }
    // we don't paint per-row background colors, yet. so as an
    // exception inherit the background color here
    if (parent->id == Html_tr && isTableCell) {
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
        case Html_cite:
        case Html_address:
        case Html_var:
        case Html_dfn:
            fontItalic = On;
            break;
        case Html_big:
            fontSizeAdjustment = 1;
            hasFontSizeAdjustment = true;
            break;
        case Html_small:
            fontSizeAdjustment = -1;
            hasFontSizeAdjustment = true;
            break;
        case Html_strong:
        case Html_b:
            fontWeight = QFont::Bold;
            break;
        case Html_h1:
            fontWeight = QFont::Bold;
            fontSizeAdjustment = 3;
            hasFontSizeAdjustment = true;
            margin[QTextHtmlParser::MarginTop] = 18;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h2:
            fontWeight = QFont::Bold;
            fontSizeAdjustment = 2;
            hasFontSizeAdjustment = true;
            margin[QTextHtmlParser::MarginTop] = 16;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h3:
            fontWeight = QFont::Bold;
            fontSizeAdjustment = 1;
            hasFontSizeAdjustment = true;
            margin[QTextHtmlParser::MarginTop] = 14;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h4:
            fontWeight = QFont::Bold;
            fontSizeAdjustment = 0;
            hasFontSizeAdjustment = true;
            margin[QTextHtmlParser::MarginTop] = 12;
            margin[QTextHtmlParser::MarginBottom] = 12;
            break;
        case Html_h5:
            fontWeight = QFont::Bold;
            fontSizeAdjustment = -1;
            hasFontSizeAdjustment = true;
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
        case Html_kbd:
        case Html_samp:
            fontFamily = QString::fromLatin1("Courier New,courier");
            // <tt> uses a fixed font, so set the property
            fontFixedPitch = On;
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
            // <pre> uses a fixed font
            fontFixedPitch = On;
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
            fontUnderline = On;
            break;
        case Html_s:
            fontStrikeOut = On;
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

static bool setFloatAttribute(qreal *destination, const QString &value)
{
    bool ok = false;
    qreal val = value.toDouble(&ok);
    if (ok)
        *destination = val;

    return ok;
}

static void setWidthAttribute(QTextLength *width, QString value)
{
    qreal realVal;
    bool ok = false;
    realVal = value.toDouble(&ok);
    if (ok) {
        *width = QTextLength(QTextLength::FixedLength, realVal);
    } else {
        value = value.trimmed();
        if (!value.isEmpty() && value.at(value.length() - 1) == QLatin1Char('%')) {
            value.chop(1);
            realVal = value.toDouble(&ok);
            if (ok)
                *width = QTextLength(QTextLength::PercentageLength, realVal);
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

static bool parseFontSize(QTextHtmlParserNode *node, QString value)
{
    bool parsed = false;
    value = value.trimmed();
    if (value.endsWith(QLatin1String("pt"))) {
        value = value.left(value.length() - 2);
        qreal real = 0;
        if (setFloatAttribute(&real, value)) {
            node->hasFontPointSize = true;
            node->fontPointSize = qRound(real);
            parsed = true;
        }
    } else if (value.endsWith(QLatin1String("px"))) {
        value = value.left(value.length() - 2);
        if (setIntAttribute(&node->fontPixelSize, value)) {
            node->hasFontPixelSize = true;
            parsed = true;
        }
    } else if (value == QLatin1String("small")) {
        node->hasFontSizeAdjustment = true;
        node->fontSizeAdjustment = -1;
        parsed = true;
    } else if (value == QLatin1String("medium")) {
        node->hasFontSizeAdjustment = true;
        node->fontSizeAdjustment = 0;
        parsed = true;
    } else if (value == QLatin1String("large")) {
        node->hasFontSizeAdjustment = true;
        node->fontSizeAdjustment = 1;
        parsed = true;
    } else if (value == QLatin1String("x-large")) {
        node->hasFontSizeAdjustment = true;
        node->fontSizeAdjustment = 2;
        parsed = true;
    } else if (value == QLatin1String("xx-large")) {
        node->hasFontSizeAdjustment = true;
        node->fontSizeAdjustment = 3;
        parsed = true;
    }
    return parsed;
}

static bool parseFontStyle(QTextHtmlParserNode *node, const QString &value)
{
    bool parsed = false;
    if (value == QLatin1String("normal")) {
        node->fontItalic = Off;
        parsed = true;
    } else if (value == QLatin1String("italic") || value == QLatin1String("oblique")) {
        node->fontItalic = On;
        parsed = true;
    }
    return parsed;
}

static bool parseFontWeight(QTextHtmlParserNode *node, const QString &value)
{
    bool parsed = false;
    if (value == QLatin1String("normal")) {
        node->fontWeight = QFont::Normal;
        parsed = true;
    } else if (value == QLatin1String("bold")) {
        node->fontWeight = QFont::Bold;
        parsed = true;
    } else {
        bool ok = false;
        int n = value.toInt(&ok);
        if (ok) {
            node->fontWeight = n/8;
            parsed = true;
        }
    }
    return parsed;
}

static bool parseFontFamily(QTextHtmlParserNode *node, const QString &value)
{
    node->fontFamily = value.trimmed();
    if ((node->fontFamily.startsWith(QLatin1Char('\''))
         && node->fontFamily.endsWith(QLatin1Char('\'')))
        ||
        (node->fontFamily.startsWith(QLatin1Char('\"'))
         && node->fontFamily.endsWith(QLatin1Char('\"')))
       ) {
        node->fontFamily.remove(0, 1);
        node->fontFamily.chop(1);
    }
    return true;
}

static QColor parseColor(QString value)
{
    QColor color;
    if (value.startsWith(QLatin1String("rgb("))
        && value.endsWith(QLatin1Char(')'))) {

        value.chop(1);
        value.remove(0, 4);

        const QStringList rgb = value.split(',');
        if (rgb.count() == 3)
            color.setRgb(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt());
    } else {
        color.setNamedColor(value);
    }
    return color;
}

// style parser taken from Qt 3
static void parseStyleAttribute(QTextHtmlParserNode *node, const QString &value)
{
    QString a = value;
    int count = a.count(';')+1;
    for (int s = 0; s < count; s++) {
        QString style = a.section(';', s, s).trimmed();
        if (style.startsWith(QLatin1String("font-size:"))) {
            parseFontSize(node, style.mid(10));
        } if (style.startsWith(QLatin1String("font-style:"))) {
            parseFontStyle(node, style.mid(11).trimmed());
        } else if (style.startsWith(QLatin1String("font-weight:"))) {
            parseFontWeight(node, style.mid(12));
        } else if (style.startsWith(QLatin1String("font-family:"))) {
            parseFontFamily(node, style.mid(12));
        } else if (style.startsWith(QLatin1String("text-decoration:"))) {
            QString s = style.mid(16);
            node->fontUnderline = static_cast<bool>(s.contains("underline")) ? On : Off;
            node->fontOverline = static_cast<bool>(s.contains("overline")) ? On : Off;
            node->fontStrikeOut = static_cast<bool>(s.contains("line-through")) ? On : Off;
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
            node->color = parseColor(style.mid(6).trimmed());
        } else if (style.startsWith(QLatin1String("background-color:"))) {
            node->bgColor = parseColor(style.mid(17).trimmed());
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
        } else if (style.startsWith(QLatin1String("-qt-table-type:"))) {
            const QString s = style.mid(15).trimmed().toLower();
            if (s == QLatin1String("frame"))
                node->isTextFrame = true;
        } else if (style.startsWith(QLatin1String("white-space:"))) {
            const QString s = style.mid(12).trimmed().toLower();
            QTextHtmlParserNode::WhiteSpaceMode ws = stringToWhiteSpaceMode(s);
            if (ws != QTextHtmlParserNode::WhiteSpaceModeUndefined)
                node->wsm = ws;
        } else if (style.startsWith(QLatin1String("margin-top:")) && style.endsWith("px")) {
            const QString s = style.mid(11, style.length() - 13).trimmed();
            setIntAttribute(&node->margin[QTextHtmlParser::MarginTop], s);
        } else if (style.startsWith(QLatin1String("margin-bottom:")) && style.endsWith("px")) {
            const QString s = style.mid(14, style.length() - 16).trimmed();
            setIntAttribute(&node->margin[QTextHtmlParser::MarginBottom], s);
        } else if (style.startsWith(QLatin1String("margin-left:")) && style.endsWith("px")) {
            const QString s = style.mid(12, style.length() - 14).trimmed();
            setIntAttribute(&node->margin[QTextHtmlParser::MarginLeft], s);
        } else if (style.startsWith(QLatin1String("margin-right:")) && style.endsWith("px")) {
            const QString s = style.mid(13, style.length() - 15).trimmed();
            setIntAttribute(&node->margin[QTextHtmlParser::MarginRight], s);
        } else if (style.startsWith(QLatin1String("vertical-align:"))) {
            const QString s = style.mid(15, style.length() - 15).trimmed();
            if (s == "sub")
                node->verticalAlignment = QTextCharFormat::AlignSubScript;
            else if (s == "super")
                node->verticalAlignment = QTextCharFormat::AlignSuperScript;
            else
                node->verticalAlignment = QTextCharFormat::AlignNormal;
        } else if (style.startsWith(QLatin1String("font:"))) { // shorthand font property
            // first reset to initial values
            node->fontItalic = false;
            node->fontWeight = QFont::Normal;
            node->hasFontPointSize = false;
            node->fontSizeAdjustment = 0;
            node->fontFamily.clear();

            style = style.mid(5).trimmed();

            QStringList values;
            for (int i = 0; i < style.count(); ++i) {
                if (style.at(i) == QLatin1Char('\"')) {
                    ++i;
                    while (i < style.count() && style.at(i) != QLatin1Char('\"'))
                        ++i;
                } else if (style.at(i) == QLatin1Char('\'')) {
                    ++i;
                    while (i < style.count() && style.at(i) != QLatin1Char('\''))
                        ++i;
                } else if (style.at(i).isSpace()) {
                    values << style.left(i);
                    style.remove(0, i + 1);
                    i = -1;
                }
            }

            if (!style.isEmpty()) {
                values << style;
                style.clear();
            }

            if (values.isEmpty())
                continue;

            bool foundStyleOrWeight = false;
            do {
                const QString val = values.first();

                foundStyleOrWeight = parseFontStyle(node, val);
                if (!foundStyleOrWeight)
                    foundStyleOrWeight = parseFontWeight(node, val);

                if (foundStyleOrWeight)
                    values.removeFirst();
            } while (foundStyleOrWeight && !values.isEmpty());

            if (values.isEmpty())
                continue;

            parseFontSize(node, values.takeFirst());

            if (values.isEmpty())
                continue;

            parseFontFamily(node, values.takeFirst());
        }
    }
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

        switch (node->id) {
            case Html_font:
                // the infamous font tag
                if (key == QLatin1String("size") && value.size()) {
                    int n = value.toInt();
                    if (value.at(0) != QLatin1Char('+') && value.at(0) != QLatin1Char('-'))
                        n -= 3;
                    node->fontSizeAdjustment = n;
                    node->hasFontSizeAdjustment = true;
                } else if (key == QLatin1String("face")) {
                    node->fontFamily = value;
                } else if (key == QLatin1String("color")) {
                    node->color.setNamedColor(value);
                }
                break;
            case Html_ol:
            case Html_ul:
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
                break;
            case Html_a:
                if (key == QLatin1String("href"))
                    node->anchorHref = value;
                else if (key == QLatin1String("name"))
                    node->anchorName = value;
                break;
            case Html_img:
                if (key == QLatin1String("src") || key == QLatin1String("source")) {
                    node->imageName = value;
                } else if (key == QLatin1String("width")) {
                    setFloatAttribute(&node->imageWidth, value);
                } else if (key == QLatin1String("height")) {
                    setFloatAttribute(&node->imageHeight, value);
                }
                break;
            case Html_tr:
            case Html_body:
                if (key == QLatin1String("bgcolor"))
                    node->bgColor.setNamedColor(value);
                break;
            case Html_th:
            case Html_td:
                if (key == QLatin1String("width")) {
                    setWidthAttribute(&node->width, value);
                } else if (key == QLatin1String("bgcolor")) {
                    node->bgColor.setNamedColor(value);
                } else if (key == QLatin1String("rowspan")) {
                    setIntAttribute(&node->tableCellRowSpan, value);
                } else if (key == QLatin1String("colspan")) {
                    setIntAttribute(&node->tableCellColSpan, value);
                }
                break;
            case Html_table:
                if (key == QLatin1String("border")) {
                    setFloatAttribute(&node->tableBorder, value);
                } else if (key == QLatin1String("bgcolor")) {
                    node->bgColor.setNamedColor(value);
                } else if (key == QLatin1String("cellspacing")) {
                    setFloatAttribute(&node->tableCellSpacing, value);
                } else if (key == QLatin1String("cellpadding")) {
                    setFloatAttribute(&node->tableCellPadding, value);
                } else if (key == QLatin1String("width")) {
                    setWidthAttribute(&node->width, value);
                } else if (key == QLatin1String("height")) {
                    setWidthAttribute(&node->height, value);
                }
                break;
            case Html_meta:
                if (key == QLatin1String("name")
                    && value == QLatin1String("qrichtext")) {
                    seenQt3Richtext = true;
                }

                if (key == QLatin1String("content")
                    && value == QLatin1String("1")
                    && seenQt3Richtext) {

                    textEditMode = true;
                }
                break;
            case Html_hr:
                if (key == QLatin1String("width"))
                    setWidthAttribute(&node->width, value);
                break;
            default:
                break;
        }

        if (key == QLatin1String("style")) {
            parseStyleAttribute(node, value);
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

