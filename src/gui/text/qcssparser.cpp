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

#include "qcssparser_p.h"

#include <QDebug>
#include <QColor>
#include <QFont>

#include "qcssscanner.cpp"

using namespace QCss;

const char *Scanner::tokenName(TokenType t)
{
    switch (t) {
        case NONE: return "NONE";
        case S: return "S";
        case CDO: return "CDO";
        case CDC: return "CDC";
        case INCLUDES: return "INCLUDES";
        case DASHMATCH: return "DASHMATCH";
        case LBRACE: return "LBRACE";
        case PLUS: return "PLUS";
        case GREATER: return "GREATER";
        case COMMA: return "COMMA";
        case STRING: return "STRING";
        case INVALID: return "INVALID";
        case IDENT: return "IDENT";
        case HASH: return "HASH";
        case ATKEYWORD_SYM: return "ATKEYWORD_SYM";
        case EXCLAMATION_SYM: return "EXCLAMATION_SYM";
        case LENGTH: return "LENGTH";
        case PERCENTAGE: return "PERCENTAGE";
        case NUMBER: return "NUMBER";
        case FUNCTION: return "FUNCTION";
        case COLON: return "COLON";
        case SEMICOLON: return "SEMICOLON";
        case RBRACE: return "RBRACE";
        case SLASH: return "SLASH";
        case MINUS: return "MINUS";
        case DOT: return "DOT";
        case STAR: return "STAR";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case EQUAL: return "EQUAL";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case OR: return "OR";
    }
    return "";
}

struct QCssKnownValue
{
    const char *name;
    int id;
};

static const QCssKnownValue properties[NumProperties - 1] = {
    { "-qt-block-indent", QtBlockIndent },
    { "-qt-list-indent", QtListIndent },
    { "-qt-paragraph-type", QtParagraphType },
    { "-qt-table-type", QtTableType },
    { "background-color", BackgroundColor },
    { "color", Color },
    { "float", Float },
    { "font", Font },
    { "font-family", FontFamily },
    { "font-size", FontSize },
    { "font-style", FontStyle },
    { "font-weight", FontWeight },
    { "margin-bottom", MarginBottom },
    { "margin-left", MarginLeft },
    { "margin-right", MarginRight },
    { "margin-top", MarginTop },
    { "text-decoration", TextDecoration },
    { "text-indent", TextIndent },
    { "vertical-align", VerticalAlignment },
    { "white-space", Whitespace }
};

static const QCssKnownValue values[NumKnownValues - 1] = {
    { "bold", Value_Bold },
    { "italic", Value_Italic },
    { "large", Value_Large },
    { "left", Value_Left },
    { "line-through", Value_LineThrough },
    { "medium", Value_Medium },
    { "normal", Value_Normal },
    { "nowrap", Value_NoWrap },
    { "oblique", Value_Oblique },
    { "overline", Value_Overline },
    { "pre", Value_Pre },
    { "pre-wrap", Value_PreWrap },
    { "right", Value_Right },
    { "small" , Value_Small },
    { "sub", Value_Sub },
    { "super", Value_Super },
    { "underline", Value_Underline },
    { "x-large", Value_XLarge },
    { "xx-large", Value_XXLarge }
};

static bool operator<(const QString &name, const QCssKnownValue &prop)
{
    return QString::compare(name, QLatin1String(prop.name), Qt::CaseInsensitive) < 0;
}

static bool operator<(const QCssKnownValue &prop, const QString &name)
{
    return QString::compare(QLatin1String(prop.name), name, Qt::CaseInsensitive) < 0;
}

static int findKnownValue(const QString &name, const QCssKnownValue *start, int numValues)
{
    const QCssKnownValue *end = &start[numValues - 1];
    const QCssKnownValue *prop = qBinaryFind(start, end, name);
    if (prop == end)
        return 0;
    return prop->id;
}

