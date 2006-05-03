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

#ifndef QCssPARSER_H
#define QCssPARSER_H

#include <QStringList>
#include <QVector>
#include <QVariant>

namespace QCss
{

enum Property {
    UnknownProperty,
    BackgroundColor,
    Color,
    Float,
    Font,
    FontFamily,
    FontSize,
    FontStyle,
    FontWeight,
    MarginBottom,
    MarginLeft,
    MarginRight,
    MarginTop,
    QtBlockIndent,
    QtListIndent,
    QtParagraphType,
    QtTableType,
    TextDecoration,
    TextIndent,
    VerticalAlignment,
    Whitespace,
    NumProperties
};

enum KnownValue {
    UnknownValue,
    Value_Normal,
    Value_Pre,
    Value_NoWrap,
    Value_PreWrap,
    Value_Small,
    Value_Medium,
    Value_Large,
    Value_XLarge,
    Value_XXLarge,
    Value_Italic,
    Value_Oblique,
    Value_Bold,
    Value_Underline,
    Value_Overline,
    Value_LineThrough,
    Value_Sub,
    Value_Super,
    Value_Left,
    Value_Right,
    NumKnownValues
};

struct Q_INTERNAL_EXPORT Value
{
    enum Type {
        Unknown,
        Number,
        Percentage,
        Length,
        String,
        Identifier,
        KnownIdentifier,
        Uri,
        Color,
        Function,
        TermOperatorSlash,
        TermOperatorComma
    };
    inline Value() : type(Unknown) { }
    Type type;
    QVariant variant;
};

struct Q_INTERNAL_EXPORT Declaration
{
    inline Declaration() : propertyId(UnknownProperty), important(false) {}
    QString property;
    Property propertyId;
    inline bool isEmpty() const { return property.isEmpty() && propertyId == UnknownProperty; }
    QVector<Value> values;
    bool important;
    
    // helper functions
    QColor colorValue() const;
    bool realValue(qreal *value, const char *unit = 0) const;
    bool intValue(int *value, const char *unit = 0) const;
};

struct Q_INTERNAL_EXPORT PseudoClass
{
    QString name;
    QString function;
};

struct Q_INTERNAL_EXPORT AttributeSelector
{
    enum ValueMatchType {
        NoMatch,
        MatchEqual,
        MatchContains,
        MatchBeginsWith
    };
    inline AttributeSelector() : valueMatchCriterium(NoMatch) {}

    QString name;
    QString value;
    ValueMatchType valueMatchCriterium;
};

struct Q_INTERNAL_EXPORT BasicSelector
{
    inline BasicSelector() : relationToNext(NoRelation) {}

    enum Relation {
        NoRelation,
        MatchNextSelectorIfAncestor,
        MatchNextSelectorIfParent,
        MatchNextSelectorIfPreceeds
    };

    QString elementName;

    QStringList ids;
    QVector<PseudoClass> pseudoClasses;
    QVector<AttributeSelector> attributeSelectors;

    Relation relationToNext;
};

struct Q_INTERNAL_EXPORT Selector
{
    QVector<BasicSelector> basicSelectors;
    int specificity() const;
};

struct StyleRule;
struct MediaRule;
struct PageRule;
struct ImportRule;

void extractFontProperties(const QVector<Declaration> &declarations, QFont *font, int *fontSizeAdjustment);

struct Q_INTERNAL_EXPORT StyleRule
{
    QVector<Selector> selectors;
    QVector<Declaration> declarations;
};

struct Q_INTERNAL_EXPORT MediaRule
{
    QStringList media;
    QVector<StyleRule> styleRules;
};

struct Q_INTERNAL_EXPORT PageRule
{
    QString selector;
    QVector<Declaration> declarations;
};

struct Q_INTERNAL_EXPORT ImportRule
{
    QString href;
    QStringList media;
};

struct Q_INTERNAL_EXPORT StyleSheet
{
    QVector<StyleRule> styleRules;
    QVector<MediaRule> mediaRules;
    QVector<PageRule> pageRules;
    QVector<ImportRule> importRules;
};

class Q_INTERNAL_EXPORT StyleSelector
{
public:
    inline StyleSelector(const StyleSheet &sheet) : styleSheet(sheet) {}
    virtual ~StyleSelector();
    
    union NodePtr {
        void *ptr;
        int id;
    };
    
    QVector<Declaration> declarationsForNode(NodePtr node);
    
    virtual QString nodeName(NodePtr node) const = 0;
    virtual QString attribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttributes(NodePtr node) const = 0;
    virtual QStringList nodeIds(NodePtr node) const;
    virtual NodePtr parentNode(NodePtr node) = 0;
    virtual void freeNode(NodePtr node) = 0;

    StyleSheet styleSheet;
private:
    bool selectorMatches(const Selector &rule, NodePtr node);
};

enum TokenType {
    NONE,

