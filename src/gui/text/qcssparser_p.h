/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCSSPARSER_P_H
#define QCSSPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QPair>
#include <QSize>
#include <QBrush>

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
    Margin,
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
    SelectionForeground,
    SelectionBackground,
    Border,
    BorderLeft,
    BorderRight,
    BorderTop,
    BorderBottom,
    Padding,
    PaddingLeft,
    PaddingRight,
    PaddingTop,
    PaddingBottom,
    AlternateBackground,
    BorderLeftStyle,
    BorderRightStyle,
    BorderTopStyle,
    BorderBottomStyle,
    BorderStyles,
    BorderLeftColor,
    BorderRightColor,
    BorderTopColor,
    BorderBottomColor,
    BorderColor,
    BorderLeftWidth,
    BorderRightWidth,
    BorderTopWidth,
    BorderBottomWidth,
    BorderWidth,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderRadius,
    Background,
    BackgroundOrigin,
    BackgroundRepeat,
    BackgroundPosition,
    BackgroundImage,
    BorderImage,
    Spacing,
    Width,
    Height,
    MinimumWidth,
    MinimumHeight,
    MaximumWidth,
    MaximumHeight,
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
    Value_Top,
    Value_Bottom,
    Value_Center,
    Value_Native,
    Value_Auto,
    NumKnownValues
};

enum BorderStyle {
    BorderStyle_Unknown,
    BorderStyle_None,
    BorderStyle_Dotted,
    BorderStyle_Dashed,
    BorderStyle_Solid,
    BorderStyle_Double,
    BorderStyle_DotDash,
    BorderStyle_DotDotDash,
    BorderStyle_Groove,
    BorderStyle_Ridge,
    BorderStyle_Inset,
    BorderStyle_Outset,
    NumKnownBorderStyles
};

struct BorderStyles
{
    BorderStyles() : left(BorderStyle_None), right(BorderStyle_None),
                     top(BorderStyle_None), bottom(BorderStyle_None) { }
    BorderStyle left, right, top, bottom;
};

struct BorderBrushes
{
    QBrush left, right, top, bottom;
};

enum Edge {
    TopEdge,
    RightEdge,
    BottomEdge,
    LeftEdge,
    NumEdges
};

enum Corner {
    TopLeftCorner,
    TopRightCorner,
    BottomLeftCorner,
    BottomRightCorner
};

enum TileMode {
    TileMode_Unknown,
    TileMode_Round,
    TileMode_Stretch,
    TileMode_Repeat,
    NumKnownTileModes
};

enum Repeat {
    Repeat_Unknown,
    Repeat_None,
    Repeat_X,
    Repeat_Y,
    Repeat_XY,
    NumKnownRepeats
};

enum Origin {
    Origin_Unknown,
    Origin_Padding,
    Origin_Border,
    Origin_Content,
    NumKnownOrigins
};

struct Q_GUI_EXPORT Value
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

struct Q_GUI_EXPORT Declaration
{
    inline Declaration() : propertyId(UnknownProperty), important(false) {}
    QString property;
    Property propertyId;
    inline bool isEmpty() const { return property.isEmpty() && propertyId == UnknownProperty; }
    QVector<Value> values;
    bool important;

    // helper functions
    QColor colorValue() const;
    void colorValues(QColor *c) const;
    QBrush brushValue() const;
    bool realValue(qreal *value, const char *unit = 0) const;
    bool realValue(Value v, qreal *real, const char *unit = 0) const;
    bool intValue(int *value, const char *unit = 0) const;
    bool intValue(Value v, int *real, const char *unit = 0) const;
    void realValues(qreal *reals, const char *unit = 0, int offset = 0) const;
    void intValues(int *ints, const char *unit = 0, int offset = 0) const;
    BorderStyle styleValue() const;
    BorderStyle styleValue(Value v) const;
    void styleValues(BorderStyle *s) const;
    void radiiValues(QSize *radii, const char *unit = 0) const;
    void radiusValue(QSize *radius, const char *unit = 0) const;
    Origin originValue() const;
    Repeat repeatValue() const;
    Qt::Alignment alignmentValue() const;
    QString uriValue() const;
    void pixmapValue(QPixmap *pixmap, QSize *size) const;
    QIcon iconValue() const;
    QSize sizeValue(const char *unit = 0, int offset = 0) const;
    void borderImageValue(QPixmap *pixmap, int *cuts, TileMode *h, TileMode *v) const;
};