QColor Declaration::colorValue() const
{
    if (values.count() != 1)
        return QColor();

    Value v = values.first();
    if (v.type == Value::Identifier || v.type == Value::String) {
        v.variant.convert(QVariant::Color);
        v.type = Value::Color;
    }
    if (v.type == Value::Color)
        return qvariant_cast<QColor>(v.variant);

    if (v.type != Value::Function)
        return QColor();

    QStringList lst = v.variant.toStringList();
    if (lst.count() != 2)
        return QColor();
    
    // function name
    if (lst.at(0).compare(QLatin1String("rgb"), Qt::CaseInsensitive) != 0)
        return QColor();
    
    Parser p(lst.at(1));
    if (!p.testExpr())
        return QColor();
    
    QVector<Value> colorDigits;
    if (!p.parseExpr(&colorDigits))
        return QColor();
    
    if (colorDigits.count() != 5
        || colorDigits.at(1).type != Value::TermOperatorComma
        || colorDigits.at(3).type != Value::TermOperatorComma)
        return QColor();
    
    for (int i = 0; i < 5; i += 2)
        if (colorDigits.at(i).type == Value::Percentage) {
            colorDigits[i].variant = colorDigits.at(i).variant.toDouble() * 255. / 100.;
            colorDigits[i].type = Value::Number;
        }

    return QColor(colorDigits.at(0).variant.toInt(),
                  colorDigits.at(2).variant.toInt(), 
                  colorDigits.at(4).variant.toInt());
}

bool Declaration::realValue(qreal *real, const char *unit) const
{
    if (values.count() != 1)
        return false;
    Value v = values.first();
    if (unit && v.type != Value::Length)
        return false;
    QString s = v.variant.toString();
    if (unit) {
        if (!s.endsWith(QLatin1String(unit), Qt::CaseInsensitive))
            return false;
        s.chop(qstrlen(unit));
    }
    bool ok = false;
    qreal val = s.toDouble(&ok);
    if (ok)
        *real = val;
    return ok;
}

bool Declaration::intValue(int *real, const char *unit) const
{
    if (values.count() != 1)
        return false;
    Value v = values.first();
    if (unit && v.type != Value::Length)
        return false;
    QString s = v.variant.toString();
    if (unit) {
        if (!s.endsWith(QLatin1String(unit), Qt::CaseInsensitive))
            return false;
        s.chop(qstrlen(unit));
    }
    bool ok = false;
    int val = s.toInt(&ok);
    if (ok)
        *real = val;
    return ok;
}

static bool setFontSizeFromValue(Value value, QFont *font, int *fontSizeAdjustment)
{
    if (value.type == Value::KnownIdentifier) {
        bool valid = true;
        switch (value.variant.toInt()) {
            case Value_Small: *fontSizeAdjustment = -1; break;
            case Value_Medium: *fontSizeAdjustment = 0; break;
            case Value_Large: *fontSizeAdjustment = 1; break;
            case Value_XLarge: *fontSizeAdjustment = 2; break;
            case Value_XXLarge: *fontSizeAdjustment = 3; break;
            default: valid = false; break;
        }
        return valid;
    }
    if (value.type != Value::Length)
        return false;
    
    bool valid = false;
    QString s = value.variant.toString();
    if (s.endsWith(QLatin1String("pt"), Qt::CaseInsensitive)) {
        s.chop(2);
        value.variant = s;
        if (value.variant.convert(QVariant::Double)) {
            font->setPointSizeF(value.variant.toDouble());
            valid = true;
        }
    } else if (s.endsWith(QLatin1String("px"), Qt::CaseInsensitive)) {
        s.chop(2);
        value.variant = s;
        if (value.variant.convert(QVariant::Int)) {
            font->setPixelSize(value.variant.toInt());
            valid = true;
        }
    }
    return valid;
}

static bool setFontStyleFromValue(const Value &value, QFont *font)
{
    if (value.type != Value::KnownIdentifier)
        return false ;
    switch (value.variant.toInt()) {
        case Value_Normal: font->setStyle(QFont::StyleNormal); return true;
        case Value_Italic: font->setStyle(QFont::StyleItalic); return true;
        case Value_Oblique: font->setStyle(QFont::StyleOblique); return true;
        default: break;
    }
    return false;
}

static bool setFontWeightFromValue(const Value &value, QFont *font)
{
    if (value.type == Value::KnownIdentifier) {
        switch (value.variant.toInt()) {
            case Value_Normal: font->setWeight(QFont::Normal); return true;
            case Value_Bold: font->setWeight(QFont::Bold); return true;
            default: break;
        }
        return false;
    }
    if (value.type != Value::Number)
        return false;
    font->setWeight(value.variant.toInt() / 8);
    return true;
}

