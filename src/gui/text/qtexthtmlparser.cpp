#include "qtexthtmlparser_p.h"

#include <qbytearray.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdebug.h>
#include <qstylesheet.h>

#include "qtextdocument.h"
#include "qtextglobal_p.h"
#include <private/qtextformat_p.h>
#include <private/qtextpiecetable_p.h>
#include "qtextcursor.h"

#define QChar_linesep QChar(0x2028U) // in QChar maybe?

#define MAX_ENTITY 259
static const struct QTextHtmlEntity { const char *name; Q_UINT16 code; } entities[MAX_ENTITY]= {
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
    const QTextHtmlEntity *end = &entities[MAX_ENTITY];
    const QTextHtmlEntity *e = qBinaryFind(start, end, entity);
    if (!e->name)
        return QChar::null;
    return e->code;
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

static QTextListFormat::Style convertListStyle(QStyleSheetItem::ListStyle style)
{
    switch (style) {
        case QStyleSheetItem::ListDisc: return QTextListFormat::ListDisc;
        case QStyleSheetItem::ListCircle: return QTextListFormat::ListCircle;
        case QStyleSheetItem::ListSquare: return QTextListFormat::ListSquare;
        case QStyleSheetItem::ListDecimal: return QTextListFormat::ListDecimal;
        case QStyleSheetItem::ListLowerAlpha: return QTextListFormat::ListLowerAlpha;
        case QStyleSheetItem::ListUpperAlpha: return QTextListFormat::ListUpperAlpha;
        case QStyleSheetItem::ListStyleUndefined: return QTextListFormat::ListStyleUndefined;
    }
    Q_ASSERT(false);
    return QTextListFormat::ListStyleUndefined;
}

void QTextHtmlParser::dumpHtml()
{
    for (int i = 0; i < count(); ++i) {
        qDebug().nospace() << QString(depth(i)*4, ' ')
                           << at(i).tag << ":"
                           << "\"" << quoteNewline(at(i).text) << "\""
            ;
    }
}

QTextHtmlParserNode *QTextHtmlParser::newNode(int parent)
{
    QTextHtmlParserNode *node = &nodes.last();
    // create new new, or reuse the last one
    if (nodes.count() == 1 || node->tag.size()
        || (node->text.size() && node->text != QLatin1String("\n"))) {
        nodes.resize(nodes.size() + 1);
        node = &nodes.last();
    } else {
        node->tag.clear();
        node->text.clear();
        node->style = 0;
    }
    node->parent = parent;
    return node;
}

void QTextHtmlParser::parse(const QString &text)
{
    // ###
    QStyleSheet::defaultSheet()->item("ul")->setMargin(QStyleSheetItem::MarginLeft, 0);
    QStyleSheet::defaultSheet()->item("ol")->setMargin(QStyleSheetItem::MarginLeft, 0);

    nodes.clear();
    nodes.resize(1);
    txt = text;
    pos = 0;
    len = txt.length();
    nodes[0].style = QStyleSheet::defaultSheet()->item(QLatin1String(""));
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
    if (mar == QStyleSheetItem::MarginLeft
        || mar == QStyleSheetItem::MarginRight
        || mar == QStyleSheetItem::MarginFirstLine) {
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
        m = qMax(m, node->margin[QStyleSheetItem::MarginTop]);

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
        m = qMax(m, node->margin[QStyleSheetItem::MarginTop]);

        // get next block
        while (i+1 < count() && !at(i+1).isBlock)
            ++i;
        if (i && node->parent == at(i).parent)
            break;
        i = node->parent;
    }
    return m;
}

int QTextHtmlParser::findChild(int i, const char *tag) const
{
    int parent = i;
    int grandParent = at(i).parent;
    ++i;
    for (; i < count(); ++i)
        if (at(i).parent == parent) {
            if (at(i).tag == QLatin1String(tag))
                return i;
            continue;
        } else if (i == grandParent)
            return -1;

    return -1;
}

int QTextHtmlParser::findNextChild(int parent, int currentChild) const
{
    QString tag = at(currentChild).tag;
    int grandParent = at(parent).parent;

    int i = currentChild + 1;
    for (; i < count(); ++i)
        if (at(i).parent == parent) {
            if (at(i).tag == tag)
                return i;
            continue;
        } else if (i == grandParent)
            return -1;

    return -1;
}

void QTextHtmlParser::eatSpace()
{
    while (pos < len && txt.at(pos).isSpace() && txt.at(pos) != QTextParagraphSeparator)
        pos++;
}

void QTextHtmlParser::parse() {
    QStyleSheetItem::WhiteSpaceMode wsm = QStyleSheetItem::WhiteSpaceNormal;
    while (pos < len) {
        QChar c = txt.at(pos++);
        if (c == QLatin1Char('<')) {
            parseTag();
            wsm = nodes.last().wsm;
        } else if (c == QLatin1Char('&')) {
            nodes.last().text += parseEntity();
        } else {
            if (c.isSpace() && c != QChar(QChar::nbsp) && c != QTextParagraphSeparator) {
                if (wsm == QStyleSheetItem::WhiteSpacePre) {
                    if (c == QLatin1Char('\n'))
                        c = QChar_linesep;
                } else { // non-pre mode: collapse whitespace except nbsp
                    while (pos < len && txt.at(pos).isSpace()
                           && txt.at(pos) != QChar::nbsp)
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

    // resolve style
    node->style = QStyleSheet::defaultSheet()->item(node->tag);
    if (!node->style)
        node->style = QStyleSheet::defaultSheet()->item("");
    Q_ASSERT(node->style != 0);

    node->isBlock = (node->style->displayMode() != QStyleSheetItem::DisplayInline);
    node->isImage = (node->tag == QLatin1String("img"));
    node->isListItem = (node->style->displayMode() == QStyleSheetItem::DisplayListItem);
    node->isListStart = (node->tag == QLatin1String("ol") || node->tag == QLatin1String("ul"));
    if (node->isListStart)
        node->listStyle = convertListStyle(node->style->listStyle());
    node->isTableCell = (node->tag == QLatin1String("td") || node->tag == QLatin1String("th"));

    resolveParent();
    resolveNode();
    parseAttributes();

    // finish tag
    while (pos < len && txt.at(pos++) != QLatin1Char('>'))
        ;

    if (node->wsm != QStyleSheetItem::WhiteSpacePre)
        eatSpace();
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
QChar QTextHtmlParser::parseEntity()
{
    int recover = pos;
    QString entity;
    while (pos < len) {
        QChar c = txt.at(pos++);
        if (c.isSpace() || pos - recover > 8) {
            pos = recover;
            return QLatin1Char('&');
        }
        if (c == QLatin1Char(';'))
            break;
        entity += c;
    }

    QChar resolved = resolveEntity(entity);
    if (!resolved.isNull())
        return resolved;
    if (entity.length() > 1 && entity.at(0) == QLatin1Char('#')) {
        int num = entity.mid(1).toInt();
        if (num == 151) // ### hack for designer manual
            num = '-';
        return num;
    }
    pos = recover;
    return QLatin1Char('&');
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
    if (node->isBlock)
        while (p && !at(p).isBlock)
            p = at(p).parent;

    // some elements are not self nesting
    if (node->tag == at(p).tag) {
        if (node->style && !node->style->selfNesting())
            p = at(p).parent;
    }

    // some elements are not allowed in certain contexts
    while (p && !node->style->allowedInContext(at(p).style)
           // ### make new styles aware of empty tags
           || at(p).tag == QLatin1String("hr")
           || at(p).tag == QLatin1String("br")
           || at(p).tag == QLatin1String("img")
       ){
        p = at(p).parent;
    }
    node->parent = p;
}

// sets all properties on the new node
void QTextHtmlParser::resolveNode()
{
    QTextHtmlParserNode *node = &nodes.last();
    const QTextHtmlParserNode *parent = &nodes.at(node->parent);
    const QStyleSheetItem *style = node->style;

    // inherit properties from parent element
    node->isAnchor = parent->isAnchor;
    node->fontItalic = parent->fontItalic;
    node->fontUnderline = parent->fontUnderline;
    node->fontOverline = parent->fontOverline;
    node->fontStrikeOut = parent->fontStrikeOut;
    node->fontFixedPitch = parent->fontFixedPitch;
    node->fontFamily = parent->fontFamily;
    node->fontPointSize = parent->fontPointSize;
    node->fontWeight = parent->fontWeight;
    node->color = parent->color;
    node->bgColor = parent->bgColor;
    node->alignment = parent->alignment;
    node->listStyle = parent->listStyle;
    node->anchorHref = parent->anchorHref;
    node->anchorName = parent->anchorName;
    node->wsm = parent->wsm;

    // initialize remaining properties
    node->margin[0] = 0;
    node->margin[1] = 0;
    node->margin[2] = 0;
    node->margin[3] = 0;
    node->margin[4] = 0;
    node->cssFloat = QTextFormat::FloatNone;

    if (node->tag == QLatin1String("br")) {
        node->text = QChar_linesep;
    } else if (node->tag == QLatin1String("a")) {
        node->isAnchor = true;
    }

    if (!node->style)
        return;

    // apply styles settings
    if (style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceModeUndefined)
        node->wsm = style->whiteSpaceMode();
    if (style->definesFontItalic())
        node->fontItalic = style->fontItalic();
    if (style->definesFontUnderline())
        node->fontUnderline = style->fontUnderline();
    if (style->definesFontStrikeOut())
        node->fontStrikeOut = style->fontStrikeOut();
    if (style->fontFamily().size())
        node->fontFamily = style->fontFamily();
    if (style->fontSize() >= 0)
        node->fontPointSize = style->fontSize();
    if (style->fontWeight() >= 0)
        node->fontWeight = style->fontWeight();
    if (style->color().isValid())
        node->color = style->color();
    if (style->alignment() >= 0)
        node->alignment = (Qt::Alignment)style->alignment();
    if (style->listStyle() != QStyleSheetItem::ListStyleUndefined)
        node->listStyle = convertListStyle(style->listStyle());

    if (style->margin(QStyleSheetItem::MarginTop) != -1)
        node->margin[QStyleSheetItem::MarginTop] = style->margin(QStyleSheetItem::MarginTop);
    if (style->margin(QStyleSheetItem::MarginBottom) != -1)
        node->margin[QStyleSheetItem::MarginBottom] = style->margin(QStyleSheetItem::MarginBottom);
    if (style->margin(QStyleSheetItem::MarginLeft) != -1)
        node->margin[QStyleSheetItem::MarginLeft] = style->margin(QStyleSheetItem::MarginLeft);
    if (style->margin(QStyleSheetItem::MarginRight) != -1)
        node->margin[QStyleSheetItem::MarginRight] = style->margin(QStyleSheetItem::MarginRight);
    if (style->margin(QStyleSheetItem::MarginFirstLine) != -1)
        node->margin[QStyleSheetItem::MarginFirstLine] = style->margin(QStyleSheetItem::MarginFirstLine);

    if (style->logicalFontSize() > 0 || style->logicalFontSizeStep())
        node->fontPointSize = scaleFontPointSize(12, style->logicalFontSize(), style->logicalFontSizeStep());
}


void QTextHtmlParser::parseAttributes()
{
    QTextHtmlParserNode *node = &nodes.last();
    while (pos < len) {
        eatSpace();
        if (hasPrefix(QLatin1Char('>')) || hasPrefix(QLatin1Char('/')))
            break;
        QString key = parseWord().toLower();
        QString value = QLatin1String("1");
        if (key.size() == 0)
            continue;
        eatSpace();
        if (hasPrefix(QLatin1Char('='))){
            pos++;
            eatSpace();
            value = parseWord();
        }
        if (value.size() == 0)
            continue;
        if (node->tag == QLatin1String("font")) {
            // the infamous font tag
            if (key == QLatin1String("size") && value.size()) {
                int n = value.toInt();
                if (value.at(0) == QLatin1Char('+') || value.at(0) == QLatin1Char('-'))
                    n += 3;
                node->fontPointSize = scaleFontPointSize(12, n);
            } else if (key == QLatin1String("face")) {
                node->fontFamily = value;
            } else if (key == QLatin1String("color")) {
                node->color.setNamedColor(value);
            }
        } else if (node->tag == QLatin1String("ol")
                   || node->tag == QLatin1String("ul")) {
            if (key == QLatin1String("type")) {
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
        } else if (node->tag == QLatin1String("img")) {
            bool ok = false;
            if (key == QLatin1String("src") || key == QLatin1String("source")) {
                node->imageName = value;
            } else if (key == QLatin1String("width")) {
                node->imageWidth = value.toInt(&ok);
                if (!ok)
                    node->imageWidth = -1;
            } else if (key == QLatin1String("height")) {
                node->imageHeight = value.toInt(&ok);
                if (!ok)
                    node->imageHeight = -1;
            }
        } else if (node->tag == QLatin1String("tr")) {
            if (key == QLatin1String("bgcolor"))
                node->bgColor.setNamedColor(value);
        }

        if (key == QLatin1String("style")) {
            // style parser taken from Qt 3
            QString a = value;
            int count = a.count(';')+1;
            for (int s = 0; s < count; s++) {
                QString style = a.section(';', s, s).trimmed();
                if (style.startsWith(QLatin1String("font-size:")) && style.endsWith(QLatin1String("pt"))) {
                    node->fontPointSize = int(style.mid(10, style.length() - 12).toDouble());
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
                    node->fontFamily = style.mid(12).section(',',0,0);
                } else if (style.startsWith(QLatin1String("text-decoration:"))) {
                    QString s = style.mid(16).trimmed();
                    node->fontUnderline = (s == QLatin1String("underline"));
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
                    node->color.setNamedColor(style.mid(6));
                } else if (style.startsWith(QLatin1String("float:"))) {
                    QString s = style.mid(6).trimmed();
                    node->cssFloat = QTextFormat::FloatNone;
                    if (s == QLatin1String("left"))
                        node->cssFloat = QTextFormat::FloatLeft;
                    else if (s == QLatin1String("right"))
                        node->cssFloat = QTextFormat::FloatRight;
                }
            }
        } else if (key == QLatin1String("align")) {
            if (value == QLatin1String("left"))
                node->alignment = Qt::AlignLeft;
            else if (value == QLatin1String("right"))
                node->alignment = Qt::AlignRight;
            else if (value == QLatin1String("center"))
                node->alignment = Qt::AlignHCenter;
            else if (value == QLatin1String("justify"))
                node->alignment = Qt::AlignJustify;
        }
    }
}

QTextCodec *QTextHtmlParser::codecForStream(const QByteArray &ba)
{
    // determine charset
    int mib = 4; // Latin1
    int pos;
    QTextCodec *c = 0;

    if (ba.size() > 1 && (((uchar)ba[0] == 0xfe && (uchar)ba[1] == 0xff)
                          || ((uchar)ba[0] == 0xff && (uchar)ba[1] == 0xfe))) {
        mib = 1000; // utf16
    } else if (ba.size() > 2
             && (uchar)ba[0] == 0xef
             && (uchar)ba[1] == 0xbb
             && (uchar)ba[2] == 0xbf) {
        mib = 106; // utf-8
    } else if ((pos = ba.indexOf("http-equiv=")) != -1) {
        pos = ba.indexOf("charset=", pos) + strlen("charset=");
        if (pos != -1) {
            int pos2 = ba.indexOf('\"', pos+1);
            QByteArray cs = ba.mid(pos, pos2-pos);
            qDebug("found charset: %s", cs.data());
            c = QTextCodec::codecForName(cs);
        }
    }
    if (!c)
        c = QTextCodec::codecForMib(mib);

    return c;
}

#if 0
QByteArray QTextHtmlFilter::save() const
{
    if (!d->codec)
        d->codec = QTextCodec::codecForMib(106); // utf-8

    QByteArray ba;
    ba.append("<html>\n<head>\n  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=\"");
    ba.append(d->codec->name());
    ba.append("\">\n</head>\n<body>\n");

    d->html = QString();
    if (d->doc)
        d->html = d->doc->text();

    ba.append(d->html);
    ba.append("</body>\n</html>\n");

    return ba;
}
#endif