    S,

    CDO,
    CDC,
    INCLUDES,
    DASHMATCH,

    LBRACE,
    PLUS,
    GREATER,
    COMMA,

    STRING,
    INVALID,

    IDENT,

    HASH,

    ATKEYWORD_SYM,
    
    EXCLAMATION_SYM,

    LENGTH,

    PERCENTAGE,
    NUMBER,

    FUNCTION,

    COLON,
    SEMICOLON,
    RBRACE,
    SLASH,
    MINUS,
    DOT,
    STAR,
    LBRACKET,
    RBRACKET,
    EQUAL,
    LPAREN,
    RPAREN,
    OR
};

struct Q_INTERNAL_EXPORT Symbol
{
    inline Symbol() : start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class Q_INTERNAL_EXPORT Scanner
{
public:
    static QString preprocess(const QString &input);
    static QVector<Symbol> scan(const QString &preprocessedInput);
    static const char *tokenName(TokenType t);
};

class Q_INTERNAL_EXPORT Parser
{
public:
    explicit Parser(const QString &css);

    bool parse(StyleSheet *styleSheet);
    Symbol errorSymbol();

    bool parseImport(ImportRule *importRule);
    bool parseMedia(MediaRule *mediaRule);
    bool parseMedium(QStringList *media);
    bool parsePage(PageRule *pageRule);
    bool parsePseudoPage(QString *selector);
    bool parseNextOperator(Value *value);
    bool parseCombinator(BasicSelector::Relation *relation);
    bool parseProperty(Declaration *decl);
    bool parseRuleset(StyleRule *styleRule);
    bool parseSelector(Selector *sel);
    bool parseSimpleSelector(BasicSelector *basicSel);
    bool parseClass(QString *name);
    bool parseElementName(QString *name);
    bool parseAttrib(AttributeSelector *attr);
    bool parsePseudo(PseudoClass *pseudo);
    bool parseNextDeclaration(Declaration *declaration);
    bool parsePrio(Declaration *declaration);
    bool parseExpr(QVector<Value> *values);
    bool parseTerm(Value *value);
    bool parseFunction(QString *name, QString *args);
    bool parseHexColor(QColor *col);
    bool testAndParseUri(QString *uri);

    inline bool testRuleset() { return testSelector(); }
    inline bool testSelector() { return testSimpleSelector(); }
    inline bool parseNextSelector(Selector *sel) { if (!testSelector()) return recordError(); return parseSelector(sel); }
    bool testSimpleSelector();
    inline bool parseNextSimpleSelector(BasicSelector *basicSel) { if (!testSimpleSelector()) return recordError(); return parseSimpleSelector(basicSel); }
    inline bool testElementName() { return test(IDENT) || test(STAR); }
    inline bool testClass() { return test(DOT); }
    inline bool testAttrib() { return test(LBRACKET); }
    inline bool testPseudo() { return test(COLON); }
    inline bool testMedium() { return test(IDENT); }
    inline bool parseNextMedium(QStringList *media) { if (!testMedium()) return recordError(); return parseMedium(media); }
    inline bool testPseudoPage() { return test(COLON); }
    inline bool testImport() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("import")); }
    inline bool testMedia() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("media")); }
    inline bool testPage() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("page")); }
    inline bool testCombinator() { return test(PLUS) || test(GREATER) || test(S); }
    inline bool testProperty() { return test(IDENT); }
    bool testTerm();
    inline bool testExpr() { return testTerm(); }
    inline bool parseNextExpr(QVector<Value> *values) { if (!testExpr()) return recordError(); return parseExpr(values); }
    bool testPrio();
    inline bool testHexColor() { return test(HASH); }
    inline bool testFunction() { return test(FUNCTION); }
    inline bool parseNextFunction(QString *name, QString *args) { if (!testFunction()) return recordError(); return parseFunction(name, args); }

    inline bool lookupElementName() const { return lookup() == IDENT || lookup() == STAR; }

    inline void skipSpace() { while (test(S)); }

    inline bool hasNext() const { return index < symbols.count(); }
    inline TokenType next() { return symbols.at(index++).token; }
    bool next(TokenType t);
    bool test(TokenType t);
    inline void prev() { index--; }
    inline const Symbol &symbol() const { return symbols.at(index - 1); }
    inline QString lexem() const { return symbol().lexem(); }
    QString unquotedLexem() const;
    QString lexemUntil(TokenType t);
    bool until(TokenType target, TokenType target2 = NONE);
    inline TokenType lookup() const {
        return (index - 1) < symbols.count() ? symbols.at(index - 1).token : NONE;
    }

    bool testTokenAndEndsWith(TokenType t, const QLatin1String &str);
    
    inline bool recordError() { errorIndex = index; return false; }

    QVector<Symbol> symbols;
    int index;
    int errorIndex;
};

}

#endif