static bool setFontFamilyFromValue(const Value &value, QFont *font)
{
    QString fam = value.variant.toString();
    if (fam.isEmpty()) return false;
    font->setFamily(fam);
    return true;
}

static void setTextDecorationFromValues(const QVector<Value> &values, QFont *font)
{
    font->setUnderline(false);
    font->setOverline(false);
    font->setStrikeOut(false);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).type != Value::KnownIdentifier)
            continue;
        switch (values.at(i).variant.toInt()) {
            case Value_Underline: font->setUnderline(true); break;
            case Value_Overline: font->setOverline(true); break;
            case Value_LineThrough: font->setStrikeOut(true); break;
            default: break;
        }
    }
}

static void parseShorthandFontProperty(const QVector<Value> &values, QFont *font, int *fontSizeAdjustment)
{
    font->setStyle(QFont::StyleNormal);
    font->setWeight(QFont::Normal);
    *fontSizeAdjustment = 0;

    int i = 0;
    while (i < values.count()) {
        if (setFontStyleFromValue(values.at(i), font)
            || setFontWeightFromValue(values.at(i), font))
            ++i;
        else
            break;
    }
    
    if (i < values.count()) {
        setFontSizeFromValue(values.at(i), font, fontSizeAdjustment);
        ++i;
    }
    
    if (i < values.count()) {
        setFontFamilyFromValue(values.at(i), font);
    }
}

void QCss::extractFontProperties(const QVector<Declaration> &declarations, QFont *font, int *fontSizeAdjustment)
{
    *font = QFont();
    *fontSizeAdjustment = -255;
    
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        if (decl.values.isEmpty())
            continue;
        const Value val = decl.values.first();
        switch (decl.propertyId) {
            case FontSize: setFontSizeFromValue(val, font, fontSizeAdjustment); break;
            case FontStyle: setFontStyleFromValue(val, font); break;
            case FontWeight: setFontWeightFromValue(val, font); break;
            case FontFamily: setFontFamilyFromValue(val, font); break;
            case TextDecoration: setTextDecorationFromValues(decl.values, font); break;
            case Font: parseShorthandFontProperty(decl.values, font, fontSizeAdjustment); break;
            default: break;
        }
    }
}

int Selector::specificity() const
{
    int val = 0;
    for (int i = 0; i < basicSelectors.count(); ++i) {
        const BasicSelector &sel = basicSelectors.at(i);
        if (!sel.elementName.isEmpty())
            val += 1;
        
        val += (sel.pseudoClasses.count() + sel.attributeSelectors.count()) * 0x10;
        val += sel.ids.count() * 0x100;
    }
    return val;
}

StyleSelector::~StyleSelector()
{
}

QStringList StyleSelector::nodeIds(NodePtr node) const
{
    return QStringList(attribute(node, QLatin1String("id")));
}

bool StyleSelector::selectorMatches(const Selector &selector, NodePtr node)
{
    if (selector.basicSelectors.isEmpty()) return false;

    if (selector.basicSelectors.first().relationToNext == BasicSelector::NoRelation) {
        if (selector.basicSelectors.count() != 1)
            return false;
        return basicSelectorMatches(selector.basicSelectors.first(), node);
    }
    if (selector.basicSelectors.count() <= 1)
        return false;
    
    int i = selector.basicSelectors.count() - 1;
    node = duplicateNode(node);
    bool match = true;

    BasicSelector sel = selector.basicSelectors.at(i);
    do {
        match = basicSelectorMatches(sel, node);
        if (!match) {
            if (sel.relationToNext == BasicSelector::MatchNextSelectorIfParent)
                break;
        }

        if (match || sel.relationToNext != BasicSelector::MatchNextSelectorIfAncestor)
            --i;

        if (i < 0)
            break;
        
        sel = selector.basicSelectors.at(i);
        if (sel.relationToNext == BasicSelector::MatchNextSelectorIfAncestor
            || sel.relationToNext == BasicSelector::MatchNextSelectorIfParent) {
            
            NodePtr nextParent = parentNode(node);
            freeNode(node);
            node = nextParent;
       } else if (sel.relationToNext == BasicSelector::MatchNextSelectorIfPreceeds) {
            NodePtr previousSibling = previousSiblingNode(node);
            freeNode(node);
            node = previousSibling;
       }
        if (isNullNode(node)) {
            match = false;
            break;
        }
   } while (i >= 0 && (match || sel.relationToNext == BasicSelector::MatchNextSelectorIfAncestor));
    
    freeNode(node);

    return match;
}