enum PseudoState
{
    PseudoState_Unknown         = 0x00000000,
    PseudoState_Enabled         = 0x00000001,
    PseudoState_Disabled        = 0x00000002,
    PseudoState_Pressed         = 0x00000004,
    PseudoState_Focus           = 0x00000008,
    PseudoState_Hover           = 0x00000010,
    PseudoState_Checked         = 0x00000020,
    PseudoState_Unchecked       = 0x00000040,
    PseudoState_Indeterminate   = 0x00000080,
    PseudoState_Unspecified     = 0x00000100,
    NumPseudos = 9
};

struct Q_GUI_EXPORT PseudoClass
{
    PseudoState type;
    QString name;
    QString function;
};

struct Q_GUI_EXPORT AttributeSelector
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

struct Q_GUI_EXPORT BasicSelector
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

struct Q_GUI_EXPORT Selector
{
    QVector<BasicSelector> basicSelectors;
    int specificity() const;
    int pseudoState() const;
};

struct StyleRule;
struct MediaRule;
struct PageRule;
struct ImportRule;

void extractFontProperties(const QVector<Declaration> &declarations, QFont *font, int *fontSizeAdjustment);
void Q_AUTOTEST_EXPORT extractBackgroundProperties(const QVector<Declaration> &declarations, QColor *color, QString *image,
                                                   Repeat *repeat, Qt::Alignment *alignment);

struct Q_GUI_EXPORT StyleRule
{
    QVector<Selector> selectors;
    QVector<Declaration> declarations;
};

struct Q_GUI_EXPORT MediaRule
{
    QStringList media;
    QVector<StyleRule> styleRules;
};

struct Q_GUI_EXPORT PageRule
{
    QString selector;
    QVector<Declaration> declarations;
};

struct Q_GUI_EXPORT ImportRule
{
    QString href;
    QStringList media;
};

struct Q_GUI_EXPORT StyleSheet
{
    QVector<StyleRule> styleRules;
    QVector<MediaRule> mediaRules;
    QVector<PageRule> pageRules;
    QVector<ImportRule> importRules;
};

class Q_GUI_EXPORT StyleSelector
{
public:
    virtual ~StyleSelector();

    union NodePtr {
        void *ptr;
        int id;
    };

    QVector<StyleRule> styleRulesForNode(NodePtr node);
    QVector<Declaration> declarationsForNode(NodePtr node);

    virtual bool hasNodeName(NodePtr node, const QString& nodeName) const = 0;
    virtual QString attribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttributes(NodePtr node) const = 0;
    virtual QStringList nodeIds(NodePtr node) const;
    virtual bool isNullNode(NodePtr node) const = 0;
    virtual NodePtr parentNode(NodePtr node) = 0;
    virtual NodePtr previousSiblingNode(NodePtr node) = 0;
    virtual NodePtr duplicateNode(NodePtr node) = 0;
    virtual void freeNode(NodePtr node) = 0;

    QList<StyleSheet> styleSheets;
    QString medium;
private:
    void matchRules(NodePtr node, const QVector<StyleRule> &rules, QVector<QPair<int, StyleRule> > *weightedRules);
    bool selectorMatches(const Selector &rule, NodePtr node);
    bool basicSelectorMatches(const BasicSelector &rule, NodePtr node);
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

struct Q_AUTOTEST_EXPORT Symbol
{
    inline Symbol() : start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class Q_AUTOTEST_EXPORT Scanner
{
public:
    static QString preprocess(const QString &input);
    static QVector<Symbol> scan(const QString &preprocessedInput);
    static const char *tokenName(TokenType t);
};

class Q_GUI_EXPORT Parser
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