bool StyleSelector::basicSelectorMatches(const BasicSelector &sel, NodePtr node)
{
    if (!sel.attributeSelectors.isEmpty()) {
        if (!hasAttributes(node))
            return false;

        for (int i = 0; i < sel.attributeSelectors.count(); ++i) {
            const QCss::AttributeSelector &a = sel.attributeSelectors.at(i);
            if (!hasAttribute(node, a.name))
                return false;

            const QString attrValue = attribute(node, a.name);

            if (a.valueMatchCriterium == QCss::AttributeSelector::MatchContains) {

                QStringList lst = attrValue.split(QLatin1Char(' '));
                if (!lst.contains(a.value))
                    return false;
            } else if (
                (a.valueMatchCriterium == QCss::AttributeSelector::MatchEqual
                 && attrValue != a.value)
                ||
                (a.valueMatchCriterium == QCss::AttributeSelector::MatchBeginsWith
                 && !attrValue.startsWith(a.value))
               )
                return false;
        }
    }
    
    if (!sel.elementName.isEmpty()
        && sel.elementName != nodeName(node))
            return false;

    if (!sel.ids.isEmpty()
        && sel.ids != nodeIds(node))
            return false;

    if (!sel.pseudoClasses.isEmpty())
        return false;

    return true;
}

static inline bool qcss_selectorLessThan(const QPair<int, QCss::StyleRule> &lhs, const QPair<int, QCss::StyleRule> &rhs)
{
    return lhs.first < rhs.first;
}

void StyleSelector::matchRules(NodePtr node, const QVector<StyleRule> &rules, QVector<QPair<int, StyleRule> > *matchingRules)
{
    for (int i = 0; i < rules.count(); ++i) {
        const StyleRule &rule = rules.at(i);
        int maxSpecificity = -1;
        for (int j = 0; j < rule.selectors.count(); ++j) {
            if (selectorMatches(rule.selectors.at(j), node)) {
                maxSpecificity = qMax(maxSpecificity, rule.selectors.at(j).specificity());
            }
        }
        if (maxSpecificity != -1) {
            QPair<int, StyleRule> r;
            r.first = maxSpecificity;
            r.second = rule;
            matchingRules->append(r);
        }
    }
}

QVector<Declaration> StyleSelector::declarationsForNode(NodePtr node)
{
    QVector<Declaration> decls;
    
    QVector<QPair<int, StyleRule> > matchingRules;
    matchRules(node, styleSheet.styleRules, &matchingRules);
    if (!medium.isEmpty()) {
        for (int i = 0; i < styleSheet.mediaRules.count(); ++i) {
            if (styleSheet.mediaRules.at(i).media.contains(medium, Qt::CaseInsensitive)) {
                matchRules(node, styleSheet.mediaRules.at(i).styleRules, &matchingRules);
            }
        }
    }

    // sort by specificity
    qSort(matchingRules.begin(), matchingRules.end(), qcss_selectorLessThan);

    for (int i = 0; i < matchingRules.count(); ++i)
        decls += matchingRules.at(i).second.declarations;
    
    return decls;
}

static inline bool isHexDigit(const char c)
{
    return (c >= '0' && c <= '9')
           || (c >= 'a' && c <= 'f')
           || (c >= 'A' && c <= 'F')
           ;
}         

QString Scanner::preprocess(const QString &input)
{
    QString output = input;

    int i = 0;
    while (i < output.size()) {
        if (output.at(i) == QLatin1Char('\\')) {

            ++i;
            // test for unicode hex escape
            int hexCount = 0;
            const int hexStart = i;
            while (i < output.size()
                   && isHexDigit(output.at(i).toLatin1())
                   && hexCount < 7) {
                ++hexCount;
                ++i;
            }
            if (hexCount == 0)
                continue;

            hexCount = qMin(hexCount, 6);
            bool ok = false;
            ushort code = output.mid(hexStart, hexCount).toUShort(&ok, 16);
            if (ok) {
                output.replace(hexStart - 1, hexCount + 1, QChar(code));
                i = hexStart;
            } else {
                i = hexStart;
            }
        } else {
            ++i;
        }
    }
    return output;
}

int QCssScanner_Generated::handleCommentStart()
{
    while (pos < input.size() - 1) {
        if (input.at(pos) == QLatin1Char('*')
            && input.at(pos + 1) == QLatin1Char('/')) {
            pos += 2;
            break;
        }
        ++pos;
    }
    return S;
}

QVector<Symbol> Scanner::scan(const QString &preprocessedInput)
{
    QVector<Symbol> symbols;
    QCssScanner_Generated scanner(preprocessedInput);
    Symbol sym;
    int tok = scanner.lex();
    while (tok != -1) {
        sym.token = static_cast<TokenType>(tok);
        sym.text = scanner.input;
        sym.start = scanner.lexemStart;
        sym.len = scanner.lexemLength;
        symbols.append(sym);
        tok = scanner.lex();
    }
    return symbols;
}

QString Symbol::lexem() const
{
    QString result;
    result.reserve(len);
    for (int i = 0; i < len; ++i) {
        if (text.at(start + i) == QLatin1Char('\\') && i < len - 1)
            ++i;
        result.append(text.at(start + i));
    }
    return result;
}

Parser::Parser(const QString &css)
{
    symbols = Scanner::scan(Scanner::preprocess(css));
    index = 0;
    errorIndex = -1;
}

bool Parser::parse(StyleSheet *styleSheet)
{
    if (testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("charset"))) {
        if (!next(STRING)) return false;
        if (!next(SEMICOLON)) return false;
    }

    while (test(S) || test(CDO) || test(CDC));

    while (testImport()) {
        ImportRule rule;
        if (!parseImport(&rule)) return false;
        styleSheet->importRules.append(rule);
        while (test(S) || test(CDO) || test(CDC));
    }

    do {
        if (testMedia()) {
            MediaRule rule;
            if (!parseMedia(&rule)) return false;
            styleSheet->mediaRules.append(rule);
        } else if (testPage()) {
            PageRule rule;
            if (!parsePage(&rule)) return false;
            styleSheet->pageRules.append(rule);
        } else if (testRuleset()) {
            StyleRule rule;
            if (!parseRuleset(&rule)) return false;
            styleSheet->styleRules.append(rule);
        } else if (test(ATKEYWORD_SYM)) {
            if (!until(RBRACE)) return false;
        } else {
            return false;
        }
        while (test(S) || test(CDO) || test(CDC));
    } while (hasNext());
    return true;
}

Symbol Parser::errorSymbol()
{
    if (errorIndex == -1) return Symbol();
    return symbols.at(errorIndex);
}

static inline void removeOptionalQuotes(QString *str)
{
    if (!str->startsWith(QLatin1Char('\''))
        && !str->startsWith(QLatin1Char('\"')))
        return;
    str->remove(0, 1);
    str->chop(1);
}

bool Parser::parseImport(ImportRule *importRule)
{
    skipSpace();

    if (test(STRING)) {
        importRule->href = lexem();
    } else {
        if (!testAndParseUri(&importRule->href)) return false;
    }
    removeOptionalQuotes(&importRule->href);

    skipSpace();

    if (testMedium()) {
        if (!parseMedium(&importRule->media)) return false;

        while (test(COMMA)) {
            skipSpace();
            if (!parseNextMedium(&importRule->media)) return false;
        }
    }

    if (!next(SEMICOLON)) return false;

    skipSpace();
    return true;
}

bool Parser::parseMedia(MediaRule *mediaRule)
{
    do {
        skipSpace();
        if (!parseNextMedium(&mediaRule->media)) return false;
    } while (test(COMMA));

    if (!next(LBRACE)) return false;
    skipSpace();

    while (testRuleset()) {
        StyleRule rule;
        if (!parseRuleset(&rule)) return false;
        mediaRule->styleRules.append(rule);
    }

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parseMedium(QStringList *media)
{
    media->append(lexem());
    skipSpace();
    return true;
}

bool Parser::parsePage(PageRule *pageRule)
{
    skipSpace();

    if (testPseudoPage())
        if (!parsePseudoPage(&pageRule->selector)) return false;

    skipSpace();
    if (!next(LBRACE)) return false;

    do {
        skipSpace();
        Declaration decl;
        if (!parseNextDeclaration(&decl)) return false;
        if (!decl.isEmpty())
            pageRule->declarations.append(decl);
    } while (test(SEMICOLON));

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parsePseudoPage(QString *selector)
{
    if (!next(IDENT)) return false;
    *selector = lexem();
    return true;
}

bool Parser::parseNextOperator(Value *value)
{
    if (!hasNext()) return true;
    switch (next()) {
        case SLASH: value->type = Value::TermOperatorSlash; skipSpace(); break;
        case COMMA: value->type = Value::TermOperatorComma; skipSpace(); break;
        default: prev(); break;
    }
    return true;
}

bool Parser::parseCombinator(BasicSelector::Relation *relation)
{
    *relation = BasicSelector::NoRelation;
    if (lookup() == S) {
        *relation = BasicSelector::MatchNextSelectorIfAncestor;
        skipSpace();
    } else {
        prev();
    }
    if (test(PLUS)) {
        *relation = BasicSelector::MatchNextSelectorIfPreceeds;
    } else if (test(GREATER)) {
        *relation = BasicSelector::MatchNextSelectorIfParent;
    }
    skipSpace();
    return true;
}

bool Parser::parseProperty(Declaration *decl)
{
    decl->property = lexem();
    decl->propertyId = static_cast<Property>(findKnownValue(decl->property, properties, NumProperties));
    skipSpace();
    return true;
}

bool Parser::parseRuleset(StyleRule *styleRule)
{
    Selector sel;
    if (!parseSelector(&sel)) return false;
    styleRule->selectors.append(sel);

    while (test(COMMA)) {
        skipSpace();
        Selector sel;
        if (!parseNextSelector(&sel)) return false;
        styleRule->selectors.append(sel);
    }

    skipSpace();
    if (!next(LBRACE)) return false;
    const int declarationStart = index;

    do {
        skipSpace();
        Declaration decl;
        const int rewind = index;
        if (!parseNextDeclaration(&decl)) {
            index = rewind;
            const bool foundSemicolon = until(SEMICOLON);
            const int semicolonIndex = index;

            index = declarationStart;
            const bool foundRBrace = until(RBRACE);

            if (foundSemicolon && semicolonIndex < index) {
                decl = Declaration();
                index = semicolonIndex - 1;
            } else {
                skipSpace();
                return foundRBrace;
            }
        }
        if (!decl.isEmpty())
            styleRule->declarations.append(decl);
    } while (test(SEMICOLON));

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parseSelector(Selector *sel)
{
    BasicSelector basicSel;
    if (!parseSimpleSelector(&basicSel)) return false;
    while (testCombinator()) {
        if (!parseCombinator(&basicSel.relationToNext)) return false;

        if (!testSimpleSelector()) break;
        sel->basicSelectors.append(basicSel);

        basicSel = BasicSelector();
        if (!parseSimpleSelector(&basicSel)) return false;
    }
    sel->basicSelectors.append(basicSel);
    return true;
}

bool Parser::parseSimpleSelector(BasicSelector *basicSel)
{
    int minCount = 0;
    if (lookupElementName()) {
        if (!parseElementName(&basicSel->elementName)) return false;
    } else {
        prev();
        minCount = 1;
    }
    bool onceMore;
    int count = 0;
    do {
        onceMore = false;
        if (test(HASH)) {
            QString id = lexem();
            // chop off leading #
            id.remove(0, 1);
            basicSel->ids.append(id);
            onceMore = true;
        } else if (testClass()) {
            onceMore = true;
            AttributeSelector a;
            a.name = QLatin1String("class");
            a.valueMatchCriterium = AttributeSelector::MatchContains;
            if (!parseClass(&a.value)) return false;
            basicSel->attributeSelectors.append(a);
        } else if (testAttrib()) {
            onceMore = true;
            AttributeSelector a;
            if (!parseAttrib(&a)) return false;
            basicSel->attributeSelectors.append(a);
        } else if (testPseudo()) {
            onceMore = true;
            PseudoClass ps;
            if (!parsePseudo(&ps)) return false;
            basicSel->pseudoClasses.append(ps);
        }
        if (onceMore) ++count;
    } while (onceMore);
    return count >= minCount;
}

bool Parser::parseClass(QString *name)
{
    if (!next(IDENT)) return false;
    *name = lexem();
    return true;
}

bool Parser::parseElementName(QString *name)
{
    switch (lookup()) {
        case STAR: name->clear(); break;
        case IDENT: *name = lexem(); break;
        default: return false;
    }
    return true;
}

bool Parser::parseAttrib(AttributeSelector *attr)
{
    skipSpace();
    if (!next(IDENT)) return false;
    attr->name = lexem();
    skipSpace();

    if (test(EQUAL)) {
        attr->valueMatchCriterium = AttributeSelector::MatchEqual;
    } else if (test(INCLUDES)) {
        attr->valueMatchCriterium = AttributeSelector::MatchContains;
    } else if (test(DASHMATCH)) {
        attr->valueMatchCriterium = AttributeSelector::MatchBeginsWith;
    } else {
        return next(RBRACKET);
    }

    skipSpace();

    if (!test(IDENT) && !test(STRING)) return false;
    attr->value = unquotedLexem();

    skipSpace();
    return next(RBRACKET);
}

bool Parser::parsePseudo(PseudoClass *pseudo)
{
    if (test(IDENT)) {
        pseudo->name = lexem();
        return true;
    }
    if (!next(FUNCTION)) return false;
    pseudo->function = lexem();
    // chop off trailing parenthesis
    pseudo->function.chop(1);
    skipSpace();
    if (!test(IDENT)) return false;
    pseudo->name = lexem();
    skipSpace();
    return next(RPAREN);
}

bool Parser::parseNextDeclaration(Declaration *decl)
{
    if (!testProperty())
        return true; // not an error!
    if (!parseProperty(decl)) return false;
    if (!next(COLON)) return false;
    skipSpace();
    if (!parseNextExpr(&decl->values)) return false;
    if (testPrio())
        if (!parsePrio(decl)) return false;
    return true;
}

bool Parser::testPrio()
{
    const int rewind = index;
    if (!test(EXCLAMATION_SYM)) return false;
    skipSpace();
    if (!test(IDENT)) {
        index = rewind;
        return false;
    }
    if (lexem().compare(QLatin1String("important"), Qt::CaseInsensitive) != 0) {
        index = rewind;
        return false;
    }
    return true;
}

bool Parser::parsePrio(Declaration *declaration)
{
    declaration->important = true;
    skipSpace();
    return true;
}

bool Parser::parseExpr(QVector<Value> *values)
{
    Value val;
    if (!parseTerm(&val)) return false;
    values->append(val);
    bool onceMore;
    do {
        onceMore = false;
        val = Value();
        if (!parseNextOperator(&val)) return false;
        if (val.type != QCss::Value::Unknown)
            values->append(val);
        if (testTerm()) {
            onceMore = true;
            val = Value();
            if (!parseTerm(&val)) return false;
            values->append(val);
        }
    } while (onceMore);
    return true;
}

bool Parser::testTerm()
{
    return test(PLUS) || test(MINUS)
           || test(NUMBER)
           || test(PERCENTAGE)
           || test(LENGTH)
           || test(STRING)
           || test(IDENT)
           || testHexColor()
           || testFunction();
}

bool Parser::parseTerm(Value *value)
{
    QString str = lexem();
    bool haveUnary = false;
    if (lookup() == PLUS || lookup() == MINUS) {
        haveUnary = true;
        if (!hasNext()) return false;
        next();
        str += lexem();
    }

    value->variant = str;
    value->type = QCss::Value::String;
    switch (lookup()) {
        case NUMBER:
            value->type = Value::Number;
            value->variant.convert(QVariant::Double);
            break;
        case PERCENTAGE:
            value->type = Value::Percentage;
            str.chop(1); // strip off %
            value->variant = str;
            break;
        case LENGTH:
            value->type = Value::Length;
            break;

        case STRING:
            if (haveUnary) return false;
            value->type = Value::String;
            str.chop(1);
            str.remove(0, 1);
            value->variant = str;
            break;
        case IDENT: {
            if (haveUnary) return false;
            value->type = Value::Identifier;
            const int id = findKnownValue(str, values, NumKnownValues);
            if (id != 0) {
                value->type = Value::KnownIdentifier;
                value->variant = id;
            }
            break;
        }
        default: {
            if (haveUnary) return false;
            prev();
            if (testHexColor()) {
                QColor col;
                if (!parseHexColor(&col)) return false;
                value->type = Value::Color;
                value->variant = col;
            } else if (testFunction()) {
                QString name, args;
                if (!parseFunction(&name, &args)) return false;
                if (name == QLatin1String("url")) {
                    value->type = Value::Uri;
                    removeOptionalQuotes(&args);
                    value->variant = args;
                } else {
                    value->type = Value::Function;
                    value->variant = QStringList() << name << args;
                }
            } else {
                return recordError();
            }
            return true;
        }
    }
    skipSpace();
    return true;
}

bool Parser::parseFunction(QString *name, QString *args)
{
    *name = lexem();
    name->chop(1);
    skipSpace();
    const int start = index;
    if (!until(RPAREN)) return false;
    for (int i = start; i < index - 1; ++i)
        args->append(symbols.at(i).lexem());
    /*
    if (!nextExpr(&arguments)) return false;
    if (!next(RPAREN)) return false;
    */
    skipSpace();
    return true;
}

bool Parser::parseHexColor(QColor *col)
{
    col->setNamedColor(lexem());
    if (!col->isValid()) return false;
    skipSpace();
    return true;
}

bool Parser::testAndParseUri(QString *uri)
{
    const int rewind = index;
    if (!testFunction()) return false;

    QString name, args;
    if (!parseFunction(&name, &args)) {
        index = rewind;
        return false;
    }
    if (name.toLower() != QLatin1String("url")) {
        index = rewind;
        return false;
    }
    *uri = args;
    removeOptionalQuotes(uri);
    return true;
}

bool Parser::testSimpleSelector()
{
    return testElementName()
           || (test(HASH))
           || testClass()
           || testAttrib()
           || testPseudo();
}

bool Parser::next(TokenType t)
{
    if (hasNext() && next() == t)
        return true;
    return recordError();
}

bool Parser::test(TokenType t)
{
    if (index >= symbols.count())
        return false;
    if (symbols.at(index).token == t) {
        ++index;
        return true;
    }
    return false;
}

QString Parser::unquotedLexem() const
{
    QString s = lexem();
    if (lookup() == STRING) {
        s.chop(1);
        s.remove(0, 1);
    }
    return s;
}

QString Parser::lexemUntil(TokenType t)
{
    QString lexem;
    while (hasNext() && next() != t)
        lexem += symbol().lexem();
    return lexem;
}

bool Parser::until(TokenType target, TokenType target2)
{
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    if (index) {
        switch(symbols.at(index-1).token) {
        case LBRACE: ++braceCount; break;
        case LBRACKET: ++brackCount; break;
        case FUNCTION:
        case LPAREN: ++parenCount; break;
        default: ;
        }
    }
    while (index < symbols.size()) {
        TokenType t = symbols.at(index++).token;
        switch (t) {
        case LBRACE: ++braceCount; break;
        case RBRACE: --braceCount; break;
        case LBRACKET: ++brackCount; break;
        case RBRACKET: --brackCount; break;
        case FUNCTION:
        case LPAREN: ++parenCount; break;
        case RPAREN: --parenCount; break;
        default: break;
        }
        if ((t == target || (target2 != NONE && t == target2))
            && braceCount <= 0
            && brackCount <= 0
            && parenCount <= 0)
            return true;

        if (braceCount < 0 || brackCount < 0 || parenCount < 0) {
            --index;
            break;
        }
    }
    return false;
}

bool Parser::testTokenAndEndsWith(TokenType t, const QLatin1String &str)
{
    if (!test(t)) return false;
    if (!lexem().endsWith(str, Qt::CaseInsensitive)) {
        prev();
        return false;
    }
    return true;
}

