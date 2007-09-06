/* A Bison parser, made by GNU Bison 2.3a.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3a"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse XPathparse
#define yylex   XPathlex
#define yyerror XPatherror
#define yylval  XPathlval
#define yychar  XPathchar
#define yydebug XPathdebug
#define yynerrs XPathnerrs
#define yylloc XPathlloc

/* Copy the first part of user declarations.  */
/* Line 164 of yacc.c.  */
#line 1 "../../sdk/QueryTransformParser.ypp"

/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include <QUrl>

#include "AbstractFloat.h"
#include "AndExpression.h"
#include "AnyURI.h"
#include "ArgumentReference.h"
#include "ArithmeticExpression.h"
#include "AtomicString.h"
#include "AttributeConstructor.h"
#include "AttributeNameValidator.h"
#include "AxisStep.h"
#include "BuiltinTypes.h"
#include "CastableAs.h"
#include "CastAs.h"
#include "CombineNodes.h"
#include "CommentConstructor.h"
#include "CommonNamespaces.h"
#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "ContextItem.h"
#include "Debug.h"
#include "DelegatingNamespaceResolver.h"
#include "DocumentConstructor.h"
#include "ElementConstructor.h"
#include "EmptySequence.h"
#include "EmptySequenceType.h"
#include "EvaluationCache.h"
#include "ExpressionSequence.h"
#include "ExpressionVariableReference.h"
#include "ExternalVariableReference.h"
#include "ForClause.h"
#include "FunctionFactory.h"
#include "GeneralComparison.h"
#include "GenericPredicate.h"
#include "GenericSequenceType.h"
#include "IfThenClause.h"
#include "InstanceOf.h"
#include "LetClause.h"
#include "Literal.h"
#include "PatternistLocale.h"
#include "LocalNameTest.h"
#include "NamespaceConstructor.h"
#include "NamespaceNameTest.h"
#include "NCNameConstructor.h"
#include "NodeComparison.h"
#include "Numeric.h"
#include "OrderBy.h"
#include "OrExpression.h"
#include "ParserContext.h"
#include "Path.h"
#include "PositionalVariableReference.h"
#include "ProcessingInstructionConstructor.h"
#include "QNameConstructor.h"
#include "QNameTest.h"
#include "QNameValue.h"
#include "QuantifiedExpression.h"
#include "RangeExpression.h"
#include "RangeVariableReference.h"
#include "ReturnOrderBy.h"
#include "SchemaTypeFactory.h"
#include "SimpleContentConstructor.h"
#include "TextNodeConstructor.h"
#include "Tokenizer.h"
#include "TreatAs.h"
#include "TypeChecker.h"
#include "UnaryExpression.h"
#include "UserFunctionCallsite.h"
#include "ValueComparison.h"
#include "XPathHelper.h"

/*
 * The cpp generated with bison 2.1 wants to
 * redeclare the C-like prototypes of 'malloc' and 'free', so we avoid that.
 */
#define YYMALLOC malloc
#define YYFREE free

using namespace Patternist;

/**
 * "Macro that you define with #define in the Bison declarations
 * section to request verbose, specific error message strings when
 * yyerror is called."
 */
#define YYERROR_VERBOSE 1

#undef YYLTYPE_IS_TRIVIAL
#define YYLTYPE_IS_TRIVIAL 0

/* Supresses `warning: "YYENABLE_NLS" is not defined`
 * @c YYENABLE_NLS enables Bison internationalization, and we don't
 * use that, so disable it. See the Bison Manual, section 4.5 Parser Internationalization.
 */
#define YYENABLE_NLS 0

static inline QSourceLocation fromYYLTYPE(const YYLTYPE &sourceLocator,
                                          const ParserContext *const parseInfo)
{
    qDebug() << Q_FUNC_INFO << parseInfo->tokenizer->uri();
    return QSourceLocation(parseInfo->tokenizer->uri(),
                           sourceLocator.first_line,
                           sourceLocator.first_column);
}

class ReflectYYLTYPE : public SourceLocationReflection
{
public:
    inline ReflectYYLTYPE(const YYLTYPE &sourceLocator,
                          const ParserContext *const pi) : m_sl(sourceLocator)
                                                         , m_parseInfo(pi)
    {
    }

    virtual const SourceLocationReflection *actualReflection() const
    {
        return this;
    }

    virtual QSourceLocation sourceLocation() const
    {
        return fromYYLTYPE(m_sl, m_parseInfo);
    }

    virtual QString description() const
    {
        Q_ASSERT(false);
        return QLatin1String("ReflectYYLTYPE, not implemented");
    }

private:
    const YYLTYPE &m_sl;
    const ParserContext *const m_parseInfo;
};

static inline Expression::Ptr create(Expression *expr,
                                     const YYLTYPE &sourceLocator,
                                     const ParserContext *const parseInfo)
{
    Expression::Ptr e(expr);
    parseInfo->staticContext->addLocation(expr, fromYYLTYPE(sourceLocator, parseInfo));
    return e;
}

static inline Expression::Ptr create(const Expression::Ptr &expr,
                                     const YYLTYPE &sourceLocator,
                                     const ParserContext *const parseInfo)
{
    parseInfo->staticContext->addLocation(expr.get(), fromYYLTYPE(sourceLocator, parseInfo));
    return expr;
}

/**
 * @short The generated Bison parser calls this function when there is a parse error.
 *
 * It is not called, nor should be, for logical errors(which the Bison not know about). For those,
 * ReportContext::error() is called.
 */
static int XPatherror(YYLTYPE *sourceLocator, const ParserContext *const parseInfo, const char *const msg)
{
    Q_UNUSED(sourceLocator);
    Q_ASSERT(parseInfo);

    parseInfo->staticContext->error(QLatin1String(msg), ReportContext::XPST0003, fromYYLTYPE(*sourceLocator, parseInfo));
    return 1;
}

/**
 * @short Centralizes a translation message, for the
 * purpose of consistency and modularization.
 */
static inline QString prologMessage(const char *const msg)
{
    Q_ASSERT(msg);
    return tr("Only one %1 declaration can occur in the query prolog.").arg(formatKeyword(msg));
}

/**
 * @short Resolves against the static base URI and checks that @p collation
 * is a supported Unicode Collation.
 *
 * "If a default collation declaration specifies a collation by a
 *  relative URI, that relative URI is resolved to an absolute
 *  URI using the base URI in the static context."
 *
 * @returns the Unicode Collation properly resolved, if @p collation is a valid collation
 */
template<const ReportContext::ErrorCode errorCode>
static QUrl resolveAndCheckCollation(const QString &collation,
                                     const ParserContext *const parseInfo,
                                     const YYLTYPE &sl)
{
    Q_ASSERT(parseInfo);
    const ReflectYYLTYPE ryy(sl, parseInfo);

    QUrl uri(AnyURI::toQUrl<ReportContext::XQST0046>(collation, parseInfo->staticContext, &ryy));

    if(uri.isRelative())
        uri = parseInfo->staticContext->baseURI().resolved(uri);

    XPathHelper::checkCollationSupport<errorCode>(uri.toString(), parseInfo->staticContext, &ryy);

    return uri;
}

/* The Bison generated parser declares macros that aren't used
 * so supress the warnings by fake usage of them.
 *
 * We do the same for some more defines in the first action. */
#if    defined(YYLSP_NEEDED)    \
    or defined(YYBISON)         \
    or defined(YYBISON_VERSION) \
    or defined(YYPURE)          \
    or defined(yydebug)         \
    or defined(YYSKELETON_NAME)
#endif

/**
 * @short Creates an Expression that corresponds to <tt>/</tt>. This is literally
 * <tt>fn:root(self::node()) treat as document-node()</tt>.
 */
static Expression::Ptr createRootExpression(const ParserContext *const parseInfo,
                                            const YYLTYPE &sl)
{
    Q_ASSERT(parseInfo);
    const QName name(StandardNamespaces::fn, StandardLocalNames::root);

    Expression::List args;
    args.append(create(new ContextItem(), sl, parseInfo));

    const ReflectYYLTYPE ryy(sl, parseInfo);

    // TODO Shouldn't fnRoot be add with addLocation()?
    const Expression::Ptr fnRoot(parseInfo->staticContext->functionSignatures()
                                 ->createFunctionCall(name, args, parseInfo->staticContext, &ryy));
    Q_ASSERT(fnRoot);

    return create(new TreatAs(create(fnRoot, sl, parseInfo), CommonSequenceTypes::ExactlyOneDocumentNode), sl, parseInfo);
}

static int XPathlex(YYSTYPE *lexVal, YYLTYPE *sourceLocator, const ParserContext *const parseInfo)
{
#ifdef Patternist_DEBUG_PARSER
    /**
     * "External integer variable set to zero by default. If yydebug
     *  is given a nonzero value, the parser will output information on
     *  input symbols and parser action. See section Debugging Your Parser."
     */
#   define YYDEBUG 1

    extern int XPathdebug;
    XPathdebug = 1;
#endif

    Q_ASSERT(parseInfo);

    const Tokenizer::Token tok(parseInfo->tokenizer->nextToken(sourceLocator));

    if(tok.enums.zeroer)
        (*lexVal).enums = tok.enums;
    else
        (*lexVal).sval = tok.value;

    return static_cast<int>(tok.type);
}

/**
 * @short Creates a path expression which contains the step <tt>//</tt> between
 * @p begin and and @p end.
 *
 * <tt>begin//end</tt> is a short form for: <tt>begin/descendant-or-self::node()/end</tt>
 *
 * This will be compiled as two-path expression: <tt>(/)/(//.)/step/</tt>
 */
static Expression::Ptr createSlashSlashPath(const Expression::Ptr &begin,
                                            const Expression::Ptr &end,
                                            const YYLTYPE &sourceLocator,
                                            const ParserContext *const parseInfo)
{
    const Expression::Ptr twoSlash(new AxisStep(Node::DescendantOrSelf, BuiltinTypes::node));
    const Expression::Ptr p1(new Path(begin, twoSlash));

    return create(new Path(p1, end), sourceLocator, parseInfo);
}

/**
 * @short Creates a call to <tt>fn:concat()</tt> with @p args as the arguments.
 */
static inline Expression::Ptr createConcatFN(const ParserContext *const parseInfo,
                                             const Expression::List &args,
                                             const YYLTYPE &sourceLocator)
{
    Q_ASSERT(parseInfo);
    const QName name(StandardNamespaces::fn, StandardLocalNames::concat);
    const ReflectYYLTYPE ryy(sourceLocator, parseInfo);

    return create(parseInfo->staticContext->functionSignatures()->createFunctionCall(name, args, parseInfo->staticContext, &ryy),
                  sourceLocator, parseInfo);
}

static inline Expression::Ptr createDirAttributeValue(const Expression::List &content,
                                                      const ParserContext *const parseInfo,
                                                      const YYLTYPE &sourceLocator)
{
    if(content.isEmpty())
        return create(new EmptySequence(), sourceLocator, parseInfo);
    else if(content.size() == 1)
        return content.first();
    else
        return createConcatFN(parseInfo, content, sourceLocator);
}

/**
 * @short Determines whether @p consists only of whitespace. Characters
 * considered whitespace are the ones for which QChar::isSpace() returns @c true for.
 *
 * @returns @c true if @p string consists only of whitespace, otherwise @c false.
 */
static inline bool isWhitespaceOnly(const QString &string)
{
    const int len = string.length();

    for(int i = 0; i < len; ++i)
    {
        if(!string.at(i).isSpace())
            return false;
    }

    return true;
}

/**
 * @short Checks for variable initialization circularity.
 *
 * Issues an error via @p parseInfo's StaticContext if the initialization expression @p checkee for the
 * global variable @p var, contains a variable reference to @p var. That is, if there's
 * a circularity.
 * @see <a href="http://www.w3.org/TR/xquery/#ERRXQST0054">XQuery 1.0: An XML
 * Query Language, err:XQST0054</a>
 */
static void checkVariableCircularity(const VariableDeclaration::Ptr &var,
                                     const Expression::Ptr &checkee,
                                     FunctionSignature::List &signList,
                                     const ParserContext *const parseInfo)
{
    Q_ASSERT(var);
    Q_ASSERT(checkee);
    Q_ASSERT(parseInfo);
    qDebug() << Q_FUNC_INFO << "slot:" << var->slot << endl;

    const Expression::ID id = checkee->id();

    if(id == Expression::IDExpressionVariableReference)
    {
        const ExpressionVariableReference *const ref =
                    static_cast<ExpressionVariableReference *>(checkee.get());
        qDebug() << "Found ExpressionVariableReference, its slot: " << ref->slot() << endl;

        if(var->slot == ref->slot())
        {
            parseInfo->staticContext->error(tr("The initialization of variable %1 "
                                               "depends on itself").arg(formatKeyword(var, parseInfo->staticContext->namePool())),
                                            ReportContext::XQST0054, ref);
            return;
        }
        else
        {
            /* If the variable we're checking is below another variable, it can be a recursive
               dependency through functions, so we need to check variable references too. */
            checkVariableCircularity(var, ref->sourceExpression(), signList, parseInfo);
            return;
        }
    }
    else if(id == Expression::IDUserFunctionCallsite)
    {
        qDebug() << "Found IDUserFunctionCallsite" << endl;
        const UserFunctionCallsite::Ptr callsite(checkee);
        const FunctionSignature::Ptr sign(callsite->signature());
        const FunctionSignature::List::const_iterator end(signList.constEnd());
        FunctionSignature::List::const_iterator it(signList.constBegin());
        bool noMatch = true;

        for(; it != end; ++it)
        {
            if(*it == sign)
            {
                /* The variable we're checking is depending on a function that's recursive. The
                 * user has written a weird query, in other words. Since it's the second time
                 * we've encountered a callsite, we now skip it. */
                noMatch = false;
                qDebug() << "MATCH" << endl;
                break;
            }
        }

        if(noMatch)
        {
            signList.append(sign);
            /* Check the body of the function being called. */
            checkVariableCircularity(var, callsite->body(), signList, parseInfo);
        }
        /* Continue with the operands, such that we also check the arguments of the callsite. */
        qDebug() << "Continuing with ops" << endl;
    }

    /* Check the operands. */
    const Expression::List ops(checkee->operands());
    if(ops.isEmpty())
        return;

    const Expression::List::const_iterator end(ops.constEnd());
    Expression::List::const_iterator it(ops.constBegin());

    for(; it != end; ++it)
        checkVariableCircularity(var, *it, signList, parseInfo);
}

static void checkCallsiteCircularity(FunctionSignature::List &signList,
                                     const Expression::Ptr &expr,
                                     const ParserContext *const parseInfo)
{
    qDebug() << Q_FUNC_INFO << endl;
    Q_ASSERT(expr);
    Q_ASSERT(parseInfo);

    if(expr->is(Expression::IDUserFunctionCallsite))
    {
        FunctionSignature::List::const_iterator it(signList.constBegin());
        const FunctionSignature::List::const_iterator end(signList.constEnd());
        UserFunctionCallsite *const callsite = static_cast<UserFunctionCallsite *>(expr.get());

        for(; it != end; ++it)
        {
            if(callsite->configureRecursion(*it))
            {
                /* A callsite inside the function body to the function. This user function
                 * is recursive in other words. */
                return; /* We're done in this case. */
            }
        }
        /* Check the body of the function so this callsite isn't "indirectly" a
         * recursive call to the function we're checking. XQTS test case
         * default_namespace-011 is an example of this. */
        signList.append(callsite->signature());
        checkCallsiteCircularity(signList, callsite->body(), parseInfo);
    }
    else
    {
        /* Check the operands. */
        const Expression::List ops(expr->operands());
        const Expression::List::const_iterator end(ops.constEnd());
        Expression::List::const_iterator it(ops.constBegin());

        for(; it != end; ++it)
            checkCallsiteCircularity(signList, *it, parseInfo);
    }
}

/**
 * The Cardinality in a TypeDeclaration for a variable in a quantification has no effect,
 * and this function ensures this by changing @p type to Cardinality Cardinality::zeroOrMore().
 *
 * @see <a href="http://www.w3.org/Bugs/Public/show_bug.cgi?id=3305">Bugzilla Bug 3305
 * Cardinality + on range variables</a>
 */
static inline SequenceType::Ptr quantificationType(const SequenceType::Ptr &type)
{
    Q_ASSERT(type);
    return makeGenericSequenceType(type->itemType(), Cardinality::zeroOrMore());
}

/**
 * @p seqType and @p expr may be @c null.
 */
static Expression::Ptr pushVariable(const QName name,
                                    const SequenceType::Ptr &seqType,
                                    const Expression::Ptr &expr,
                                    const VariableDeclaration::Type type,
                                    const YYLTYPE &sourceLocator,
                                    ParserContext *const parseInfo,
                                    const bool checksource = true)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(parseInfo);

    /* -2 will cause Q_ASSERTs to trigger if it isn't changed. */
    VariableSlotID slot = -2;

    switch(type)
    {
        case VariableDeclaration::FunctionArgument:     /* Fallthrough. */
        case VariableDeclaration::ExpressionVariable:   slot = parseInfo->allocateExpressionSlot(); break;
        case VariableDeclaration::RangeVariable:        slot = parseInfo->allocateRangeSlot();      break;
        case VariableDeclaration::PositionalVariable:   slot = parseInfo->allocatePositionalSlot(); break;
    }

    const VariableDeclaration::Ptr var(new VariableDeclaration(name, slot, type, seqType));

    Expression::Ptr checked;

    if(checksource)
    {
        if(expr)
        {
            /* We only want to add conversion for function arguments.
             *
             * We unconditionally skip TypeChecker::CheckFocus because the StaticContext we
             * pass hasn't set up the focus yet, since that's the parent's responsibility. */
            const TypeChecker::Options options((type == VariableDeclaration::FunctionArgument ? TypeChecker::AutomaticallyConvert
                                                                                              : TypeChecker::Options()));

            checked = TypeChecker::applyFunctionConversion(expr, seqType, parseInfo->staticContext,
                                                           ReportContext::XPTY0004, options);
        }
    }
    else
        checked = expr;

    /* Add an evaluation cache for all expression variables. No EvaluationCache is needed for
     * positional variables because in the end they are calls to Iterator::position(). Similarly,
     * no need to cache range variables either because they are calls to DynamicContext::rangeVariable().
     *
     * We don't do it for function arguments because the Expression being cached depends -- it depends
     * on the callsite. UserFunctionCallsite is responsible for the evaluation caches in that case.
     *
     * In some cases the EvaluationCache instance isn't necessary, but in those cases EvaluationCache
     * optimizes itself away. */
    if(type == VariableDeclaration::ExpressionVariable)
        checked = create(new EvaluationCache(checked, var, parseInfo->allocateCacheSlot()), sourceLocator, parseInfo);

    var->setExpression(checked);

    parseInfo->variables.push(var);
    return checked;
}

/**
 * @short Removes the recently pushed variables from
 * scope. The amount of removed variables is @p amount.
 */
static void finalizePushedVariable(ParserContext *const parseInfo, const int amount = 1)
{
    Q_ASSERT(parseInfo);

    for(int i = 0; i < amount; ++i)
    {
        const VariableDeclaration::Ptr var(parseInfo->variables.pop());
        Q_ASSERT(var);

        if(var->isUsed())
            continue;
        else
        {
            parseInfo->staticContext->warning(tr("The variable %1 is unused")
                                                .arg(formatKeyword(var, parseInfo->staticContext->namePool())));
        }
    }
}

static inline VariableDeclaration::Ptr variableByName(const QName name,
                                                      const ParserContext *const parseInfo)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(parseInfo);

    /* We walk the list backwards. */
    const VariableDeclaration::Stack::const_iterator start(parseInfo->variables.constBegin());
    VariableDeclaration::Stack::const_iterator it(parseInfo->variables.constEnd());

    while(it != start)
    {
        --it;
        Q_ASSERT(*it);
        if((*it)->name == name)
            return *it;
    }

    return VariableDeclaration::Ptr();
}

static Expression::Ptr createReturnOrderBy(const OrderSpecTransfer::List &orderSpecTransfer,
                                           const Expression::Ptr &returnExpr,
                                           const OrderBy::Stability stability,
                                           const YYLTYPE &sourceLocator,
                                           const ParserContext *const parseInfo)
{
    // TODO do resize(orderSpec.size() + 1)
    Expression::List exprs;
    OrderBy::OrderSpec::Vector orderSpecs;

    exprs.append(returnExpr);

    const int len = orderSpecTransfer.size();

    for(int i = 0; i < len; ++i)
    {
        exprs.append(orderSpecTransfer.at(i).expression);
        orderSpecs.append(orderSpecTransfer.at(i).orderSpec);
    }

    return create(new ReturnOrderBy(stability, orderSpecs, exprs), sourceLocator, parseInfo);
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END_OF_FILE = 0,
     STRING_LITERAL = 258,
     NON_BOUNDARY_WS = 259,
     XPATH2_STRING_LITERAL = 260,
     QNAME = 261,
     NCNAME = 262,
     ANY_LOCAL_NAME = 263,
     ANY_PREFIX = 264,
     NUMBER = 265,
     XPATH2_NUMBER = 266,
     AND = 267,
     APOS = 268,
     AS = 269,
     ASCENDING = 270,
     ASSIGN = 271,
     AT = 272,
     AT_SIGN = 273,
     ATTRIBUTE = 274,
     BAR = 275,
     BASEURI = 276,
     BEGIN_END_TAG = 277,
     BOUNDARY_SPACE = 278,
     BY = 279,
     CASE = 280,
     CASTABLE = 281,
     CAST = 282,
     COLLATION = 283,
     COLON = 284,
     COLONCOLON = 285,
     COMMA = 286,
     COMMENT = 287,
     COMMENT_START = 288,
     CONSTRUCTION = 289,
     COPY_NAMESPACES = 290,
     CURLY_LBRACE = 291,
     CURLY_RBRACE = 292,
     DECLARE = 293,
     DEFAULT = 294,
     DESCENDING = 295,
     DIV = 296,
     DOCUMENT = 297,
     DOCUMENT_NODE = 298,
     DOLLAR = 299,
     DOT = 300,
     DOTDOT = 301,
     ELEMENT = 302,
     ELSE = 303,
     EMPTY = 304,
     EMPTY_SEQUENCE = 305,
     ENCODING = 306,
     EQ = 307,
     ERROR = 308,
     EVERY = 309,
     EXCEPT = 310,
     EXTERNAL = 311,
     FOLLOWS = 312,
     FOR = 313,
     FUNCTION = 314,
     GE = 315,
     G_EQ = 316,
     G_GE = 317,
     G_GT = 318,
     G_LE = 319,
     G_LT = 320,
     G_NE = 321,
     GREATEST = 322,
     GT = 323,
     IDIV = 324,
     IF = 325,
     IMPORT = 326,
     INHERIT = 327,
     IN = 328,
     INSTANCE = 329,
     INTERSECT = 330,
     IS = 331,
     ITEM = 332,
     LAX = 333,
     LBRACKET = 334,
     LEAST = 335,
     LE = 336,
     LET = 337,
     LPAREN = 338,
     LT = 339,
     MINUS = 340,
     MOD = 341,
     MODULE = 342,
     NAMESPACE = 343,
     NE = 344,
     NODE = 345,
     NO_ELEMENT_CONTENT = 346,
     NO_INHERIT = 347,
     NO_PRESERVE = 348,
     OF = 349,
     OPTION = 350,
     ORDER_BY = 351,
     ORDERED = 352,
     ORDERING = 353,
     ORDER = 354,
     OR = 355,
     PI_START = 356,
     PLUS = 357,
     POSITION_SET = 358,
     PRAGMA_END = 359,
     PRAGMA_START = 360,
     PRECEDES = 361,
     PRESERVE = 362,
     PROCESSING_INSTRUCTION = 363,
     QUESTION = 364,
     QUICK_TAG_END = 365,
     QUOTE = 366,
     RBRACKET = 367,
     RETURN = 368,
     RPAREN = 369,
     SATISFIES = 370,
     SCHEMA = 371,
     SCHEMA_ATTRIBUTE = 372,
     SCHEMA_ELEMENT = 373,
     SEMI_COLON = 374,
     SLASH = 375,
     SLASHSLASH = 376,
     SOME = 377,
     STABLE = 378,
     STAR = 379,
     STRICT = 380,
     STRIP = 381,
     SUCCESS = 382,
     COMMENT_CONTENT = 383,
     PI_CONTENT = 384,
     PI_TARGET = 385,
     TEXT = 386,
     THEN = 387,
     TO = 388,
     TREAT = 389,
     TYPESWITCH = 390,
     UNION = 391,
     UNORDERED = 392,
     VALIDATE = 393,
     VARIABLE = 394,
     VERSION = 395,
     WHERE = 396,
     XQUERY = 397,
     ANCESTOR_OR_SELF = 398,
     ANCESTOR = 399,
     CHILD = 400,
     DESCENDANT_OR_SELF = 401,
     DESCENDANT = 402,
     FOLLOWING_SIBLING = 403,
     FOLLOWING = 404,
     PRECEDING = 405,
     PARENT = 406,
     PRECEDING_SIBLING = 407,
     SELF = 408
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */

/* Line 221 of yacc.c.  */
#line 899 "QueryTransformParser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1462

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  154
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  206
/* YYNRULES -- Number of rules.  */
#define YYNRULES  401
/* YYNRULES -- Number of states.  */
#define YYNSTATES  666

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   408

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    16,    17,    20,    23,
      26,    33,    34,    37,    40,    43,    46,    49,    52,    55,
      57,    59,    61,    63,    65,    67,    69,    71,    73,    75,
      82,    87,    89,    91,    93,    95,   102,   109,   115,   120,
     122,   124,   130,   133,   136,   143,   145,   147,   149,   151,
     157,   162,   169,   170,   174,   178,   185,   186,   190,   191,
     194,   196,   200,   208,   210,   213,   218,   220,   222,   223,
     234,   235,   237,   241,   245,   247,   249,   253,   255,   257,
     259,   263,   267,   269,   271,   273,   275,   277,   279,   281,
     282,   283,   294,   295,   296,   307,   309,   311,   313,   314,
     318,   319,   328,   329,   338,   340,   342,   344,   348,   354,
     355,   358,   362,   364,   369,   370,   372,   374,   375,   377,
     378,   381,   385,   388,   390,   392,   393,   394,   404,   405,
     406,   416,   418,   419,   420,   430,   431,   432,   442,   444,
     447,   448,   455,   456,   457,   466,   468,   470,   471,   475,
     479,   480,   487,   496,   498,   502,   504,   508,   510,   512,
     514,   516,   518,   522,   524,   528,   530,   532,   534,   538,
     540,   542,   544,   546,   548,   552,   554,   558,   560,   562,
     564,   566,   568,   573,   575,   580,   582,   587,   589,   594,
     596,   599,   601,   603,   605,   607,   609,   613,   615,   617,
     619,   621,   623,   625,   629,   631,   633,   635,   637,   639,
     641,   645,   647,   649,   651,   654,   656,   659,   662,   665,
     668,   672,   675,   677,   682,   683,   685,   688,   691,   693,
     695,   697,   701,   705,   707,   709,   711,   716,   718,   720,
     721,   725,   727,   729,   731,   734,   736,   738,   740,   742,
     744,   746,   748,   750,   752,   754,   756,   758,   759,   763,
     765,   767,   769,   771,   773,   775,   777,   779,   781,   783,
     785,   787,   792,   794,   796,   798,   800,   802,   804,   806,
     808,   810,   812,   814,   817,   819,   821,   825,   828,   830,
     833,   838,   839,   841,   843,   845,   847,   849,   851,   853,
     854,   855,   864,   866,   872,   873,   876,   880,   884,   888,
     889,   892,   895,   896,   899,   902,   905,   908,   911,   915,
     917,   919,   921,   923,   925,   927,   930,   931,   936,   940,
     943,   946,   950,   951,   952,   956,   958,   960,   962,   964,
     966,   968,   970,   973,   974,   977,   980,   983,   984,   986,
     988,   990,   992,   994,   996,   999,  1001,  1003,  1005,  1007,
    1009,  1011,  1013,  1015,  1018,  1021,  1026,  1028,  1030,  1033,
    1036,  1039,  1044,  1049,  1051,  1053,  1056,  1061,  1066,  1073,
    1080,  1085,  1088,  1093,  1098,  1105,  1112,  1117,  1120,  1122,
    1124,  1126,  1128,  1130,  1132,  1134,  1136,  1138,  1140,  1142,
    1144,  1146
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     155,     0,    -1,   156,   159,    -1,   156,   158,    -1,    -1,
     142,   140,   358,   157,   164,    -1,    -1,    51,   358,    -1,
     161,   197,    -1,   160,   161,    -1,    87,    88,     7,    61,
     357,   164,    -1,    -1,   161,   168,    -1,   161,   162,    -1,
     161,   165,    -1,   161,   163,    -1,   161,   187,    -1,   161,
     191,    -1,   161,   171,    -1,   166,    -1,   179,    -1,   180,
      -1,   189,    -1,   172,    -1,   174,    -1,   176,    -1,   181,
      -1,   183,    -1,   119,    -1,    38,    88,     7,    61,   357,
     164,    -1,    38,    23,   167,   164,    -1,   126,    -1,   107,
      -1,   169,    -1,   170,    -1,    38,    39,    47,    88,   357,
     164,    -1,    38,    39,    59,    88,   357,   164,    -1,    38,
      95,   352,   358,   164,    -1,    38,    98,   173,   164,    -1,
      97,    -1,   137,    -1,    38,    39,    99,   175,   164,    -1,
      49,    80,    -1,    49,    67,    -1,    38,    35,   177,    31,
     178,   164,    -1,   107,    -1,    93,    -1,    72,    -1,    92,
      -1,    38,    39,    28,   358,   164,    -1,    38,    21,   357,
     164,    -1,    71,   116,   182,   357,   185,   164,    -1,    -1,
      39,    47,    88,    -1,    88,     7,    61,    -1,    71,    87,
     184,   357,   185,   164,    -1,    -1,    88,     7,    61,    -1,
      -1,    17,   186,    -1,   357,    -1,   186,    31,   357,    -1,
      38,   139,    44,   299,   333,   188,   164,    -1,    56,    -1,
      16,   200,    -1,    38,    34,   190,   164,    -1,   126,    -1,
     107,    -1,    -1,    38,    59,   354,    83,   193,   114,   192,
     333,   195,   164,    -1,    -1,   194,    -1,   193,    31,   194,
      -1,    44,   299,   333,    -1,    56,    -1,   196,    -1,    36,
     198,    37,    -1,   198,    -1,   200,    -1,   199,    -1,   200,
      31,   200,    -1,   199,    31,   200,    -1,   245,    -1,   201,
      -1,   221,    -1,   235,    -1,   244,    -1,   202,    -1,   209,
      -1,    -1,    -1,    58,    44,   299,   333,   208,    73,   200,
     203,   204,   205,    -1,    -1,    -1,    31,    44,   299,   333,
     208,    73,   200,   206,   207,   205,    -1,   213,    -1,   202,
      -1,   209,    -1,    -1,    17,    44,   299,    -1,    -1,    82,
      44,   299,   333,    16,   200,   210,   211,    -1,    -1,    31,
      44,   299,   333,    16,   200,   212,   211,    -1,   213,    -1,
     202,    -1,   209,    -1,   214,   113,   200,    -1,   141,   200,
     214,   113,   200,    -1,    -1,   220,   215,    -1,   215,    31,
     216,    -1,   216,    -1,   200,   217,   218,   219,    -1,    -1,
      15,    -1,    40,    -1,    -1,   175,    -1,    -1,    28,   357,
      -1,   123,    99,    24,    -1,    99,    24,    -1,   222,    -1,
     228,    -1,    -1,    -1,   122,    44,   299,   333,    73,   200,
     223,   224,   225,    -1,    -1,    -1,    31,    44,   299,   333,
      73,   200,   226,   227,   225,    -1,   234,    -1,    -1,    -1,
      54,    44,   299,   333,    73,   200,   229,   230,   231,    -1,
      -1,    -1,    31,    44,   299,   333,    73,   200,   232,   233,
     231,    -1,   234,    -1,   115,   200,    -1,    -1,   135,    83,
     198,   114,   236,   237,    -1,    -1,    -1,    25,   241,   334,
     238,   113,   200,   239,   240,    -1,   237,    -1,   242,    -1,
      -1,    44,   352,    14,    -1,    39,   113,   200,    -1,    -1,
      39,    44,   352,   243,   113,   200,    -1,    70,    83,   198,
     114,   132,   200,    48,   200,    -1,   246,    -1,   245,   100,
     246,    -1,   247,    -1,   246,    12,   247,    -1,   248,    -1,
     266,    -1,   264,    -1,   268,    -1,   249,    -1,   249,   133,
     249,    -1,   251,    -1,   249,   250,   251,    -1,   102,    -1,
      85,    -1,   253,    -1,   251,   252,   253,    -1,   124,    -1,
      41,    -1,    69,    -1,    86,    -1,   254,    -1,   254,   255,
     254,    -1,   257,    -1,   257,   256,   257,    -1,   136,    -1,
      20,    -1,    75,    -1,    55,    -1,   258,    -1,   258,    74,
      94,   334,    -1,   259,    -1,   259,   134,    14,   334,    -1,
     260,    -1,   260,    26,    14,   332,    -1,   261,    -1,   261,
      27,    14,   332,    -1,   263,    -1,   262,   261,    -1,   102,
      -1,    85,    -1,   270,    -1,   277,    -1,   272,    -1,   248,
     265,   248,    -1,    61,    -1,    66,    -1,    62,    -1,    63,
      -1,    64,    -1,    65,    -1,   248,   267,   248,    -1,    52,
      -1,    89,    -1,    60,    -1,    68,    -1,    81,    -1,    84,
      -1,   248,   269,   248,    -1,    76,    -1,   106,    -1,    57,
      -1,   271,   196,    -1,   138,    -1,   138,   125,    -1,   138,
      78,    -1,   274,   273,    -1,    36,    37,    -1,    36,   198,
      37,    -1,   274,   275,    -1,   275,    -1,   105,   356,   276,
     104,    -1,    -1,   358,    -1,   120,   278,    -1,   121,   278,
      -1,   120,    -1,   278,    -1,   279,    -1,   278,   120,   279,
      -1,   278,   121,   279,    -1,   280,    -1,   294,    -1,   281,
      -1,   280,    79,   198,   112,    -1,   282,    -1,   289,    -1,
      -1,   285,   283,   284,    -1,   287,    -1,   291,    -1,   345,
      -1,   286,    30,    -1,   143,    -1,   144,    -1,    19,    -1,
     145,    -1,   146,    -1,   147,    -1,   149,    -1,   150,    -1,
     148,    -1,   152,    -1,   151,    -1,   153,    -1,    -1,    18,
     288,   291,    -1,   291,    -1,   345,    -1,   290,    -1,    46,
      -1,   292,    -1,   338,    -1,   352,    -1,   293,    -1,   124,
      -1,     8,    -1,     9,    -1,   295,    -1,   294,    79,   198,
     112,    -1,   296,    -1,   298,    -1,   300,    -1,   301,    -1,
     303,    -1,   302,    -1,   305,    -1,   297,    -1,   358,    -1,
      11,    -1,    10,    -1,    44,   299,    -1,     7,    -1,   359,
      -1,    83,   198,   114,    -1,    83,   114,    -1,    45,    -1,
     173,   196,    -1,   354,    83,   304,   114,    -1,    -1,   200,
      -1,   199,    -1,   306,    -1,   318,    -1,   307,    -1,   316,
      -1,   317,    -1,    -1,    -1,    65,   355,   308,   311,   309,
     103,   311,   310,    -1,   110,    -1,    63,   315,    22,   352,
      63,    -1,    -1,   311,   312,    -1,   355,    61,   313,    -1,
     111,   314,   111,    -1,    13,   314,    13,    -1,    -1,   196,
     314,    -1,   358,   314,    -1,    -1,   315,   306,    -1,   315,
     358,    -1,   315,     4,    -1,   315,   196,    -1,    33,   128,
      -1,   101,   130,   129,    -1,   319,    -1,   320,    -1,   322,
      -1,   323,    -1,   324,    -1,   325,    -1,    42,   196,    -1,
      -1,    47,   329,   321,   273,    -1,    19,   326,   273,    -1,
     131,   196,    -1,    32,   196,    -1,   108,   331,   273,    -1,
      -1,    -1,   327,   352,   328,    -1,   330,    -1,   352,    -1,
     330,    -1,   196,    -1,     7,    -1,   196,    -1,   337,    -1,
     337,   109,    -1,    -1,    14,   334,    -1,   336,   335,    -1,
      50,   350,    -1,    -1,   102,    -1,   124,    -1,   109,    -1,
     337,    -1,   338,    -1,   345,    -1,    77,   350,    -1,   352,
      -1,   340,    -1,   348,    -1,   349,    -1,   344,    -1,   343,
      -1,   342,    -1,   339,    -1,    90,   350,    -1,    43,   350,
      -1,    43,    83,   341,   114,    -1,   348,    -1,   349,    -1,
     131,   350,    -1,    32,   350,    -1,   108,   350,    -1,   108,
      83,     7,   114,    -1,   108,    83,   358,   114,    -1,   346,
      -1,   347,    -1,    19,   350,    -1,    19,    83,   124,   114,
      -1,    19,    83,   352,   114,    -1,    19,    83,   351,    31,
     353,   114,    -1,    19,    83,   124,    31,   353,   114,    -1,
     117,    83,   352,   114,    -1,    47,   350,    -1,    47,    83,
     124,   114,    -1,    47,    83,   352,   114,    -1,    47,    83,
     352,    31,   353,   114,    -1,    47,    83,   124,    31,   353,
     114,    -1,   118,    83,   352,   114,    -1,    83,   114,    -1,
     352,    -1,     7,    -1,   359,    -1,   352,    -1,     7,    -1,
     359,    -1,     7,    -1,     6,    -1,     7,    -1,   359,    -1,
     358,    -1,     3,    -1,     5,    -1,     6,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   922,   922,   923,   925,   926,   957,   958,   974,  1045,
    1047,  1053,  1054,  1055,  1056,  1057,  1058,  1059,  1060,  1062,
    1063,  1064,  1065,  1066,  1067,  1068,  1070,  1071,  1073,  1075,
    1118,  1132,  1137,  1142,  1143,  1145,  1161,  1176,  1186,  1200,
    1204,  1209,  1223,  1227,  1232,  1248,  1253,  1258,  1263,  1268,
    1284,  1306,  1314,  1315,  1316,  1318,  1335,  1336,  1338,  1339,
    1341,  1342,  1344,  1389,  1393,  1398,  1412,  1416,  1422,  1421,
    1517,  1520,  1526,  1547,  1553,  1557,  1559,  1564,  1566,  1567,
    1572,  1579,  1585,  1586,  1587,  1588,  1589,  1591,  1592,  1596,
    1600,  1594,  1639,  1642,  1637,  1663,  1664,  1665,  1668,  1672,
    1680,  1679,  1689,  1688,  1697,  1698,  1699,  1701,  1709,  1720,
    1723,  1728,  1735,  1742,  1748,  1768,  1773,  1779,  1782,  1784,
    1785,  1790,  1794,  1799,  1800,  1803,  1807,  1802,  1816,  1820,
    1815,  1828,  1831,  1835,  1830,  1844,  1848,  1843,  1856,  1858,
    1886,  1885,  1896,  1904,  1895,  1915,  1916,  1919,  1923,  1928,
    1933,  1932,  1948,  1953,  1954,  1959,  1960,  1965,  1966,  1967,
    1968,  1970,  1971,  1976,  1977,  1982,  1983,  1985,  1986,  1991,
    1992,  1993,  1994,  1996,  1997,  2002,  2003,  2008,  2012,  2017,
    2021,  2026,  2027,  2033,  2034,  2039,  2040,  2045,  2046,  2051,
    2052,  2057,  2061,  2066,  2067,  2068,  2070,  2075,  2076,  2077,
    2078,  2079,  2080,  2082,  2087,  2088,  2089,  2090,  2091,  2092,
    2094,  2099,  2100,  2101,  2103,  2116,  2117,  2118,  2120,  2136,
    2140,  2145,  2146,  2148,  2150,  2151,  2153,  2159,  2163,  2169,
    2172,  2173,  2177,  2182,  2183,  2185,  2186,  2191,  2192,  2195,
    2194,  2224,  2226,  2227,  2229,  2243,  2244,  2245,  2246,  2247,
    2248,  2249,  2250,  2251,  2252,  2253,  2254,  2257,  2256,  2266,
    2270,  2275,  2277,  2282,  2283,  2285,  2289,  2291,  2295,  2304,
    2310,  2311,  2316,  2317,  2318,  2319,  2320,  2321,  2322,  2324,
    2325,  2330,  2343,  2357,  2398,  2403,  2408,  2412,  2417,  2422,
    2427,  2457,  2461,  2468,  2470,  2471,  2473,  2474,  2475,  2509,
    2518,  2507,  2753,  2757,  2777,  2780,  2786,  2791,  2796,  2802,
    2805,  2810,  2817,  2821,  2827,  2841,  2847,  2864,  2869,  2883,
    2884,  2885,  2886,  2887,  2888,  2890,  2896,  2895,  2931,  2943,
    2948,  2953,  2964,  2968,  2964,  2974,  2976,  2980,  2982,  2997,
    3001,  3006,  3010,  3016,  3019,  3024,  3029,  3034,  3035,  3036,
    3037,  3039,  3040,  3041,  3042,  3047,  3083,  3084,  3085,  3086,
    3087,  3088,  3089,  3091,  3096,  3101,  3107,  3108,  3110,  3115,
    3120,  3125,  3130,  3148,  3149,  3151,  3156,  3161,  3165,  3170,
    3176,  3186,  3191,  3196,  3201,  3207,  3213,  3223,  3225,  3232,
    3239,  3241,  3243,  3247,  3249,  3250,  3252,  3258,  3260,  3262,
    3263,  3265
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "\"<string literal>\"",
  "\"<non-boundary text node>\"", "\"<string literal(XPath 2.0)>\"",
  "\"QName\"", "\"NCName\"", "ANY_LOCAL_NAME", "ANY_PREFIX",
  "\"<number literal>\"", "\"<number literal(XPath 2.0)>\"", "\"and\"",
  "\"'\"", "\"as\"", "\"ascending\"", "\":=\"", "\"at\"", "\"@\"",
  "\"attribute\"", "\"|\"", "\"base-uri\"", "\"</\"", "\"boundary-space\"",
  "\"by\"", "\"case\"", "\"castable\"", "\"cast\"", "\"collation\"",
  "\":\"", "\"::\"", "\",\"", "\"comment\"", "\"<!--\"",
  "\"construction\"", "\"copy-namespaces\"", "\"{\"", "\"}\"",
  "\"declare\"", "\"default\"", "\"descending\"", "\"div\"",
  "\"document\"", "\"document-node\"", "\"$\"", "\".\"", "\"..\"",
  "\"element\"", "\"else\"", "\"empty\"", "\"empty-sequence\"",
  "\"encoding\"", "\"eq\"", "\"unknown keyword\"", "\"every\"",
  "\"except\"", "\"external\"", "\">>\"", "\"for\"", "\"function\"",
  "\"ge\"", "\"=\"", "\">=\"", "\">\"", "\"<=\"", "\"<\"", "\"!=\"",
  "\"greatest\"", "\"gt\"", "\"idiv\"", "\"if\"", "\"import\"",
  "\"inherit\"", "\"in\"", "\"instance\"", "\"intersect\"", "\"is\"",
  "\"item\"", "\"lax\"", "\"[\"", "\"least\"", "\"le\"", "\"let\"",
  "\"(\"", "\"lt\"", "\"-\"", "\"mod\"", "\"module\"", "\"namespace\"",
  "\"ne\"", "\"node\"", "NO_ELEMENT_CONTENT", "\"no-inherit\"",
  "\"no-preserve\"", "\"of\"", "\"option\"", "\"order by\"", "\"ordered\"",
  "\"ordering\"", "\"order\"", "\"or\"", "\"<?\"", "\"+\"", "POSITION_SET",
  "\"#)\"", "\"(#\"", "\"<<\"", "\"preserve\"",
  "\"processing-instruction\"", "\"?\"", "\"/>\"", "\"\\\"\"", "\"]\"",
  "\"return\"", "\")\"", "\"satisfies\"", "\"schema\"",
  "\"schema-attribute\"", "\"schema-element\"", "\";\"", "\"/\"", "\"//\"",
  "\"some\"", "\"stable\"", "\"*\"", "\"strict\"", "\"strip\"", "SUCCESS",
  "COMMENT_CONTENT", "PI_CONTENT", "PI_TARGET", "\"text\"", "\"then\"",
  "\"to\"", "\"treat\"", "\"typeswitch\"", "\"union\"", "\"unordered\"",
  "\"validate\"", "\"variable\"", "\"version\"", "\"where\"", "\"xquery\"",
  "\"ancestor-or-self\"", "\"ancestor\"", "\"child\"",
  "\"descendant-or-self\"", "\"descendant\"", "\"following-sibling\"",
  "\"following\"", "\"preceding\"", "\"parent\"", "\"preceding-sibling\"",
  "\"self\"", "$accept", "Module", "VersionDecl", "Encoding", "MainModule",
  "LibraryModule", "ModuleDecl", "Prolog", "Setter", "Import", "Separator",
  "NamespaceDecl", "BoundarySpaceDecl", "BoundarySpacePolicy",
  "DefaultNamespaceDecl", "DeclareDefaultElementNamespace",
  "DeclareDefaultFunctionNamespace", "OptionDecl", "OrderingModeDecl",
  "OrderingMode", "EmptyOrderDecl", "OrderingEmptySequence",
  "CopyNamespacesDecl", "PreserveMode", "InheritMode",
  "DefaultCollationDecl", "BaseURIDecl", "SchemaImport", "SchemaPrefix",
  "ModuleImport", "ModuleNamespaceDecl", "FileLocations", "FileLocation",
  "VarDecl", "VariableValue", "ConstructionDecl", "ConstructionMode",
  "FunctionDecl", "@1", "ParamList", "Param", "FunctionBody",
  "EnclosedExpr", "QueryBody", "Expr", "ExpressionSequence", "ExprSingle",
  "FLWORExpr", "ForClause", "@2", "@3", "ForTail", "@4", "@5",
  "PositionalVar", "LetClause", "@6", "LetTail", "@7", "WhereClause",
  "OrderByClause", "OrderSpecList", "OrderSpec", "DirectionModifier",
  "EmptynessModifier", "CollationModifier", "OrderByInputOrder",
  "QuantifiedExpr", "SomeQuantificationExpr", "@8", "@9",
  "SomeQuantificationTail", "@10", "@11", "EveryQuantificationExpr", "@12",
  "@13", "EveryQuantificationTail", "@14", "@15", "SatisfiesClause",
  "TypeswitchExpr", "@16", "CaseClause", "@17", "@18", "CaseTail",
  "CaseVariable", "CaseDefault", "@19", "IfExpr", "OrExpr", "AndExpr",
  "ComparisonExpr", "RangeExpr", "AdditiveExpr", "AdditiveOperator",
  "MultiplicativeExpr", "MultiplyOperator", "UnionExpr",
  "IntersectExceptExpr", "UnionOperator", "IntersectOperator",
  "InstanceOfExpr", "TreatExpr", "CastableExpr", "CastExpr", "UnaryExpr",
  "UnaryOperator", "ValueExpr", "GeneralComp", "GeneralComparisonOperator",
  "ValueComp", "ValueComparisonOperator", "NodeComp", "NodeOperator",
  "ValidateExpr", "ValidationMode", "ExtensionExpr",
  "EnclosedOptionalExpr", "Pragmas", "Pragma", "PragmaContents",
  "PathExpr", "RelativePathExpr", "StepExpr", "FilteredAxisStep",
  "AxisStep", "ForwardStep", "@20", "NodeTestInAxisStep", "Axis",
  "AxisToken", "AbbrevForwardStep", "@21", "ReverseStep",
  "AbbrevReverseStep", "NodeTest", "NameTest", "WildCard", "FilterExpr",
  "PrimaryExpr", "Literal", "NumericLiteral", "VarRef", "VarName",
  "ParenthesizedExpr", "ContextItemExpr", "OrderingExpr", "FunctionCall",
  "FunctionArguments", "Constructor", "DirectConstructor",
  "DirElemConstructor", "@22", "@23", "DirElemConstructorTail",
  "DirAttributeList", "Attribute", "DirAttributeValue", "AttrValueContent",
  "DirElemContent", "DirCommentConstructor", "DirPIConstructor",
  "ComputedConstructor", "CompDocConstructor", "CompElemConstructor",
  "@24", "CompAttrConstructor", "CompTextConstructor",
  "CompCommentConstructor", "CompPIConstructor", "CompAttributeName",
  "@25", "@26", "CompElementName", "CompNameExpr", "CompPIName",
  "SingleType", "TypeDeclaration", "SequenceType", "OccurrenceIndicator",
  "ItemType", "AtomicType", "KindTest", "AnyKindTest", "DocumentTest",
  "AnyElementTest", "TextTest", "CommentTest", "PITest",
  "AnyAttributeTest", "AttributeTest", "SchemaAttributeTest",
  "ElementTest", "SchemaElementTest", "EmptyParanteses", "AttributeName",
  "ElementName", "TypeName", "FunctionName", "LexicalName", "PragmaName",
  "URILiteral", "StringLiteral", "QName", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   154,   155,   155,   156,   156,   157,   157,   158,   159,
     160,   161,   161,   161,   161,   161,   161,   161,   161,   162,
     162,   162,   162,   162,   162,   162,   163,   163,   164,   165,
     166,   167,   167,   168,   168,   169,   170,   171,   172,   173,
     173,   174,   175,   175,   176,   177,   177,   178,   178,   179,
     180,   181,   182,   182,   182,   183,   184,   184,   185,   185,
     186,   186,   187,   188,   188,   189,   190,   190,   192,   191,
     193,   193,   193,   194,   195,   195,   196,   197,   198,   198,
     199,   199,   200,   200,   200,   200,   200,   201,   201,   203,
     204,   202,   206,   207,   205,   205,   205,   205,   208,   208,
     210,   209,   212,   211,   211,   211,   211,   213,   213,   214,
     214,   215,   215,   216,   217,   217,   217,   218,   218,   219,
     219,   220,   220,   221,   221,   223,   224,   222,   226,   227,
     225,   225,   229,   230,   228,   232,   233,   231,   231,   234,
     236,   235,   238,   239,   237,   240,   240,   241,   241,   242,
     243,   242,   244,   245,   245,   246,   246,   247,   247,   247,
     247,   248,   248,   249,   249,   250,   250,   251,   251,   252,
     252,   252,   252,   253,   253,   254,   254,   255,   255,   256,
     256,   257,   257,   258,   258,   259,   259,   260,   260,   261,
     261,   262,   262,   263,   263,   263,   264,   265,   265,   265,
     265,   265,   265,   266,   267,   267,   267,   267,   267,   267,
     268,   269,   269,   269,   270,   271,   271,   271,   272,   273,
     273,   274,   274,   275,   276,   276,   277,   277,   277,   277,
     278,   278,   278,   279,   279,   280,   280,   281,   281,   283,
     282,   282,   284,   284,   285,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   288,   287,   287,
     287,   289,   290,   291,   291,   292,   292,   293,   293,   293,
     294,   294,   295,   295,   295,   295,   295,   295,   295,   296,
     296,   297,   297,   298,   299,   299,   300,   300,   301,   302,
     303,   304,   304,   304,   305,   305,   306,   306,   306,   308,
     309,   307,   310,   310,   311,   311,   312,   313,   313,   314,
     314,   314,   315,   315,   315,   315,   315,   316,   317,   318,
     318,   318,   318,   318,   318,   319,   321,   320,   322,   323,
     324,   325,   327,   328,   326,   326,   329,   329,   330,   331,
     331,   332,   332,   333,   333,   334,   334,   335,   335,   335,
     335,   336,   336,   336,   336,   337,   338,   338,   338,   338,
     338,   338,   338,   339,   340,   340,   341,   341,   342,   343,
     344,   344,   344,   345,   345,   346,   346,   346,   346,   346,
     347,   348,   348,   348,   348,   348,   349,   350,   351,   352,
     352,   353,   354,   354,   355,   355,   356,   356,   357,   358,
     358,   359
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     5,     0,     2,     2,     2,
       6,     0,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       4,     1,     1,     1,     1,     6,     6,     5,     4,     1,
       1,     5,     2,     2,     6,     1,     1,     1,     1,     5,
       4,     6,     0,     3,     3,     6,     0,     3,     0,     2,
       1,     3,     7,     1,     2,     4,     1,     1,     0,    10,
       0,     1,     3,     3,     1,     1,     3,     1,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     0,
       0,    10,     0,     0,    10,     1,     1,     1,     0,     3,
       0,     8,     0,     8,     1,     1,     1,     3,     5,     0,
       2,     3,     1,     4,     0,     1,     1,     0,     1,     0,
       2,     3,     2,     1,     1,     0,     0,     9,     0,     0,
       9,     1,     0,     0,     9,     0,     0,     9,     1,     2,
       0,     6,     0,     0,     8,     1,     1,     0,     3,     3,
       0,     6,     8,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     4,     1,     4,     1,     4,     1,     4,     1,
       2,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     2,     1,     2,     2,     2,     2,
       3,     2,     1,     4,     0,     1,     2,     2,     1,     1,
       1,     3,     3,     1,     1,     1,     4,     1,     1,     0,
       3,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     3,     2,     1,     2,
       4,     0,     1,     1,     1,     1,     1,     1,     1,     0,
       0,     8,     1,     5,     0,     2,     3,     3,     3,     0,
       2,     2,     0,     2,     2,     2,     2,     2,     3,     1,
       1,     1,     1,     1,     1,     2,     0,     4,     3,     2,
       2,     3,     0,     0,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     2,     2,     2,     0,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     4,     1,     1,     2,     2,
       2,     4,     4,     1,     1,     2,     4,     4,     6,     6,
       4,     2,     4,     4,     6,     6,     4,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,    11,     0,     1,     0,     3,     2,    11,
       0,   399,   400,     6,     0,     9,   401,   389,   268,   269,
     282,   281,   257,   332,     0,     0,     0,     0,     0,     0,
     288,   262,     0,     0,     0,     0,     0,     0,     0,     0,
     192,     0,    39,     0,   191,     0,     0,     0,     0,   228,
       0,     0,   267,     0,     0,    40,   215,   245,   246,   248,
     249,   250,   253,   251,   252,   255,   254,   256,    13,    15,
      14,    19,    12,    33,    34,    18,    23,     0,    24,    25,
      20,    21,    26,    27,    16,    22,    17,     8,    77,    79,
      78,    83,    87,    88,    84,   123,   124,    85,    86,    82,
     153,   155,   157,   161,   163,   167,   173,   175,   181,   183,
     185,   187,     0,   189,   159,   158,   160,   193,     0,   195,
       0,   222,   194,   229,   230,   233,   235,   237,   239,     0,
     241,   238,   261,   259,   263,   266,   234,   270,   272,   279,
     273,   274,   275,   277,   276,   278,   294,   296,   297,   298,
     295,   319,   320,   321,   322,   323,   324,   264,   362,   356,
     361,   360,   359,   260,   373,   374,   357,   358,   265,     0,
     280,   390,     0,     0,     0,     0,     0,     0,   338,     0,
       0,   335,   375,     0,   330,   369,   317,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   325,     0,   364,
     284,   283,   285,   389,     0,   326,   337,   381,   336,   390,
       0,     0,   395,   394,   299,     0,    56,    52,     0,   287,
       0,   363,     0,   396,   224,   397,   339,     0,   340,     0,
     370,     0,     0,   226,   227,     0,   329,   368,     0,   217,
     216,   289,     0,     0,     0,     0,   204,   213,   206,   197,
     199,   200,   201,   202,   198,   207,   211,   208,   209,   205,
     212,     0,     0,     0,   166,   165,     0,     0,   170,   171,
     172,   169,     0,   178,   177,     0,   180,   179,     0,     0,
       0,     0,     0,   190,   214,     0,   218,   221,     0,     0,
       0,     0,   244,     0,   291,     7,    28,     5,     0,     0,
       0,     0,     0,   258,     0,   387,     0,     0,   388,   328,
     333,     0,   398,    32,    31,     0,    67,    66,     0,    46,
      45,     0,     0,     0,     0,     0,   392,     0,   393,     0,
       0,     0,     0,     0,   366,   367,     0,     0,     0,   343,
     343,   304,     0,     0,     0,     0,     0,     0,   343,   286,
     318,     0,   225,     0,     0,   331,     0,     0,   343,     0,
      81,    80,   154,   156,   196,   203,   210,   162,   164,   168,
     174,   176,     0,     0,     0,     0,   219,     0,   231,   232,
       0,     0,   240,   242,   243,     0,   293,   292,     0,     0,
      76,     0,   376,     0,   377,   334,    50,    30,    65,     0,
       0,     0,     0,     0,     0,    70,     0,     0,    38,   343,
     365,     0,   382,     0,   383,   327,     0,     0,    98,   300,
       0,     0,    58,     0,     0,    58,     0,   223,   371,   372,
     380,   386,     0,   140,     0,     0,   182,   347,   351,   352,
     353,   355,   184,   186,   341,   188,   220,   236,   271,   290,
      10,   391,     0,     0,    47,    48,     0,    49,     0,     0,
      43,    42,    41,     0,     0,    71,     0,    37,     0,     0,
       0,   344,     0,     0,     0,     0,   305,     0,     0,    57,
       0,     0,    53,    54,     0,     0,     0,     0,   346,   354,
     348,   350,   349,   345,   342,   379,   378,    44,    35,    36,
     343,     0,    68,    29,     0,    63,     0,   385,   384,   132,
       0,     0,   304,     0,     0,    59,    60,    55,    51,   100,
     125,   147,   141,    73,    72,   343,    64,    62,   133,    99,
      89,     0,   309,   309,   306,     0,     0,   109,   126,     0,
       0,     0,     0,    90,   312,   302,   301,   309,     0,   309,
       0,   152,    61,     0,     0,     0,     0,   105,   106,   101,
     104,     0,     0,     0,     0,   142,    74,     0,    75,     0,
       0,   134,   138,   109,     0,   310,   308,   311,   307,     0,
     122,     0,   109,     0,   114,   110,   112,     0,   127,   131,
     148,     0,    69,     0,   139,     0,    96,    91,    97,    95,
     315,     0,   316,   313,   314,   343,   121,     0,   107,   115,
     116,   117,     0,     0,     0,   343,     0,     0,     0,     0,
     118,   119,   111,   343,   143,     0,   343,   303,     0,   108,
       0,   113,     0,     0,     0,    98,   102,   120,     0,     0,
     145,   144,   146,   135,     0,   109,   128,     0,     0,   136,
       0,   103,   129,   150,   149,     0,    92,     0,     0,   137,
      93,   130,     0,   109,   151,    94
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,   173,     7,     8,     9,    10,    68,    69,
     297,    70,    71,   315,    72,    73,    74,    75,    76,    77,
      78,   404,    79,   321,   456,    80,    81,    82,   347,    83,
     344,   481,   515,    84,   506,    85,   318,    86,   525,   464,
     465,   567,   547,    87,    88,    89,    90,    91,    92,   543,
     573,   597,   660,   663,   474,    93,   537,   559,   645,   560,
     561,   585,   586,   611,   621,   631,   562,    94,    95,   538,
     563,   588,   652,   657,    96,   528,   542,   571,   649,   655,
     572,    97,   487,   522,   591,   633,   641,   540,   642,   658,
      98,    99,   100,   101,   102,   103,   267,   104,   272,   105,
     106,   275,   278,   107,   108,   109,   110,   111,   112,   113,
     114,   261,   115,   262,   116,   263,   117,   118,   119,   286,
     120,   121,   351,   122,   123,   124,   125,   126,   127,   291,
     382,   128,   129,   130,   175,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   201,   141,   142,   143,   144,
     388,   145,   146,   147,   341,   475,   546,   419,   476,   534,
     548,   574,   148,   149,   150,   151,   152,   338,   153,   154,
     155,   156,   179,   180,   395,   205,   181,   229,   443,   417,
     436,   493,   437,   438,   157,   158,   159,   333,   160,   161,
     162,   163,   164,   165,   166,   167,   182,   307,   168,   452,
     169,   477,   224,   311,   170,   171
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -536
static const yytype_int16 yypact[] =
{
     -77,   -53,    98,    21,   222,  -536,    14,  -536,  -536,  -536,
     554,  -536,  -536,    60,   130,    61,  -536,    66,  -536,  -536,
    -536,  -536,  -536,   172,    36,    48,    50,   134,    97,   273,
    -536,  -536,    71,   160,   205,   279,   173,   -30,   220,   705,
    -536,   188,  -536,   143,  -536,   292,    76,   198,   204,  1309,
    1309,   248,  -536,    36,   213,  -536,    17,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,   134,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,   269,
     271,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,   212,
     293,  -536,   178,   -10,   123,  -536,   -12,    19,   235,   183,
     299,   300,  1158,  -536,  -536,  -536,  -536,  -536,   134,  -536,
       8,  -536,  -536,   195,  -536,   254,  -536,  -536,  -536,   304,
    -536,  -536,  -536,  -536,  -536,  -536,   256,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,   253,
    -536,   255,   222,   218,   278,   459,  1007,    27,  -536,   305,
     313,  -536,  -536,   228,  -536,  -536,  -536,   222,    96,   105,
     126,   137,   315,   338,   313,   -16,   302,  -536,    57,  -536,
    -536,  -536,  -536,  -536,    42,  -536,  -536,  -536,  -536,  -536,
     273,   273,  -536,  -536,  -536,  1007,   264,     2,   273,  -536,
     240,  -536,   231,  -536,   222,  -536,  -536,    22,  -536,   305,
    -536,   313,   313,   195,   195,   273,  -536,  -536,  1007,  -536,
    -536,  -536,  1007,  1007,  1158,  1158,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  1158,  1158,  1158,  -536,  -536,  1158,  1158,  -536,  -536,
    -536,  -536,  1158,  -536,  -536,  1158,  -536,  -536,  1158,   267,
     349,   357,   359,  -536,  -536,   856,  -536,  -536,  1309,  1309,
    1007,   713,  -536,  1007,  1007,  -536,  -536,  -536,   222,   188,
     294,   295,   188,  -536,   342,  -536,    16,   344,   270,  -536,
    -536,   218,  -536,  -536,  -536,   218,  -536,  -536,   218,  -536,
    -536,   350,   222,   297,   301,   334,  -536,   309,  -536,   332,
     222,   218,   273,   286,  -536,  -536,    25,    32,   305,   387,
     387,  -536,   289,   397,   222,   358,   400,   222,   387,  -536,
    -536,   310,  -536,   303,   307,  -536,   308,   312,   387,   314,
    -536,  -536,   293,  -536,  -536,  -536,  -536,   168,   123,  -536,
    -536,  -536,   363,   363,   313,   313,  -536,   371,  -536,  -536,
     306,   336,  -536,  -536,  -536,   317,   269,   271,   316,   218,
    -536,   313,  -536,   313,  -536,  -536,  -536,  -536,  -536,    89,
     218,   222,   222,    13,   218,   383,   222,   218,  -536,   387,
    -536,   313,  -536,   313,  -536,  -536,   363,   360,   398,   279,
     311,   375,   415,   351,   376,   415,   425,  -536,  -536,  -536,
    -536,  -536,   369,  -536,   188,   188,  -536,   -42,  -536,  -536,
    -536,  -536,  -536,  -536,   335,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,   337,   341,  -536,  -536,   218,  -536,   218,   218,
    -536,  -536,  -536,   273,    39,  -536,   218,  -536,   102,   346,
     347,  -536,  1007,   402,   377,   345,  -536,   388,  1007,  -536,
     222,   218,  -536,  -536,   218,  1007,  1007,   427,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
     387,   383,  -536,  -536,  1007,  -536,   218,  -536,  -536,  -536,
     273,  1007,  -536,     3,   408,   431,  -536,  -536,  -536,  -536,
    -536,   414,  -536,  -536,  -536,   387,  -536,  -536,  -536,  -536,
    -536,    62,   192,   192,  -536,  1007,   222,     6,  -536,   313,
     363,   155,    -5,  -536,  -536,  -536,  -536,   192,   450,   192,
     353,  -536,  -536,   426,   445,   373,  1007,  -536,  -536,  -536,
    -536,   361,  1007,    12,   461,  -536,  -536,   218,  -536,   433,
    1007,  -536,  -536,    75,   196,  -536,  -536,  -536,  -536,   273,
    -536,   454,   -20,  1007,   211,   448,  -536,   438,  -536,  -536,
    -536,   370,  -536,   273,  -536,   440,  -536,  -536,  -536,  -536,
    -536,   313,  -536,  -536,  -536,   387,  -536,   374,  -536,  -536,
    -536,   334,  1007,   273,  1007,   387,   273,   423,   472,  1007,
    -536,   462,  -536,   387,  -536,   416,   387,  -536,  1007,  -536,
     222,  -536,   419,   209,  1007,   398,  -536,  -536,  1007,    18,
    -536,  -536,  -536,  -536,   420,     6,  -536,   313,  1007,  -536,
    1007,  -536,  -536,  -536,  -536,    -5,  -536,    12,   382,  -536,
    -536,  -536,  1007,    75,  -536,  -536
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -536,  -536,  -536,  -536,  -536,  -536,  -536,   487,  -536,  -536,
     -94,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,   318,
    -536,  -114,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,    73,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
      -2,  -536,   -22,  -536,   -33,   206,  -203,  -536,  -528,  -536,
    -536,  -162,  -536,  -536,  -132,  -523,  -536,  -141,  -536,  -535,
     -75,  -536,  -107,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -149,  -536,  -536,  -536,  -536,  -536,  -146,  -536,  -536,
    -531,  -536,  -536,  -123,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,   268,   266,    15,   250,  -536,   251,  -536,   245,
     246,  -536,  -536,   242,  -536,  -536,  -536,   412,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -176,
    -536,   405,  -536,  -536,   274,    40,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,  -536,  -157,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -159,  -536,  -536,  -536,  -536,
    -536,  -536,   -48,  -536,  -536,  -536,  -536,    23,  -536,  -536,
    -275,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,  -536,
    -536,  -536,  -536,  -536,  -536,  -536,   495,  -536,   156,  -312,
    -362,  -536,  -536,   -44,  -350,  -536,  -536,  -536,  -536,  -536,
    -536,  -272,  -536,  -536,   339,   340,   -11,  -536,   -17,  -148,
     348,   498,  -536,  -286,    -4,   -25
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -394
static const yytype_int16 yytable[] =
{
      13,   178,   184,   309,   202,   197,   220,   209,   273,   557,
     178,   442,   389,   185,   558,   208,   532,   199,   303,   384,
     225,   207,   439,   439,   228,    11,   569,    12,   418,   353,
     221,   236,   589,    16,   203,   230,   426,   553,   599,   360,
     361,   345,   237,   587,   285,   596,   432,   391,    16,   203,
     598,   339,   340,   355,   471,   241,   411,   216,   422,   348,
     490,   425,   647,   413,    34,     1,   439,   491,   212,   213,
     501,   187,   176,   188,   276,   264,   358,    16,   203,   554,
     460,    42,   492,   226,   189,   190,   217,     4,    38,   191,
     346,   387,   265,   461,   277,   239,   284,   468,     5,    26,
     440,   440,    14,   555,   300,   554,   595,   176,     6,   192,
     570,   172,   176,    45,   533,   458,   459,   557,   504,   183,
     466,    55,   558,   266,   274,   544,   589,   570,   599,   555,
     392,   648,    37,    34,   383,   596,   305,   174,   193,   412,
     598,   305,   240,   304,   440,   194,   414,   556,   195,  -392,
     209,   306,   209,   502,   204,   209,   305,    38,   505,   227,
     308,   454,   415,   310,   268,   322,   336,   328,   295,   209,
     176,   305,   545,   409,   554,    48,   186,   330,   565,   209,
     198,   455,   342,   312,   323,   202,   202,   337,   523,   196,
     439,   176,   269,   202,   516,    11,   324,    12,   555,    11,
     600,    12,  -247,   313,   210,   359,   209,   209,   176,   270,
     202,   566,   316,   541,   356,   357,   556,   396,   601,   319,
     352,   397,   314,   354,   398,    11,   609,    12,   176,    25,
     246,   317,   176,   320,   521,   247,   325,   408,   248,   249,
     250,   251,   252,   253,   254,   453,   255,   271,   639,   211,
     552,   610,   377,   264,   256,   177,   215,   380,   550,   257,
     385,    35,   258,   469,   218,   470,   209,   259,   440,   509,
     265,   183,   575,   222,   577,   514,   364,   365,   366,    16,
     200,   231,   519,   520,   260,   212,   213,   232,   185,   207,
     230,   237,   235,   618,   312,   450,   238,    43,    16,   223,
     242,   526,   243,   625,   500,   245,   457,   202,   530,   279,
     462,   632,   244,   467,   635,   288,   289,   280,   400,    16,
     203,    16,   326,   233,   234,   281,   407,   282,   378,   379,
     444,   444,   551,   290,   292,   293,   294,   296,  -393,   298,
     312,   285,   305,   312,   637,   329,   332,   209,   209,   209,
     209,   529,   343,   582,   349,   441,   441,   441,   441,   584,
     350,   372,   497,   373,   498,   499,   209,   594,   209,    16,
     203,   374,   503,   375,   451,   393,   451,   204,   227,   390,
     608,   399,   381,   403,   394,   401,   209,   517,   209,   402,
     518,   209,   405,   406,   451,   299,   451,   312,   312,   441,
     410,   416,   312,   420,   421,   423,    28,   424,   446,   584,
     300,   624,   527,   434,   427,   473,   629,   428,   447,   177,
     605,   429,   430,   488,   489,   636,   431,   463,   433,   448,
     449,   643,   480,   472,   615,   646,   479,   483,   202,   482,
     435,   485,   486,   478,   494,   654,   510,   656,   512,   513,
     511,   495,   521,    41,   623,   496,   535,   626,   539,   664,
     507,   508,   536,   576,   578,    16,   203,    18,    19,   580,
     579,   301,   581,   592,   583,   590,   312,   593,   606,   612,
      47,    48,   613,   614,   616,   202,   627,   619,   628,   634,
     630,   299,   638,   650,   302,   662,    15,   620,   484,   524,
     386,   665,    28,   644,   651,   622,   300,   607,   661,   659,
     640,   363,   362,   331,   209,   209,   367,   369,   368,   568,
     371,   370,   564,   441,   283,   287,   603,   206,   549,   549,
       0,   445,   312,   214,     0,   531,     0,   334,   335,     0,
     327,     0,     0,   549,     0,   549,     0,     0,     0,    41,
       0,     0,   602,     0,   202,     0,     0,    11,     0,    12,
      16,    17,    18,    19,    20,    21,     0,   301,   202,     0,
     604,     0,    22,    23,     0,     0,   209,    48,     0,     0,
       0,     0,     0,    52,   617,     0,    24,    25,   202,     0,
     302,   202,    26,     0,     0,     0,    27,    28,    29,    30,
      31,    32,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,    34,     0,     0,     0,     0,     0,     0,    35,
       0,     0,   209,     0,    36,    37,   312,     0,     0,     0,
     653,     0,     0,     0,     0,     0,    38,    39,     0,    40,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,    42,     0,     0,     0,    43,    44,     0,     0,    45,
       0,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,    47,    48,     0,    49,    50,    51,     0,    52,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,    54,
       0,    55,    56,     0,     0,     0,     0,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    11,     0,
      12,    16,    17,    18,    19,    20,    21,     0,     0,    16,
     203,    18,    19,    22,    23,     0,     0,     0,     0,     0,
       0,     0,   381,     0,     0,     0,     0,    24,    25,     0,
       0,     0,     0,     0,     0,   299,     0,    27,    28,    29,
      30,    31,    32,     0,     0,     0,    28,     0,     0,    33,
     300,     0,     0,    34,     0,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    38,    39,     0,
      40,     0,     0,     0,     0,    41,     0,     0,     0,     0,
       0,     0,    42,    41,     0,     0,    43,    44,     0,     0,
      45,     0,     0,    46,     0,     0,     0,     0,     0,   219,
       0,   301,    47,    48,     0,    49,    50,    51,     0,    52,
      47,    48,     0,     0,     0,     0,    53,    52,     0,     0,
      54,     0,    55,    56,   302,     0,     0,     0,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    11,
       0,    12,    16,    17,    18,    19,    20,    21,     0,     0,
       0,     0,     0,     0,    22,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
       0,     0,     0,   376,     0,     0,     0,     0,    27,    28,
      29,    30,    31,    32,     0,     0,     0,     0,     0,     0,
      33,     0,     0,     0,    34,     0,     0,     0,     0,     0,
       0,    35,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    38,    39,
       0,    40,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,    42,     0,     0,     0,    43,    44,     0,
       0,    45,     0,     0,    46,     0,     0,     0,     0,     0,
       0,     0,     0,    47,    48,     0,    49,    50,    51,     0,
      52,     0,     0,     0,     0,     0,     0,    53,     0,     0,
       0,    54,     0,    55,    56,     0,     0,     0,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      11,     0,    12,    16,    17,    18,    19,    20,    21,     0,
       0,     0,     0,     0,     0,    22,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,     0,     0,     0,     0,     0,     0,     0,     0,    27,
      28,    29,    30,    31,    32,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    38,
      39,     0,    40,     0,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,    42,     0,     0,     0,    43,    44,
       0,     0,    45,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,    47,    48,     0,    49,    50,    51,
       0,    52,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,    54,     0,    55,    56,     0,     0,     0,     0,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    11,     0,    12,    16,    17,    18,    19,    20,    21,
       0,     0,     0,     0,     0,     0,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    30,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    35,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,     0,    40,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,    42,     0,     0,     0,    43,
      44,     0,     0,    45,     0,     0,    46,     0,     0,     0,
       0,     0,     0,     0,     0,    47,    48,     0,    49,    50,
       0,     0,    52,     0,     0,     0,     0,     0,     0,    53,
       0,     0,     0,     0,     0,    55,    56,     0,     0,     0,
       0,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    11,     0,    12,    16,    17,    18,    19,    20,
      21,     0,     0,     0,     0,     0,     0,    22,    23,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,     0,     0,     0,     0,     0,
       0,    27,    28,    29,    30,    31,    32,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,    42,     0,     0,     0,
      43,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,    47,    48,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,    55,     0,     0,     0,
       0,     0,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

static const yytype_int16 yycheck[] =
{
       4,    23,    24,   179,    29,    27,    39,    32,    20,   537,
      32,   373,   298,    24,   537,    32,    13,    28,   175,   291,
      45,    32,   372,   373,    46,     3,    31,     5,   340,     7,
      41,    53,   563,     6,     7,    46,   348,    31,   573,   242,
     243,    39,    53,    31,    36,   573,   358,    31,     6,     7,
     573,   210,   211,   229,   416,    77,    31,    87,   344,   218,
     102,   347,    44,    31,    58,   142,   416,   109,     6,     7,
      31,    21,    36,    23,    55,    85,   235,     6,     7,    99,
      67,    97,   124,     7,    34,    35,   116,   140,    82,    39,
      88,   294,   102,    80,    75,    78,   118,   409,     0,    38,
     372,   373,    88,   123,    47,    99,    31,    36,    87,    59,
     115,    51,    36,   105,   111,   401,   402,   645,    16,    83,
     406,   137,   645,   133,   136,    63,   657,   115,   663,   123,
     114,   113,    71,    58,   291,   663,   114,     7,    88,   114,
     663,   114,   125,   176,   416,    95,   114,   141,    98,    83,
     175,   124,   177,   114,    83,   180,   114,    82,    56,    83,
     177,    72,   338,   180,    41,    28,   124,   192,   172,   194,
      36,   114,   110,   332,    99,   118,   128,   194,   540,   204,
      83,    92,   215,   187,    47,   210,   211,   204,   500,   139,
     540,    36,    69,   218,   480,     3,    59,     5,   123,     3,
       4,     5,    30,   107,    44,   238,   231,   232,    36,    86,
     235,    56,   107,   525,   231,   232,   141,   311,    22,    93,
     224,   315,   126,   227,   318,     3,    15,     5,    36,    33,
      52,   126,    36,   107,    25,    57,    99,   331,    60,    61,
      62,    63,    64,    65,    66,   393,    68,   124,    39,    44,
     536,    40,   285,    85,    76,    83,    83,   290,   533,    81,
     293,    65,    84,   411,    44,   413,   291,    89,   540,   472,
     102,    83,   547,   130,   549,   478,   261,   262,   263,     6,
       7,    83,   485,   486,   106,     6,     7,    83,   299,   300,
     301,   302,    44,   605,   298,   389,    83,   101,     6,     7,
      31,   504,    31,   615,   463,    12,   400,   332,   511,    74,
     404,   623,   100,   407,   626,   120,   121,   134,   322,     6,
       7,     6,     7,    49,    50,    26,   330,    27,   288,   289,
     374,   375,   535,    79,    30,    79,    83,   119,    83,    61,
     344,    36,   114,   347,   630,     7,    44,   372,   373,   374,
     375,   510,    88,   556,   114,   372,   373,   374,   375,   562,
     129,    94,   456,    14,   458,   459,   391,   570,   393,     6,
       7,    14,   466,    14,   391,    31,   393,    83,    83,    37,
     583,    31,    19,    49,   114,    88,   411,   481,   413,    88,
     484,   416,    83,    61,   411,    32,   413,   401,   402,   416,
     114,    14,   406,   114,     7,    47,    43,     7,    37,   612,
      47,   614,   506,    50,   104,    17,   619,   114,   112,    83,
     579,   114,   114,   434,   435,   628,   114,    44,   114,   112,
     114,   634,    17,    73,   593,   638,    61,    61,   463,    88,
      77,    16,    73,   132,   109,   648,    44,   650,   103,    61,
      73,   114,    25,    90,   613,   114,    48,   616,    44,   662,
     114,   114,    31,    13,   111,     6,     7,     8,     9,    24,
      44,   108,    99,   567,   113,    14,   480,    44,    24,    31,
     117,   118,    44,   113,    44,   510,    63,   113,    16,    73,
      28,    32,    73,    73,   131,   113,     9,   611,   425,   501,
     294,   663,    43,   635,   645,   612,    47,   582,   657,   655,
     633,   245,   244,   195,   539,   540,   266,   272,   267,   541,
     278,   275,   539,   540,   112,   120,   574,    32,   532,   533,
      -1,   375,   536,    35,    -1,   512,    -1,   198,   198,    -1,
     192,    -1,    -1,   547,    -1,   549,    -1,    -1,    -1,    90,
      -1,    -1,   574,    -1,   579,    -1,    -1,     3,    -1,     5,
       6,     7,     8,     9,    10,    11,    -1,   108,   593,    -1,
     574,    -1,    18,    19,    -1,    -1,   601,   118,    -1,    -1,
      -1,    -1,    -1,   124,   601,    -1,    32,    33,   613,    -1,
     131,   616,    38,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,   647,    -1,    70,    71,   630,    -1,    -1,    -1,
     647,    -1,    -1,    -1,    -1,    -1,    82,    83,    -1,    85,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,    -1,    -1,   105,
      -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,    -1,   120,   121,   122,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,   135,
      -1,   137,   138,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,     3,    -1,
       5,     6,     7,     8,     9,    10,    11,    -1,    -1,     6,
       7,     8,     9,    18,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    42,    43,    44,
      45,    46,    47,    -1,    -1,    -1,    43,    -1,    -1,    54,
      47,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,    -1,
      85,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,
      -1,    -1,    97,    90,    -1,    -1,   101,   102,    -1,    -1,
     105,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,   114,
      -1,   108,   117,   118,    -1,   120,   121,   122,    -1,   124,
     117,   118,    -1,    -1,    -1,    -1,   131,   124,    -1,    -1,
     135,    -1,   137,   138,   131,    -1,    -1,    -1,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,     3,
      -1,     5,     6,     7,     8,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    18,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    42,    43,
      44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,    83,
      -1,    85,    -1,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      -1,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,    -1,
      -1,   105,    -1,    -1,   108,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   117,   118,    -1,   120,   121,   122,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,
      -1,   135,    -1,   137,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       3,    -1,     5,     6,     7,     8,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    18,    19,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,
      -1,    54,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    82,
      83,    -1,    85,    -1,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
      -1,    -1,   105,    -1,    -1,   108,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   117,   118,    -1,   120,   121,   122,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,
      -1,    -1,   135,    -1,   137,   138,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,     3,    -1,     5,     6,     7,     8,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    -1,    -1,    -1,    -1,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,    -1,    -1,   105,    -1,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,    -1,   120,   121,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,   131,
      -1,    -1,    -1,    -1,    -1,   137,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     3,    -1,     5,     6,     7,     8,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    45,    46,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      -1,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   117,   118,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
     131,    -1,    -1,    -1,    -1,    -1,   137,    -1,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   142,   155,   156,   140,     0,    87,   158,   159,   160,
     161,     3,     5,   358,    88,   161,     6,     7,     8,     9,
      10,    11,    18,    19,    32,    33,    38,    42,    43,    44,
      45,    46,    47,    54,    58,    65,    70,    71,    82,    83,
      85,    90,    97,   101,   102,   105,   108,   117,   118,   120,
     121,   122,   124,   131,   135,   137,   138,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   162,   163,
     165,   166,   168,   169,   170,   171,   172,   173,   174,   176,
     179,   180,   181,   183,   187,   189,   191,   197,   198,   199,
     200,   201,   202,   209,   221,   222,   228,   235,   244,   245,
     246,   247,   248,   249,   251,   253,   254,   257,   258,   259,
     260,   261,   262,   263,   264,   266,   268,   270,   271,   272,
     274,   275,   277,   278,   279,   280,   281,   282,   285,   286,
     287,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   300,   301,   302,   303,   305,   306,   307,   316,   317,
     318,   319,   320,   322,   323,   324,   325,   338,   339,   340,
     342,   343,   344,   345,   346,   347,   348,   349,   352,   354,
     358,   359,    51,   157,     7,   288,    36,    83,   196,   326,
     327,   330,   350,    83,   196,   350,   128,    21,    23,    34,
      35,    39,    59,    88,    95,    98,   139,   196,    83,   350,
       7,   299,   359,     7,    83,   329,   330,   350,   352,   359,
      44,    44,     6,     7,   355,    83,    87,   116,    44,   114,
     198,   350,   130,     7,   356,   359,     7,    83,   196,   331,
     350,    83,    83,   278,   278,    44,   196,   350,    83,    78,
     125,   196,    31,    31,   100,    12,    52,    57,    60,    61,
      62,    63,    64,    65,    66,    68,    76,    81,    84,    89,
     106,   265,   267,   269,    85,   102,   133,   250,    41,    69,
      86,   124,   252,    20,   136,   255,    55,    75,   256,    74,
     134,    26,    27,   261,   196,    36,   273,   275,   120,   121,
      79,   283,    30,    79,    83,   358,   119,   164,    61,    32,
      47,   108,   131,   291,   198,   114,   124,   351,   352,   273,
     352,   357,   358,   107,   126,   167,   107,   126,   190,    93,
     107,   177,    28,    47,    59,    99,     7,   354,   359,     7,
     352,   173,    44,   341,   348,   349,   124,   352,   321,   299,
     299,   308,   198,    88,   184,    39,    88,   182,   299,   114,
     129,   276,   358,     7,   358,   273,   352,   352,   299,   198,
     200,   200,   246,   247,   248,   248,   248,   249,   251,   253,
     254,   257,    94,    14,    14,    14,    37,   198,   279,   279,
     198,    19,   284,   291,   345,   198,   199,   200,   304,   357,
      37,    31,   114,    31,   114,   328,   164,   164,   164,    31,
     358,    88,    88,    49,   175,    83,    61,   358,   164,   299,
     114,    31,   114,    31,   114,   273,    14,   333,   333,   311,
     114,     7,   357,    47,     7,   357,   333,   104,   114,   114,
     114,   114,   333,   114,    50,    77,   334,   336,   337,   338,
     345,   352,   334,   332,   337,   332,    37,   112,   112,   114,
     164,   352,   353,   353,    72,    92,   178,   164,   357,   357,
      67,    80,   164,    44,   193,   194,   357,   164,   333,   353,
     353,   334,    73,    17,   208,   309,   312,   355,   132,    61,
      17,   185,    88,    61,   185,    16,    73,   236,   350,   350,
     102,   109,   124,   335,   109,   114,   114,   164,   164,   164,
     299,    31,   114,   164,    16,    56,   188,   114,   114,   200,
      44,    73,   103,    61,   200,   186,   357,   164,   164,   200,
     200,    25,   237,   333,   194,   192,   200,   164,   229,   299,
     200,   311,    13,   111,   313,    48,    31,   210,   223,    44,
     241,   333,   230,   203,    63,   110,   310,   196,   314,   358,
     314,   200,   357,    31,    99,   123,   141,   202,   209,   211,
     213,   214,   220,   224,   352,   334,    56,   195,   196,    31,
     115,   231,   234,   204,   315,   314,    13,   314,   111,    44,
      24,    99,   200,   113,   200,   215,   216,    31,   225,   234,
      14,   238,   164,    44,   200,    31,   202,   205,   209,   213,
       4,    22,   196,   306,   358,   299,    24,   214,   200,    15,
      40,   217,    31,    44,   113,   299,    44,   352,   333,   113,
     175,   218,   216,   299,   200,   333,   299,    63,    16,   200,
      28,   219,   333,   239,    73,   333,   200,   357,    73,    39,
     237,   240,   242,   200,   208,   212,   200,    44,   113,   232,
      73,   211,   226,   352,   200,   233,   200,   227,   243,   231,
     206,   225,   113,   207,   200,   205
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, parseInfo, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, parseInfo)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, parseInfo); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParserContext *const parseInfo)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, parseInfo)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    ParserContext *const parseInfo;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (parseInfo);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParserContext *const parseInfo)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, parseInfo)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    ParserContext *const parseInfo;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, parseInfo);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ParserContext *const parseInfo)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, parseInfo)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    ParserContext *const parseInfo;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , parseInfo);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, parseInfo); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ParserContext *const parseInfo)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, parseInfo)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    ParserContext *const parseInfo;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (parseInfo);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (ParserContext *const parseInfo);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (ParserContext *const parseInfo)
#else
int
yyparse (parseInfo)
    ParserContext *const parseInfo;
#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
/* Line 1269 of yacc.c.  */
#line 927 "../../sdk/QueryTransformParser.ypp"
    {

/* Supress more compiler warnings about unused defines. */
#if    defined(YYNNTS)              \
    or defined(yyerrok)             \
    or defined(YYNSTATES)           \
    or defined(YYRHSLOC)            \
    or defined(YYRECOVERING)        \
    or defined(YYFAIL)              \
    or defined(YYERROR)             \
    or defined(YYNRULES)            \
    or defined(YYBACKUP)            \
    or defined(YYMAXDEPTH)          \
    or defined(yyclearin)           \
    or defined(YYERRCODE)           \
    or defined(YY_LOCATION_PRINT)   \
    or defined(YYLLOC_DEFAULT)
#endif

        if((yyvsp[(3) - (5)].sval) != QLatin1String("1.0"))
        {
            const ReflectYYLTYPE ryy((yyloc), parseInfo);

            parseInfo->staticContext->error(tr("Version %1 is not supported. The supported "
                                               "XQuery version is 1.0.")
                                               .arg(formatData((yyvsp[(3) - (5)].sval))),
                                            ReportContext::XQST0031, &ryy);
        }
    }
    break;

  case 7:
/* Line 1269 of yacc.c.  */
#line 959 "../../sdk/QueryTransformParser.ypp"
    {
        const QRegExp encNameRegExp(QLatin1String("[A-Za-z][A-Za-z0-9._\\-]*"));

        if(!encNameRegExp.exactMatch((yyvsp[(2) - (2)].sval)))
        {
            parseInfo->staticContext->error(tr("The encoding %1 is invalid, it may "
                                               "only consist of Latin characters, "
                                               "no whitespace, and must match the "
                                               "regular expression %2.")
                                               .arg(formatKeyword((yyvsp[(2) - (2)].sval)),
                                               formatExpression(encNameRegExp.pattern())),
                                            ReportContext::XQST0087, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 8:
/* Line 1269 of yacc.c.  */
#line 975 "../../sdk/QueryTransformParser.ypp"
    {
        /* First, the UserFunction callsites aren't bound yet, so bind them(if possible!). */

        const UserFunctionCallsite::List::const_iterator cend(parseInfo->userFunctionCallsites.constEnd());
        UserFunctionCallsite::List::const_iterator cit(parseInfo->userFunctionCallsites.constBegin());

        qDebug() << "About to iterate callsites.." << endl;
        for(; cit != cend; ++cit) /* For each callsite. */
        {
            qDebug() << "Walking through a callsite.." << endl;
            const UserFunctionCallsite::Ptr callsite(*cit);
            Q_ASSERT(callsite);
            const UserFunction::List::const_iterator end(parseInfo->userFunctions.constEnd());
            UserFunction::List::const_iterator it(parseInfo->userFunctions.constBegin());

            for(; it != end; ++it) /* For each UserFunction. */
            {
                const FunctionSignature::Ptr sign((*it)->signature());
                Q_ASSERT(sign);

                if(callsite->isSignatureValid(sign))
                {
                    callsite->setSource((*it)->body(), sign, (*it)->expressionSlotOffset(), (*it)->argumentDeclarations());
                    break;
                }
            }
            qDebug() << "Continuing.." << endl;
            if(it == end)
            {
                parseInfo->staticContext->error(tr("No function with signature %1 is available")
                                                   .arg(formatFunction(callsite)),
                                                ReportContext::XPST0017, fromYYLTYPE((yyloc), parseInfo));
            }
        }

        /* Mark callsites in UserFunction bodies as recursive, if they are. */
        qDebug() << "About to check callsite circularity" << endl;
        const UserFunction::List::const_iterator fend(parseInfo->userFunctions.constEnd());
        UserFunction::List::const_iterator fit(parseInfo->userFunctions.constBegin());
        for(; fit != fend; ++fit)
        {
            //qDebug() << "FUNCTION CIRCULARITY CHECK START: "
                           //<< (*fit)->signature()->displayName() << endl;
            FunctionSignature::List signList;
            signList.append((*fit)->signature());
            checkCallsiteCircularity(signList, (*fit)->body(), parseInfo);
            //qDebug() << "FUNCTION CIRCULARITY CHECK END" << endl;
        }

        /* Now, check all global variables for circularity.
         * This is done backwards because global variables are only in scope below them. */
        const VariableDeclaration::List::const_iterator start(parseInfo->declaredVariables.constBegin());
        VariableDeclaration::List::const_iterator it(parseInfo->declaredVariables.constEnd());

        while(it != start)
        {
            --it;
            qDebug() << "VARIABLE CIRCULARITY CHECK START" << endl;
            if((*it)->type != VariableDeclaration::ExpressionVariable)
                continue; /* We want to ignore 'external' variables. */

            FunctionSignature::List signList;
            checkVariableCircularity(*it, (*it)->expression(), signList, parseInfo);
            finalizePushedVariable(parseInfo); /* Warn if it's unused. */
            qDebug() << "VARIABLE CIRCULARITY CHECK END" << endl;
        }

        parseInfo->queryBody = (yyvsp[(2) - (2)].expr);
    }
    break;

  case 10:
/* Line 1269 of yacc.c.  */
#line 1048 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO add to namespace context
        parseInfo->moduleNamespace = parseInfo->staticContext->namePool()->allocateNamespace((yyvsp[(3) - (6)].sval));
    }
    break;

  case 29:
/* Line 1269 of yacc.c.  */
#line 1076 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(3) - (6)].sval) == QLatin1String("xml") || (yyvsp[(3) - (6)].sval) == QLatin1String("xmlns"))
        {
            parseInfo->staticContext->error(tr("It is not possible to redeclare prefix %1.")
                                               .arg(formatKeyword((yyvsp[(3) - (6)].sval))),
                                            ReportContext::XQST0070, fromYYLTYPE((yyloc), parseInfo));
        }
        else if((yyvsp[(5) - (6)].sval) == CommonNamespaces::XML)
        {
            parseInfo->staticContext->error(tr("No prefix can be declared that binds the "
                                               "namespace %1. It is by default bound to "
                                               "the %2-prefix.")
                                               .arg(formatURI(CommonNamespaces::XML))
                                               .arg(formatKeyword("xml")),
                                            ReportContext::XQST0070, fromYYLTYPE((yyloc), parseInfo));
        }
        else if(parseInfo->declaredPrefixes.contains((yyvsp[(3) - (6)].sval)))
        {
            /* This includes the case where the user has bound a default prefix(such
             * as 'local') and now tries to do it again. */
            parseInfo->staticContext->error(tr("Prefix %1 is already declared in the prolog.")
                                               .arg(formatKeyword((yyvsp[(3) - (6)].sval))),
                                            ReportContext::XQST0033, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->declaredPrefixes.append((yyvsp[(3) - (6)].sval));

            if((yyvsp[(5) - (6)].sval).isEmpty())
            {
                qDebug() << "undeclaring prefix..";
                parseInfo->staticContext->namespaceBindings()->addBinding(NamespaceBinding(parseInfo->staticContext->namePool()->allocatePrefix((yyvsp[(3) - (6)].sval)),
                                                                                           StandardNamespaces::UndeclarePrefix));
            }
            else
            {
                qDebug() << "declaring prefix..";
                parseInfo->staticContext->namespaceBindings()->addBinding(parseInfo->staticContext->namePool()->allocateBinding((yyvsp[(3) - (6)].sval), (yyvsp[(5) - (6)].sval)));
            }
        }
    }
    break;

  case 30:
/* Line 1269 of yacc.c.  */
#line 1119 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::BoundarySpaceDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare boundary-space"),
                                            ReportContext::XQST0068, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->staticContext->setBoundarySpacePolicy((yyvsp[(3) - (4)].enums.boundarySpacePolicy));
            parseInfo->registerDeclaration(ParserContext::BoundarySpaceDecl);
        }
    }
    break;

  case 31:
/* Line 1269 of yacc.c.  */
#line 1133 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.boundarySpacePolicy) = StaticContext::BSPStrip;
    }
    break;

  case 32:
/* Line 1269 of yacc.c.  */
#line 1138 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.boundarySpacePolicy) = StaticContext::BSPPreserve;
    }
    break;

  case 35:
/* Line 1269 of yacc.c.  */
#line 1147 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::DeclareDefaultElementNamespace))
        {
            parseInfo->staticContext->error(prologMessage("declare default element namespace"),
                                            ReportContext::XQST0066, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->staticContext->namespaceBindings()->addBinding(NamespaceBinding(StandardPrefixes::empty,
                                                                                       parseInfo->staticContext->namePool()->allocateNamespace((yyvsp[(5) - (6)].sval))));
            parseInfo->registerDeclaration(ParserContext::DeclareDefaultElementNamespace);
        }
    }
    break;

  case 36:
/* Line 1269 of yacc.c.  */
#line 1163 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::DeclareDefaultFunctionNamespace))
        {
            parseInfo->staticContext->error(prologMessage("declare default function namespace"),
                                            ReportContext::XQST0066, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->staticContext->setDefaultFunctionNamespace((yyvsp[(5) - (6)].sval));
            parseInfo->registerDeclaration(ParserContext::DeclareDefaultFunctionNamespace);
        }
    }
    break;

  case 37:
/* Line 1269 of yacc.c.  */
#line 1177 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(3) - (5)].qName).prefix() == StandardPrefixes::empty)
        {
            parseInfo->staticContext->error(tr("The name of an option must have a prefix. "
                                               "There is no default namespace for options."),
                                            ReportContext::XPST0081, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 38:
/* Line 1269 of yacc.c.  */
#line 1187 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::OrderingModeDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare ordering"),
                                            ReportContext::XQST0065, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->registerDeclaration(ParserContext::OrderingModeDecl);
            parseInfo->staticContext->setOrderingMode((yyvsp[(3) - (4)].enums.orderingMode));
        }
    }
    break;

  case 39:
/* Line 1269 of yacc.c.  */
#line 1201 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.orderingMode) = StaticContext::Ordered;
    }
    break;

  case 40:
/* Line 1269 of yacc.c.  */
#line 1205 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.orderingMode) = StaticContext::Unordered;
    }
    break;

  case 41:
/* Line 1269 of yacc.c.  */
#line 1210 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::EmptyOrderDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare default order"),
                                            ReportContext::XQST0069, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->registerDeclaration(ParserContext::EmptyOrderDecl);
            parseInfo->staticContext->setOrderingEmptySequence((yyvsp[(4) - (5)].enums.orderingEmptySequence));
        }
    }
    break;

  case 42:
/* Line 1269 of yacc.c.  */
#line 1224 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.orderingEmptySequence) = StaticContext::Least;
    }
    break;

  case 43:
/* Line 1269 of yacc.c.  */
#line 1228 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.orderingEmptySequence) = StaticContext::Greatest;
    }
    break;

  case 44:
/* Line 1269 of yacc.c.  */
#line 1234 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::CopyNamespacesDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare copy-namespaces"),
                                            ReportContext::XQST0055, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->registerDeclaration(ParserContext::CopyNamespacesDecl);
            parseInfo->staticContext->setPreserveMode((yyvsp[(3) - (6)].enums.preserveMode));
            parseInfo->staticContext->setInheritMode((yyvsp[(5) - (6)].enums.inheritMode));
        }
    }
    break;

  case 45:
/* Line 1269 of yacc.c.  */
#line 1249 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.preserveMode) = StaticContext::Preserve;
    }
    break;

  case 46:
/* Line 1269 of yacc.c.  */
#line 1254 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.preserveMode) = StaticContext::NoPreserve;
    }
    break;

  case 47:
/* Line 1269 of yacc.c.  */
#line 1259 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.inheritMode) = StaticContext::Inherit;
    }
    break;

  case 48:
/* Line 1269 of yacc.c.  */
#line 1264 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.inheritMode) = StaticContext::NoInherit;
    }
    break;

  case 49:
/* Line 1269 of yacc.c.  */
#line 1269 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::DefaultCollationDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare default collation"),
                                            ReportContext::XQST0038, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            const QUrl coll(resolveAndCheckCollation<ReportContext::XQST0038>((yyvsp[(4) - (5)].sval), parseInfo, (yyloc)));

            parseInfo->registerDeclaration(ParserContext::DefaultCollationDecl);
            parseInfo->staticContext->setDefaultCollation(coll);
        }
    }
    break;

  case 50:
/* Line 1269 of yacc.c.  */
#line 1285 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::BaseURIDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare base-uri"),
                                            ReportContext::XQST0032, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->registerDeclaration(ParserContext::BaseURIDecl);
            const ReflectYYLTYPE ryy((yyloc), parseInfo);

            QUrl toBeBase(AnyURI::toQUrl<ReportContext::XQST0046>((yyvsp[(3) - (4)].sval), parseInfo->staticContext, &ryy));
            /* Now we're guaranteed that base is a valid lexical representation, but it can still be relative. */

            if(toBeBase.isRelative())
                toBeBase = parseInfo->staticContext->baseURI().resolved(toBeBase);

            parseInfo->staticContext->setBaseURI(toBeBase);
        }
    }
    break;

  case 51:
/* Line 1269 of yacc.c.  */
#line 1307 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->staticContext->error(tr("The Schema Import feature is not supported, "
                                           "and therefore %1 declarations cannot occur.")
                                           .arg(formatKeyword("import schema")),
                                        ReportContext::XQST0009, fromYYLTYPE((yyloc), parseInfo));
    }
    break;

  case 55:
/* Line 1269 of yacc.c.  */
#line 1319 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(4) - (6)].sval).isEmpty())
        {
            parseInfo->staticContext->error(tr("The target namespace of a %1 cannot be empty.")
                                               .arg(formatKeyword("module import")),
                                           ReportContext::XQST0088, fromYYLTYPE((yyloc), parseInfo));

        }
        else
        {
            /* This is temporary until we have implemented it. */
            parseInfo->staticContext->error(tr("The module import feature is not supported"),
                                            ReportContext::XQST0016, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 62:
/* Line 1269 of yacc.c.  */
#line 1345 "../../sdk/QueryTransformParser.ypp"
    {
        if(variableByName((yyvsp[(4) - (7)].qName), parseInfo))
        {
            parseInfo->staticContext->error(tr("A variable by name %1 has already "
                                               "been declared in the prolog.")
                                               .arg(formatKeyword(parseInfo->staticContext->namePool()->toLexical((yyvsp[(4) - (7)].qName)))),
                                            ReportContext::XQST0049, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            if((yyvsp[(6) - (7)].expr)) /* We got a value assigned. */
            {
                const Expression::Ptr checked
                        (TypeChecker::applyFunctionConversion((yyvsp[(6) - (7)].expr), (yyvsp[(5) - (7)].sequenceType), parseInfo->staticContext));

                pushVariable((yyvsp[(4) - (7)].qName), (yyvsp[(5) - (7)].sequenceType), checked, VariableDeclaration::ExpressionVariable, (yyloc), parseInfo);
                parseInfo->declaredVariables.append(parseInfo->variables.last());
            }
            else /* We got an 'external' declaration. */
            {
                const SequenceType::Ptr varType(parseInfo->staticContext->
                                                externalVariableLoader()->announceExternalVariable((yyvsp[(4) - (7)].qName), (yyvsp[(5) - (7)].sequenceType)));

                if(varType)
                {
                    const Expression::Ptr extRef(create(new ExternalVariableReference((yyvsp[(4) - (7)].qName), varType), (yyloc), parseInfo));
                    const Expression::Ptr checked
                        (TypeChecker::applyFunctionConversion(extRef, varType, parseInfo->staticContext));

                    pushVariable((yyvsp[(4) - (7)].qName), varType, checked, VariableDeclaration::ExpressionVariable, (yyloc), parseInfo);
                }
                else
                {
                    parseInfo->staticContext->error(tr("No value is available for the external "
                                                       "variable by name %1, declared to be of "
                                                       "type %2")
                                                       .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(4) - (7)].qName)))
                                                       .arg(formatType(parseInfo->staticContext->namePool(), (yyvsp[(5) - (7)].sequenceType))),
                                                    ReportContext::XPDY0002, fromYYLTYPE((yyloc), parseInfo));
                }
            }
        }
    }
    break;

  case 63:
/* Line 1269 of yacc.c.  */
#line 1390 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr).reset();
    }
    break;

  case 64:
/* Line 1269 of yacc.c.  */
#line 1394 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (2)].expr);
    }
    break;

  case 65:
/* Line 1269 of yacc.c.  */
#line 1399 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->hasDeclaration(ParserContext::ConstructionDecl))
        {
            parseInfo->staticContext->error(prologMessage("declare ordering"),
                                            ReportContext::XQST0067, fromYYLTYPE((yyloc), parseInfo));
        }
        else
        {
            parseInfo->registerDeclaration(ParserContext::ConstructionDecl);
            parseInfo->staticContext->setConstructionMode((yyvsp[(3) - (4)].enums.constructionMode));
        }
    }
    break;

  case 66:
/* Line 1269 of yacc.c.  */
#line 1413 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.constructionMode) = StaticContext::CMStrip;
    }
    break;

  case 67:
/* Line 1269 of yacc.c.  */
#line 1417 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.constructionMode) = StaticContext::CMPreserve;
    }
    break;

  case 68:
/* Line 1269 of yacc.c.  */
#line 1422 "../../sdk/QueryTransformParser.ypp"
    {
                (yyval.enums.slot) = parseInfo->currentExpressionSlot() - (yyvsp[(5) - (6)].functionArguments).count();
              }
    break;

  case 69:
/* Line 1269 of yacc.c.  */
#line 1426 "../../sdk/QueryTransformParser.ypp"
    {
        /* If FunctionBody is null, it is 'external', otherwise the value is the body. */
        const QName::NamespaceCode ns((yyvsp[(3) - (10)].qName).namespaceURI());

        if((yyvsp[(9) - (10)].expr)) /* We got a function body. */
        {
            if(ns == StandardNamespaces::empty)
            {
                parseInfo->staticContext->error(tr("The namespace for a user defined function "
                                                   "cannot be empty(try the predefined "
                                                   "prefix %1 which exists for cases "
                                                   "like this)")
                                                   .arg(formatKeyword("local")),
                                                ReportContext::XQST0060, fromYYLTYPE((yyloc), parseInfo));
            }
            else if(XPathHelper::isReservedNamespace(ns))
            {
                parseInfo->staticContext->error(tr("The namespace %1 is reserved, and "
                                                   "therefore cannot user defined functions "
                                                   "use it(try the predefined prefix %2 which "
                                                   "exists for cases like this)")
                                                   .arg(formatURI(parseInfo->staticContext->namePool(), ns), formatKeyword("local")),
                                                ReportContext::XQST0045, fromYYLTYPE((yyloc), parseInfo));
            }
            else if(parseInfo->moduleNamespace != StandardNamespaces::empty &&
                    ns != parseInfo->moduleNamespace)
            {
                parseInfo->staticContext->error(tr("The namespace of a user defined "
                                                   "function in a library module must be "
                                                   "equivalent to the module namespace. "
                                                   "In other words, it should be %1 instead "
                                                   "of %2")
                                                   .arg(formatURI(parseInfo->staticContext->namePool(), parseInfo->moduleNamespace),
                                                        formatURI(parseInfo->staticContext->namePool(), ns)),
                                                ReportContext::XQST0048, fromYYLTYPE((yyloc), parseInfo));
            }
            else
            {
                /* Apply function conversion such that the body matches the declared
                 * return type. */
                const Expression::Ptr checked(TypeChecker::applyFunctionConversion((yyvsp[(9) - (10)].expr), (yyvsp[(8) - (10)].sequenceType),
                                                                    parseInfo->staticContext));

                const int argCount = (yyvsp[(5) - (10)].functionArguments).count();
                qDebug() << "Arg count: " << argCount << endl;
                const FunctionSignature::Ptr sign(new FunctionSignature((yyvsp[(3) - (10)].qName) /* name */,
                                                                        argCount /* minArgs */,
                                                                        argCount /* maxArgs */,
                                                                        (yyvsp[(8) - (10)].sequenceType) /* returnType */));
                sign->setArguments((yyvsp[(5) - (10)].functionArguments));
                const UserFunction::List::const_iterator end(parseInfo->userFunctions.constEnd());
                UserFunction::List::const_iterator it(parseInfo->userFunctions.constBegin());

                for(; it != end; ++it)
                {
                    if(*(*it)->signature() == *sign)
                    {
                        parseInfo->staticContext->error(tr("A function already exists with "
                                                           "the signature %1.")
                                                           .arg(formatFunction(parseInfo->staticContext->namePool(), sign)),
                                                        ReportContext::XQST0034, fromYYLTYPE((yyloc), parseInfo));
                    }
                }

                VariableDeclaration::List argDecls;
                const int top = (yyvsp[(7) - (10)].enums.slot) + argCount;
                qDebug() << "top:" << top << "$en..:" << (yyvsp[(7) - (10)].enums.slot) << "argCount:" << argCount
                         << "variables:" << parseInfo->variables.count();

                for(int i = 0; i < argCount; ++i)
                    argDecls.append(parseInfo->variables.at(i));

                if((yyvsp[(7) - (10)].enums.slot) > -1)
                {
                    /* We have allocated slots, so now push them out of scope. */
                    finalizePushedVariable(parseInfo, argCount);
                }

                parseInfo->userFunctions.append(UserFunction::Ptr(new UserFunction(sign, checked, (yyvsp[(7) - (10)].enums.slot), argDecls)));
            }
        }
        else /* We got an 'external' declaration. */
        {
            parseInfo->staticContext->error(tr("No external functions are supported. "
                                               "All supported functions can be used directly, "
                                               "without first declaring them as external"),
                                            ReportContext::XPST0017, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 70:
/* Line 1269 of yacc.c.  */
#line 1517 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.functionArguments) = FunctionArgument::List();
    }
    break;

  case 71:
/* Line 1269 of yacc.c.  */
#line 1521 "../../sdk/QueryTransformParser.ypp"
    {
        FunctionArgument::List l;
        l.append((yyvsp[(1) - (1)].functionArgument));
        (yyval.functionArguments) = l;
    }
    break;

  case 72:
/* Line 1269 of yacc.c.  */
#line 1527 "../../sdk/QueryTransformParser.ypp"
    {
        FunctionArgument::List::const_iterator it((yyvsp[(1) - (3)].functionArguments).constBegin());
        const FunctionArgument::List::const_iterator end((yyvsp[(1) - (3)].functionArguments).constEnd());

        for(; it != end; ++it)
        {
            if((*it)->name() == (yyvsp[(3) - (3)].functionArgument)->name())
            {
                parseInfo->staticContext->error(tr("An argument by name %1 has already "
                                                   "been declared. Every argument name "
                                                   "must be unique.")
                                                   .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(3) - (3)].functionArgument)->name())),
                                                ReportContext::XQST0039, fromYYLTYPE((yyloc), parseInfo));
            }
        }

        (yyvsp[(1) - (3)].functionArguments).append((yyvsp[(3) - (3)].functionArgument));
        (yyval.functionArguments) = (yyvsp[(1) - (3)].functionArguments);
    }
    break;

  case 73:
/* Line 1269 of yacc.c.  */
#line 1548 "../../sdk/QueryTransformParser.ypp"
    {
        pushVariable((yyvsp[(2) - (3)].qName), (yyvsp[(3) - (3)].sequenceType), Expression::Ptr(), VariableDeclaration::FunctionArgument, (yyloc), parseInfo);
        (yyval.functionArgument) = FunctionArgument::Ptr(new FunctionArgument((yyvsp[(2) - (3)].qName), (yyvsp[(3) - (3)].sequenceType)));
    }
    break;

  case 74:
/* Line 1269 of yacc.c.  */
#line 1554 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr).reset();
    }
    break;

  case 76:
/* Line 1269 of yacc.c.  */
#line 1560 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (3)].expr);
    }
    break;

  case 79:
/* Line 1269 of yacc.c.  */
#line 1568 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new ExpressionSequence((yyvsp[(1) - (1)].expressionList)), (yyloc), parseInfo);
    }
    break;

  case 80:
/* Line 1269 of yacc.c.  */
#line 1573 "../../sdk/QueryTransformParser.ypp"
    {
        Expression::List l;
        l.append((yyvsp[(1) - (3)].expr));
        l.append((yyvsp[(3) - (3)].expr));
        (yyval.expressionList) = l;
    }
    break;

  case 81:
/* Line 1269 of yacc.c.  */
#line 1580 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(1) - (3)].expressionList).append((yyvsp[(3) - (3)].expr));
        (yyval.expressionList) = (yyvsp[(1) - (3)].expressionList);
    }
    break;

  case 89:
/* Line 1269 of yacc.c.  */
#line 1596 "../../sdk/QueryTransformParser.ypp"
    {
               /* We're pushing the range variable here, not the positional. */
               (yyval.expr) = pushVariable((yyvsp[(3) - (7)].qName), quantificationType((yyvsp[(4) - (7)].sequenceType)), (yyvsp[(7) - (7)].expr), VariableDeclaration::RangeVariable, (yyloc), parseInfo);
           }
    break;

  case 90:
/* Line 1269 of yacc.c.  */
#line 1600 "../../sdk/QueryTransformParser.ypp"
    {
               /* It is ok this appears after PositionalVar, because currentRangeSlot()
                * uses a different "channel" than currentPositionSlot(), so they can't trash
                * each other. */
               (yyval.enums.slot) = parseInfo->currentRangeSlot();
           }
    break;

  case 91:
/* Line 1269 of yacc.c.  */
#line 1607 "../../sdk/QueryTransformParser.ypp"
    {
        Q_ASSERT((yyvsp[(7) - (10)].expr));
        Q_ASSERT((yyvsp[(10) - (10)].expr));

        /* We want the next last pushed variable, since we push the range variable after the
         * positional variable. */
        if((yyvsp[(5) - (10)].enums.slot) != -1 && parseInfo->variables.at(parseInfo->variables.count() -2)->name == (yyvsp[(3) - (10)].qName))
        {
            /* Ok, a positional variable is used since its slot is not -1, and its name is equal
             * to our range variable. This is an error. */
            parseInfo->staticContext->error(tr("The name of a variable bound in a for-expression must be different "
                                               "from the positional variable. Hence, the two variables by name %1 clashes.")
                                               .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(3) - (10)].qName))),
                                            ReportContext::XQST0089, fromYYLTYPE((yyloc), parseInfo));

        }

        const Expression::Ptr retBody(create(new ForClause((yyvsp[(9) - (10)].enums.slot), (yyvsp[(8) - (10)].expr), (yyvsp[(10) - (10)].expr), (yyvsp[(5) - (10)].enums.slot)), (yyloc), parseInfo));

        if((yyvsp[(10) - (10)].expr)->is(Expression::IDReturnOrderBy))
            (yyval.expr) = create(new OrderBy((yyvsp[(10) - (10)].expr)->as<ReturnOrderBy>()->stability(), (yyvsp[(10) - (10)].expr)->as<ReturnOrderBy>()->orderSpecs(), retBody), (yyloc), parseInfo);
        else
            (yyval.expr) = retBody;

        finalizePushedVariable(parseInfo);

        if((yyvsp[(5) - (10)].enums.slot) != -1) /* We also have a positional variable to remove from the scope. */
            finalizePushedVariable(parseInfo);
    }
    break;

  case 92:
/* Line 1269 of yacc.c.  */
#line 1639 "../../sdk/QueryTransformParser.ypp"
    {
             pushVariable((yyvsp[(3) - (7)].qName), quantificationType((yyvsp[(4) - (7)].sequenceType)), (yyvsp[(7) - (7)].expr), VariableDeclaration::RangeVariable, (yyloc), parseInfo);
         }
    break;

  case 93:
/* Line 1269 of yacc.c.  */
#line 1642 "../../sdk/QueryTransformParser.ypp"
    {
             /* It is ok this appears after PositionalVar, because currentRangeSlot()
              * uses a different "channel" than currentPositionSlot(), so they can't trash
              * each other. */
             (yyval.enums.slot) = parseInfo->currentRangeSlot();
         }
    break;

  case 94:
/* Line 1269 of yacc.c.  */
#line 1649 "../../sdk/QueryTransformParser.ypp"
    {
        const Expression::Ptr retBody(create(new ForClause((yyvsp[(9) - (10)].enums.slot), (yyvsp[(7) - (10)].expr), (yyvsp[(10) - (10)].expr), (yyvsp[(5) - (10)].enums.slot)), (yyloc), parseInfo));

        if((yyvsp[(10) - (10)].expr)->is(Expression::IDReturnOrderBy))
            (yyval.expr) = create(new OrderBy((yyvsp[(10) - (10)].expr)->as<ReturnOrderBy>()->stability(), (yyvsp[(10) - (10)].expr)->as<ReturnOrderBy>()->orderSpecs(), retBody), (yyloc), parseInfo);
        else
            (yyval.expr) = retBody;

        finalizePushedVariable(parseInfo);

        if((yyvsp[(5) - (10)].enums.slot) != -1) /* We also have a positional variable to remove from the scope. */
            finalizePushedVariable(parseInfo);
    }
    break;

  case 98:
/* Line 1269 of yacc.c.  */
#line 1668 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.slot) = -1;
    }
    break;

  case 99:
/* Line 1269 of yacc.c.  */
#line 1673 "../../sdk/QueryTransformParser.ypp"
    {
        pushVariable((yyvsp[(3) - (3)].qName), CommonSequenceTypes::ExactlyOneInteger, Expression::Ptr(),
                     VariableDeclaration::PositionalVariable, (yyloc), parseInfo);
        (yyval.enums.slot) = parseInfo->currentPositionSlot();
    }
    break;

  case 100:
/* Line 1269 of yacc.c.  */
#line 1680 "../../sdk/QueryTransformParser.ypp"
    { (yyval.expr) = pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr), VariableDeclaration::ExpressionVariable, (yyloc), parseInfo);}
    break;

  case 101:
/* Line 1269 of yacc.c.  */
#line 1682 "../../sdk/QueryTransformParser.ypp"
    {
        Q_ASSERT(parseInfo->variables.top()->name == (yyvsp[(3) - (8)].qName));
        (yyval.expr) = create(new LetClause((yyvsp[(7) - (8)].expr), (yyvsp[(8) - (8)].expr), parseInfo->variables.top()), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 102:
/* Line 1269 of yacc.c.  */
#line 1689 "../../sdk/QueryTransformParser.ypp"
    { (yyval.expr) = pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr), VariableDeclaration::ExpressionVariable, (yyloc), parseInfo);}
    break;

  case 103:
/* Line 1269 of yacc.c.  */
#line 1691 "../../sdk/QueryTransformParser.ypp"
    {
        Q_ASSERT(parseInfo->variables.top()->name == (yyvsp[(3) - (8)].qName));
        (yyval.expr) = create(new LetClause((yyvsp[(7) - (8)].expr), (yyvsp[(8) - (8)].expr), parseInfo->variables.top()), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 107:
/* Line 1269 of yacc.c.  */
#line 1702 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(1) - (3)].orderSpecs).isEmpty())
            (yyval.expr) = (yyvsp[(3) - (3)].expr);
        else
            (yyval.expr) = createReturnOrderBy((yyvsp[(1) - (3)].orderSpecs), (yyvsp[(3) - (3)].expr), parseInfo->orderStability.pop(), (yyloc), parseInfo);
    }
    break;

  case 108:
/* Line 1269 of yacc.c.  */
#line 1710 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(3) - (5)].orderSpecs).isEmpty())
            (yyval.expr) = create(new IfThenClause((yyvsp[(2) - (5)].expr), (yyvsp[(5) - (5)].expr), create(new EmptySequence, (yyloc), parseInfo)), (yyloc), parseInfo);
        else
            (yyval.expr) = create(new IfThenClause((yyvsp[(2) - (5)].expr), createReturnOrderBy((yyvsp[(3) - (5)].orderSpecs), (yyvsp[(5) - (5)].expr), parseInfo->orderStability.pop(), (yyloc), parseInfo),
                                         create(new EmptySequence, (yyloc), parseInfo)),
                        (yyloc), parseInfo);
    }
    break;

  case 109:
/* Line 1269 of yacc.c.  */
#line 1720 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.orderSpecs) = OrderSpecTransfer::List();
    }
    break;

  case 110:
/* Line 1269 of yacc.c.  */
#line 1724 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.orderSpecs) = (yyvsp[(2) - (2)].orderSpecs);
    }
    break;

  case 111:
/* Line 1269 of yacc.c.  */
#line 1729 "../../sdk/QueryTransformParser.ypp"
    {
        OrderSpecTransfer::List list;
        list += (yyvsp[(1) - (3)].orderSpecs);
        list.append((yyvsp[(3) - (3)].orderSpec));
        (yyval.orderSpecs) = list;
    }
    break;

  case 112:
/* Line 1269 of yacc.c.  */
#line 1736 "../../sdk/QueryTransformParser.ypp"
    {
        OrderSpecTransfer::List list;
        list.append((yyvsp[(1) - (1)].orderSpec));
        (yyval.orderSpecs) = list;
    }
    break;

  case 113:
/* Line 1269 of yacc.c.  */
#line 1743 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.orderSpec) = OrderSpecTransfer((yyvsp[(1) - (4)].expr), OrderBy::OrderSpec((yyvsp[(2) - (4)].enums.sortDirection), (yyvsp[(3) - (4)].enums.orderingEmptySequence)));
    }
    break;

  case 114:
/* Line 1269 of yacc.c.  */
#line 1748 "../../sdk/QueryTransformParser.ypp"
    {
        /* Where does the specification state the default value is ascending?
         *
         * It is implicit, in the first enumerated list in 3.8.3 Order By and Return Clauses:
         *
         * "If T1 and T2 are two tuples in the tuple stream, and V1 and V2 are the first pair
         *  of values encountered when evaluating their orderspecs from left to right for
         *  which one value is greater-than the other (as defined above), then:
         *
         *      1. If V1 is greater-than V2: If the orderspec specifies descending,
         *         then T1 precedes T2 in the tuple stream; otherwise, T2 precedes T1 in the tuple stream.
         *      2. If V2 is greater-than V1: If the orderspec specifies descending,
         *         then T2 precedes T1 in the tuple stream; otherwise, T1 precedes T2 in the tuple stream."
         *
         * which means that if you don't specify anything, or you
         * specify ascending, you get the same result.
         */
        (yyval.enums.sortDirection) = OrderBy::OrderSpec::Ascending;
    }
    break;

  case 115:
/* Line 1269 of yacc.c.  */
#line 1769 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.sortDirection) = OrderBy::OrderSpec::Ascending;
    }
    break;

  case 116:
/* Line 1269 of yacc.c.  */
#line 1774 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.sortDirection) = OrderBy::OrderSpec::Descending;
    }
    break;

  case 117:
/* Line 1269 of yacc.c.  */
#line 1779 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.orderingEmptySequence) = parseInfo->staticContext->orderingEmptySequence();
    }
    break;

  case 120:
/* Line 1269 of yacc.c.  */
#line 1786 "../../sdk/QueryTransformParser.ypp"
    {
        resolveAndCheckCollation<ReportContext::XQST0076>((yyvsp[(2) - (2)].sval), parseInfo, (yyloc));
    }
    break;

  case 121:
/* Line 1269 of yacc.c.  */
#line 1791 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->orderStability.push(OrderBy::StableOrder);
    }
    break;

  case 122:
/* Line 1269 of yacc.c.  */
#line 1795 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->orderStability.push(OrderBy::UnstableOrder);
    }
    break;

  case 125:
/* Line 1269 of yacc.c.  */
#line 1803 "../../sdk/QueryTransformParser.ypp"
    {
                            pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr),
                                         VariableDeclaration::RangeVariable, (yyloc), parseInfo);
                        }
    break;

  case 126:
/* Line 1269 of yacc.c.  */
#line 1807 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.slot) = parseInfo->currentRangeSlot();}
    break;

  case 127:
/* Line 1269 of yacc.c.  */
#line 1809 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new QuantifiedExpression((yyvsp[(8) - (9)].enums.slot),
                                             QuantifiedExpression::Some, (yyvsp[(6) - (9)].expr), (yyvsp[(9) - (9)].expr)), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 128:
/* Line 1269 of yacc.c.  */
#line 1816 "../../sdk/QueryTransformParser.ypp"
    {
                            (yyval.expr) = pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr),
                                                    VariableDeclaration::RangeVariable, (yyloc), parseInfo);
                        }
    break;

  case 129:
/* Line 1269 of yacc.c.  */
#line 1820 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.slot) = parseInfo->currentRangeSlot();}
    break;

  case 130:
/* Line 1269 of yacc.c.  */
#line 1822 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new QuantifiedExpression((yyvsp[(8) - (9)].enums.slot),
                                             QuantifiedExpression::Some, (yyvsp[(7) - (9)].expr), (yyvsp[(9) - (9)].expr)), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 132:
/* Line 1269 of yacc.c.  */
#line 1831 "../../sdk/QueryTransformParser.ypp"
    {
                            pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr),
                                         VariableDeclaration::RangeVariable, (yyloc), parseInfo);
                         }
    break;

  case 133:
/* Line 1269 of yacc.c.  */
#line 1835 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.slot) = parseInfo->currentRangeSlot();}
    break;

  case 134:
/* Line 1269 of yacc.c.  */
#line 1837 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new QuantifiedExpression((yyvsp[(8) - (9)].enums.slot),
                                             QuantifiedExpression::Every, (yyvsp[(6) - (9)].expr), (yyvsp[(9) - (9)].expr)), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 135:
/* Line 1269 of yacc.c.  */
#line 1844 "../../sdk/QueryTransformParser.ypp"
    {
                            (yyval.expr) = pushVariable((yyvsp[(3) - (6)].qName), quantificationType((yyvsp[(4) - (6)].sequenceType)), (yyvsp[(6) - (6)].expr),
                                                    VariableDeclaration::RangeVariable, (yyloc), parseInfo);
                         }
    break;

  case 136:
/* Line 1269 of yacc.c.  */
#line 1848 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.slot) = parseInfo->currentRangeSlot();}
    break;

  case 137:
/* Line 1269 of yacc.c.  */
#line 1850 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new QuantifiedExpression((yyvsp[(8) - (9)].enums.slot),
                                             QuantifiedExpression::Every, (yyvsp[(7) - (9)].expr), (yyvsp[(9) - (9)].expr)), (yyloc), parseInfo);
        finalizePushedVariable(parseInfo);
    }
    break;

  case 139:
/* Line 1269 of yacc.c.  */
#line 1859 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (2)].expr);
    }
    break;

  case 140:
/* Line 1269 of yacc.c.  */
#line 1886 "../../sdk/QueryTransformParser.ypp"
    {
                    parseInfo->typeswitchSource.push((yyvsp[(3) - (4)].expr));
                }
    break;

  case 141:
/* Line 1269 of yacc.c.  */
#line 1890 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->typeswitchSource.pop();
        (yyval.expr) = (yyvsp[(6) - (6)].expr);
    }
    break;

  case 142:
/* Line 1269 of yacc.c.  */
#line 1896 "../../sdk/QueryTransformParser.ypp"
    {
        if(!(yyvsp[(2) - (3)].qName).isNull())
        {
            pushVariable((yyvsp[(2) - (3)].qName), (yyvsp[(3) - (3)].sequenceType), parseInfo->typeswitchSource.top(),
                         VariableDeclaration::ExpressionVariable, (yyloc), parseInfo, false);
        }
    }
    break;

  case 143:
/* Line 1269 of yacc.c.  */
#line 1904 "../../sdk/QueryTransformParser.ypp"
    {
        /* The variable shouldn't be in-scope for other case branches. */
        if(!(yyvsp[(2) - (6)].qName).isNull())
            finalizePushedVariable(parseInfo);
    }
    break;

  case 144:
/* Line 1269 of yacc.c.  */
#line 1910 "../../sdk/QueryTransformParser.ypp"
    {
        const Expression::Ptr instanceOf(create(new InstanceOf(parseInfo->typeswitchSource.top(), (yyvsp[(3) - (8)].sequenceType)), (yyloc), parseInfo));
        (yyval.expr) = create(new IfThenClause(instanceOf, (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].expr)), (yyloc), parseInfo);
    }
    break;

  case 147:
/* Line 1269 of yacc.c.  */
#line 1919 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.qName) = QName();
    }
    break;

  case 148:
/* Line 1269 of yacc.c.  */
#line 1924 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.qName) = (yyvsp[(2) - (3)].qName);
    }
    break;

  case 149:
/* Line 1269 of yacc.c.  */
#line 1929 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(3) - (3)].expr);
    }
    break;

  case 150:
/* Line 1269 of yacc.c.  */
#line 1933 "../../sdk/QueryTransformParser.ypp"
    {
        if(!(yyvsp[(3) - (3)].qName).isNull())
        {
            pushVariable((yyvsp[(3) - (3)].qName), parseInfo->typeswitchSource.top()->staticType(),
                         parseInfo->typeswitchSource.top(),
                         VariableDeclaration::ExpressionVariable, (yyloc), parseInfo, false);
        }
    }
    break;

  case 151:
/* Line 1269 of yacc.c.  */
#line 1942 "../../sdk/QueryTransformParser.ypp"
    {
        if(!(yyvsp[(3) - (6)].qName).isNull())
            finalizePushedVariable(parseInfo);
        (yyval.expr) = (yyvsp[(6) - (6)].expr);
    }
    break;

  case 152:
/* Line 1269 of yacc.c.  */
#line 1949 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new IfThenClause((yyvsp[(3) - (8)].expr), (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].expr)), (yyloc), parseInfo);
    }
    break;

  case 154:
/* Line 1269 of yacc.c.  */
#line 1955 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new OrExpression((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 156:
/* Line 1269 of yacc.c.  */
#line 1961 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new AndExpression((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 162:
/* Line 1269 of yacc.c.  */
#line 1972 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new RangeExpression((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 164:
/* Line 1269 of yacc.c.  */
#line 1978 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new ArithmeticExpression((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.mathOperator), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 165:
/* Line 1269 of yacc.c.  */
#line 1982 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::Add;}
    break;

  case 166:
/* Line 1269 of yacc.c.  */
#line 1983 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::Substract;}
    break;

  case 168:
/* Line 1269 of yacc.c.  */
#line 1987 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new ArithmeticExpression((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.mathOperator), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 169:
/* Line 1269 of yacc.c.  */
#line 1991 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::Multiply;}
    break;

  case 170:
/* Line 1269 of yacc.c.  */
#line 1992 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::Div;}
    break;

  case 171:
/* Line 1269 of yacc.c.  */
#line 1993 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::IDiv;}
    break;

  case 172:
/* Line 1269 of yacc.c.  */
#line 1994 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.mathOperator) = AtomicMathematician::Mod;}
    break;

  case 174:
/* Line 1269 of yacc.c.  */
#line 1998 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CombineNodes((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.combinedNodeOp), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 176:
/* Line 1269 of yacc.c.  */
#line 2004 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CombineNodes((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.combinedNodeOp), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 177:
/* Line 1269 of yacc.c.  */
#line 2009 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.combinedNodeOp) = CombineNodes::Union;
    }
    break;

  case 178:
/* Line 1269 of yacc.c.  */
#line 2013 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.combinedNodeOp) = CombineNodes::Union;
    }
    break;

  case 179:
/* Line 1269 of yacc.c.  */
#line 2018 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.combinedNodeOp) = CombineNodes::Intersect;
    }
    break;

  case 180:
/* Line 1269 of yacc.c.  */
#line 2022 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.combinedNodeOp) = CombineNodes::Except;
    }
    break;

  case 182:
/* Line 1269 of yacc.c.  */
#line 2028 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new InstanceOf((yyvsp[(1) - (4)].expr),
        SequenceType::Ptr((yyvsp[(4) - (4)].sequenceType))), (yyloc), parseInfo);
    }
    break;

  case 184:
/* Line 1269 of yacc.c.  */
#line 2035 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new TreatAs((yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].sequenceType)), (yyloc), parseInfo);
    }
    break;

  case 186:
/* Line 1269 of yacc.c.  */
#line 2041 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CastableAs((yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].sequenceType)), (yyloc), parseInfo);
    }
    break;

  case 188:
/* Line 1269 of yacc.c.  */
#line 2047 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CastAs((yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].sequenceType)), (yyloc), parseInfo);
    }
    break;

  case 190:
/* Line 1269 of yacc.c.  */
#line 2053 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(UnaryExpression::create((yyvsp[(1) - (2)].enums.mathOperator), (yyvsp[(2) - (2)].expr), parseInfo->staticContext), (yyloc), parseInfo);
    }
    break;

  case 191:
/* Line 1269 of yacc.c.  */
#line 2058 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.mathOperator) = AtomicMathematician::Add;
    }
    break;

  case 192:
/* Line 1269 of yacc.c.  */
#line 2062 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.enums.mathOperator) = AtomicMathematician::Substract;
    }
    break;

  case 196:
/* Line 1269 of yacc.c.  */
#line 2071 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new GeneralComparison((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.valueOperator), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 197:
/* Line 1269 of yacc.c.  */
#line 2075 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorEqual;}
    break;

  case 198:
/* Line 1269 of yacc.c.  */
#line 2076 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorNotEqual;}
    break;

  case 199:
/* Line 1269 of yacc.c.  */
#line 2077 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorGreaterOrEqual;}
    break;

  case 200:
/* Line 1269 of yacc.c.  */
#line 2078 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorGreaterThan;}
    break;

  case 201:
/* Line 1269 of yacc.c.  */
#line 2079 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorLessOrEqual;}
    break;

  case 202:
/* Line 1269 of yacc.c.  */
#line 2080 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorLessThan;}
    break;

  case 203:
/* Line 1269 of yacc.c.  */
#line 2083 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new ValueComparison((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.valueOperator), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 204:
/* Line 1269 of yacc.c.  */
#line 2087 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorEqual;}
    break;

  case 205:
/* Line 1269 of yacc.c.  */
#line 2088 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorNotEqual;}
    break;

  case 206:
/* Line 1269 of yacc.c.  */
#line 2089 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorGreaterOrEqual;}
    break;

  case 207:
/* Line 1269 of yacc.c.  */
#line 2090 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorGreaterThan;}
    break;

  case 208:
/* Line 1269 of yacc.c.  */
#line 2091 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorLessOrEqual;}
    break;

  case 209:
/* Line 1269 of yacc.c.  */
#line 2092 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.valueOperator) = AtomicComparator::OperatorLessThan;}
    break;

  case 210:
/* Line 1269 of yacc.c.  */
#line 2095 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new NodeComparison((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].enums.nodeOperator), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 211:
/* Line 1269 of yacc.c.  */
#line 2099 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.nodeOperator) = Node::Is;}
    break;

  case 212:
/* Line 1269 of yacc.c.  */
#line 2100 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.nodeOperator) = Node::Precedes;}
    break;

  case 213:
/* Line 1269 of yacc.c.  */
#line 2101 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.nodeOperator) = Node::Follows;}
    break;

  case 214:
/* Line 1269 of yacc.c.  */
#line 2104 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->staticContext->error(tr("The Schema Validation Feature is not supported. "
                                           "For that reason, %1-expressions cannot be used.")
                                           .arg(formatKeyword("validate")),
                                        ReportContext::XQST0075, fromYYLTYPE((yyloc), parseInfo));
        /*
        $$ = Validate::create($2, $1, parseInfo->staticContext);
        */
    }
    break;

  case 215:
/* Line 1269 of yacc.c.  */
#line 2116 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.validationMode) = Validate::Strict;}
    break;

  case 216:
/* Line 1269 of yacc.c.  */
#line 2117 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.validationMode) = Validate::Strict;}
    break;

  case 217:
/* Line 1269 of yacc.c.  */
#line 2118 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.validationMode) = Validate::Lax;}
    break;

  case 218:
/* Line 1269 of yacc.c.  */
#line 2121 "../../sdk/QueryTransformParser.ypp"
    {
        /* We don't support any pragmas, so we only do the
         * necessary validation and use the fallback expression. */

        if((yyvsp[(2) - (2)].expr))
            (yyval.expr) = (yyvsp[(2) - (2)].expr);
        else
        {
            parseInfo->staticContext->error(tr("None of the pragma expressions are supported "
                                               "and therefore a fallback expression "
                                               "must be present"),
                                            ReportContext::XQST0079, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 219:
/* Line 1269 of yacc.c.  */
#line 2137 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr).reset();
    }
    break;

  case 220:
/* Line 1269 of yacc.c.  */
#line 2141 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (3)].expr);
    }
    break;

  case 226:
/* Line 1269 of yacc.c.  */
#line 2154 "../../sdk/QueryTransformParser.ypp"
    {
        /* This is "/step". That is, fn:root(self::node()) treat as document-node()/RelativePathExpr. */
        (yyval.expr) = create(new Path(createRootExpression(parseInfo, (yyloc)), (yyvsp[(2) - (2)].expr)), (yyloc), parseInfo);
    }
    break;

  case 227:
/* Line 1269 of yacc.c.  */
#line 2160 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = createSlashSlashPath(createRootExpression(parseInfo, (yyloc)), (yyvsp[(2) - (2)].expr), (yyloc), parseInfo);
    }
    break;

  case 228:
/* Line 1269 of yacc.c.  */
#line 2164 "../../sdk/QueryTransformParser.ypp"
    {
        /* This is "/". That is, fn:root(self::node()) treat as document-node(). */
        (yyval.expr) = createRootExpression(parseInfo, (yyloc));
    }
    break;

  case 231:
/* Line 1269 of yacc.c.  */
#line 2174 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new Path((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)), (yyloc), parseInfo);
    }
    break;

  case 232:
/* Line 1269 of yacc.c.  */
#line 2178 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = createSlashSlashPath((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), (yyloc), parseInfo);
    }
    break;

  case 236:
/* Line 1269 of yacc.c.  */
#line 2187 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(GenericPredicate::create((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr), parseInfo->staticContext, fromYYLTYPE((yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 239:
/* Line 1269 of yacc.c.  */
#line 2195 "../../sdk/QueryTransformParser.ypp"
    {
                if((yyvsp[(1) - (1)].enums.axis) == Node::AttributeAxis)
                    parseInfo->nodeTestSource = BuiltinTypes::attribute;
             }
    break;

  case 240:
/* Line 1269 of yacc.c.  */
#line 2200 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(3) - (3)].itemType))
        {
            /* A node test was explicitly specified. The un-abbreviated syntax was used. */
            (yyval.expr) = create(new AxisStep((yyvsp[(1) - (3)].enums.axis), (yyvsp[(3) - (3)].itemType)), (yyloc), parseInfo);
        }
        else
        {
            /* Quote from 3.2.1.1 Axes
             *
             * [Definition: Every axis has a principal node kind. If an axis
             *  can contain elements, then the principal node kind is element;
             *  otherwise, it is the kind of nodes that the axis can contain.] Thus:
             * - For the attribute axis, the principal node kind is attribute.
             * - For all other axes, the principal node kind is element. */

            if((yyvsp[(1) - (3)].enums.axis) == Node::AttributeAxis)
                (yyval.expr) = create(new AxisStep(Node::AttributeAxis, BuiltinTypes::attribute), (yyloc), parseInfo);
            else
                (yyval.expr) = create(new AxisStep((yyvsp[(1) - (3)].enums.axis), BuiltinTypes::element), (yyloc), parseInfo);
        }

        parseInfo->restoreNodeTestSource();
    }
    break;

  case 244:
/* Line 1269 of yacc.c.  */
#line 2230 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(1) - (2)].enums.axis) == Node::NamespaceAxis)
        {
            /* We don't raise XPST0010 here because the namespace axis isn't an optional
             * axis. It simply is not part of the XQuery grammar. */
            parseInfo->staticContext->error(tr("The %1-axis is unsupported in XQuery")
                                               .arg(formatKeyword("namespace")),
                                            ReportContext::XPST0003, fromYYLTYPE((yyloc), parseInfo));
        }
        else
            (yyval.enums.axis) = (yyvsp[(1) - (2)].enums.axis);
    }
    break;

  case 245:
/* Line 1269 of yacc.c.  */
#line 2243 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::AncestorOrSelf  ;}
    break;

  case 246:
/* Line 1269 of yacc.c.  */
#line 2244 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Ancestor        ;}
    break;

  case 247:
/* Line 1269 of yacc.c.  */
#line 2245 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::AttributeAxis   ;}
    break;

  case 248:
/* Line 1269 of yacc.c.  */
#line 2246 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Child           ;}
    break;

  case 249:
/* Line 1269 of yacc.c.  */
#line 2247 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::DescendantOrSelf;}
    break;

  case 250:
/* Line 1269 of yacc.c.  */
#line 2248 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Descendant      ;}
    break;

  case 251:
/* Line 1269 of yacc.c.  */
#line 2249 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Following       ;}
    break;

  case 252:
/* Line 1269 of yacc.c.  */
#line 2250 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Preceding       ;}
    break;

  case 253:
/* Line 1269 of yacc.c.  */
#line 2251 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::FollowingSibling;}
    break;

  case 254:
/* Line 1269 of yacc.c.  */
#line 2252 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::PrecedingSibling;}
    break;

  case 255:
/* Line 1269 of yacc.c.  */
#line 2253 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Parent          ;}
    break;

  case 256:
/* Line 1269 of yacc.c.  */
#line 2254 "../../sdk/QueryTransformParser.ypp"
    {(yyval.enums.axis) = Node::Self            ;}
    break;

  case 257:
/* Line 1269 of yacc.c.  */
#line 2257 "../../sdk/QueryTransformParser.ypp"
    {
                        parseInfo->nodeTestSource = BuiltinTypes::attribute;
                   }
    break;

  case 258:
/* Line 1269 of yacc.c.  */
#line 2261 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new AxisStep(Node::AttributeAxis, (yyvsp[(3) - (3)].itemType)), (yyloc), parseInfo);

        parseInfo->restoreNodeTestSource();
    }
    break;

  case 259:
/* Line 1269 of yacc.c.  */
#line 2267 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new AxisStep(Node::Child, (yyvsp[(1) - (1)].itemType)), (yyloc), parseInfo);
    }
    break;

  case 260:
/* Line 1269 of yacc.c.  */
#line 2271 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new AxisStep(Node::AttributeAxis, (yyvsp[(1) - (1)].itemType)), (yyloc), parseInfo);
    }
    break;

  case 262:
/* Line 1269 of yacc.c.  */
#line 2278 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new AxisStep(Node::Parent, BuiltinTypes::node), (yyloc), parseInfo);
    }
    break;

  case 265:
/* Line 1269 of yacc.c.  */
#line 2286 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = QNameTest::create(parseInfo->nodeTestSource, (yyvsp[(1) - (1)].qName));
    }
    break;

  case 267:
/* Line 1269 of yacc.c.  */
#line 2292 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = parseInfo->nodeTestSource;
    }
    break;

  case 268:
/* Line 1269 of yacc.c.  */
#line 2296 "../../sdk/QueryTransformParser.ypp"
    {
        const NamePool::Ptr np(parseInfo->staticContext->namePool());
        const ReflectYYLTYPE ryy((yyloc), parseInfo);

        const QName::NamespaceCode ns(QNameConstructor::namespaceForPrefix(np->allocatePrefix((yyvsp[(1) - (1)].sval)), parseInfo->staticContext, &ryy));

        (yyval.itemType) = NamespaceNameTest::create(parseInfo->nodeTestSource, ns);
    }
    break;

  case 269:
/* Line 1269 of yacc.c.  */
#line 2305 "../../sdk/QueryTransformParser.ypp"
    {
        const QName::LocalNameCode c = parseInfo->staticContext->namePool()->allocateLocalName((yyvsp[(1) - (1)].sval));
        (yyval.itemType) = LocalNameTest::create(parseInfo->nodeTestSource, c);
    }
    break;

  case 271:
/* Line 1269 of yacc.c.  */
#line 2312 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(GenericPredicate::create((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr), parseInfo->staticContext, fromYYLTYPE((yylsp[(4) - (4)]), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 280:
/* Line 1269 of yacc.c.  */
#line 2326 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new Literal(AtomicString::fromValue((yyvsp[(1) - (1)].sval))), (yyloc), parseInfo);
    }
    break;

  case 281:
/* Line 1269 of yacc.c.  */
#line 2331 "../../sdk/QueryTransformParser.ypp"
    {
        const Item num(Double::fromLexical((yyvsp[(1) - (1)].sval)));

        if(num.as<AtomicValue>()->hasError())
        {
            parseInfo->staticContext->error(tr("%1 is not a valid number literal.")
                                               .arg(formatData((yyvsp[(1) - (1)].sval))),
                                            ReportContext::XPST0003, fromYYLTYPE((yyloc), parseInfo));
        }
        else
            (yyval.expr) = create(new Literal(num), (yyloc), parseInfo);
    }
    break;

  case 282:
/* Line 1269 of yacc.c.  */
#line 2344 "../../sdk/QueryTransformParser.ypp"
    {
        const Item num(Numeric::fromLexical((yyvsp[(1) - (1)].sval)));

        if(num.as<AtomicValue>()->hasError())
        {
            parseInfo->staticContext->error(tr("%1 is not a valid number literal.")
                                               .arg(formatData((yyvsp[(1) - (1)].sval))),
                                            ReportContext::XPST0003, fromYYLTYPE((yyloc), parseInfo));
        }
        else
            (yyval.expr) = create(new Literal(num), (yyloc), parseInfo);
    }
    break;

  case 283:
/* Line 1269 of yacc.c.  */
#line 2358 "../../sdk/QueryTransformParser.ypp"
    {
        const VariableDeclaration::Ptr var(variableByName((yyvsp[(2) - (2)].qName), parseInfo));

        /* We don't support external variables currently. */
        if(var)
        {
            switch(var->type)
            {
                case VariableDeclaration::RangeVariable:
                {
                    (yyval.expr) = create(new RangeVariableReference(var->expression(), var->slot), (yyloc), parseInfo);
                    break;
                }
                case VariableDeclaration::ExpressionVariable:
                {
                    (yyval.expr) = create(new ExpressionVariableReference(var->slot, var), (yyloc), parseInfo);
                    break;
                }
                case VariableDeclaration::FunctionArgument:
                {
                    (yyval.expr) = create(new ArgumentReference(var->sequenceType, var->slot), (yyloc), parseInfo);
                    break;
                }
                case VariableDeclaration::PositionalVariable:
                {
                    (yyval.expr) = create(new PositionalVariableReference(var->slot), (yyloc), parseInfo);
                    break;
                }
            }
            Q_ASSERT((yyval.expr));
            var->references.append((yyval.expr));
        }
        else
        {
            parseInfo->staticContext->error(tr("No variable by name %1 exists")
                                               .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(2) - (2)].qName))),
                                            ReportContext::XPST0008, fromYYLTYPE((yyloc), parseInfo));
        }
    }
    break;

  case 284:
/* Line 1269 of yacc.c.  */
#line 2399 "../../sdk/QueryTransformParser.ypp"
    {
        /* See: http://www.w3.org/TR/xpath20/#id-variables */
        (yyval.qName) = parseInfo->staticContext->namePool()->allocateQName(QString(), (yyvsp[(1) - (1)].sval));
    }
    break;

  case 285:
/* Line 1269 of yacc.c.  */
#line 2404 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.qName) = (yyvsp[(1) - (1)].qName);
    }
    break;

  case 286:
/* Line 1269 of yacc.c.  */
#line 2409 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (3)].expr);
    }
    break;

  case 287:
/* Line 1269 of yacc.c.  */
#line 2413 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new EmptySequence, (yyloc), parseInfo);
    }
    break;

  case 288:
/* Line 1269 of yacc.c.  */
#line 2418 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new ContextItem(), (yyloc), parseInfo);
    }
    break;

  case 289:
/* Line 1269 of yacc.c.  */
#line 2423 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = (yyvsp[(2) - (2)].expr);
    }
    break;

  case 290:
/* Line 1269 of yacc.c.  */
#line 2428 "../../sdk/QueryTransformParser.ypp"
    {
        if(XPathHelper::isReservedNamespace((yyvsp[(1) - (4)].qName).namespaceURI()))
        { /* We got a call to a builtin function. */
            const ReflectYYLTYPE ryy((yyloc), parseInfo);

            const Expression::Ptr
                func(parseInfo->staticContext->
                functionSignatures()->createFunctionCall((yyvsp[(1) - (4)].qName), (yyvsp[(3) - (4)].expressionList), parseInfo->staticContext, &ryy));

            if(func)
                (yyval.expr) = create(func, (yyloc), parseInfo);
            else
            {
                parseInfo->staticContext->error(tr("No function by name %1 is available.")
                                                   .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(1) - (4)].qName))),
                                                ReportContext::XPST0017, fromYYLTYPE((yyloc), parseInfo));
            }
        }
        else /* It's a call to a function created with 'declare function'.*/
        {
            qDebug() << "Found user function call.." << endl;
            (yyval.expr) = create(new UserFunctionCallsite((yyvsp[(1) - (4)].qName), (yyvsp[(3) - (4)].expressionList).count()), (yyloc), parseInfo);

            (yyval.expr)->setOperands((yyvsp[(3) - (4)].expressionList));
            parseInfo->userFunctionCallsites.append((yyval.expr));
        }
    }
    break;

  case 291:
/* Line 1269 of yacc.c.  */
#line 2457 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expressionList) = Expression::List();
    }
    break;

  case 292:
/* Line 1269 of yacc.c.  */
#line 2462 "../../sdk/QueryTransformParser.ypp"
    {
        Expression::List list;
        list.append((yyvsp[(1) - (1)].expr));
        (yyval.expressionList) = list;
    }
    break;

  case 299:
/* Line 1269 of yacc.c.  */
#line 2509 "../../sdk/QueryTransformParser.ypp"
    {
                        (yyval.enums.tokenizerPosition) = parseInfo->tokenizer->commenceScanOnly();
                        parseInfo->scanOnlyStack.push(true);
                    }
    break;

  case 300:
/* Line 1269 of yacc.c.  */
#line 2518 "../../sdk/QueryTransformParser.ypp"
    {
                        ++parseInfo->elementConstructorDepth;
                        Expression::List constructors;

                        parseInfo->resolvers.push(parseInfo->staticContext->namespaceBindings());

                        /* Fix up attributes and namespace declarations. */
                        const NamespaceResolver::Ptr resolver(new DelegatingNamespaceResolver(parseInfo->staticContext->namespaceBindings()));
                        const NamePool::Ptr namePool(parseInfo->staticContext->namePool());
                        const int len = (yyvsp[(4) - (4)].attributeHolders).size();
                        QSet<QName::PrefixCode> usedDeclarations;

                        /* Whether xmlns="" has been encountered. */
                        bool hasDefaultDeclaration = false;

                        /* For each attribute & namespace declaration, do: */
                        for(int i = 0; i < len; ++i)
                        {
                            QString strLocalName;
                            QString strPrefix;

                            XPathHelper::splitQName((yyvsp[(4) - (4)].attributeHolders).at(i).first, strPrefix, strLocalName);
                            const QName::PrefixCode prefix = namePool->allocatePrefix(strPrefix);

                            /* This can seem a bit weird. However, this name is ending up in a NamespaceBinding
                             * which consider its prefix a... prefix. So, a namespace binding name can in some cases
                             * be a local name, but that's just as the initial syntactical construct. */
                            const QName::LocalNameCode localName = namePool->allocatePrefix(strLocalName);

                            if(prefix == StandardPrefixes::xmlns ||
                               (prefix == StandardPrefixes::empty && localName == StandardPrefixes::xmlns))
                            {
                                if(localName == StandardPrefixes::xmlns)
                                    hasDefaultDeclaration = true;

                                /* We have a namespace declaration. */

                                const Expression::Ptr nsExpr((yyvsp[(4) - (4)].attributeHolders).at(i).second);

                                const QString strNamespace(nsExpr->is(Expression::IDEmptySequence) ? QString() : nsExpr->as<Literal>()->item().stringValue());

                                if(strNamespace.contains(QLatin1Char('{')))
                                {
                                    parseInfo->staticContext->error(tr("The namespace URI must be a constant and cannot "
                                                                       "use enclosed expressions."),
                                                                    ReportContext::XQST0022, fromYYLTYPE((yyloc), parseInfo));
                                }

                                const QName::NamespaceCode ns = namePool->allocateNamespace(strNamespace);
                                qDebug() << "NAMESPACE:" << strNamespace;

                                if(ns == StandardNamespaces::empty)
                                {
                                    if(localName != StandardPrefixes::xmlns)
                                    {
                                        parseInfo->staticContext->error(tr("The namespace URI cannot be the empty string when binding to a prefix, %1.")
                                                                           .arg(formatURI(strPrefix)),
                                                                        ReportContext::XQST0085, fromYYLTYPE((yyloc), parseInfo));
                                    }
                                }
                                else if(!QUrl(strNamespace).isValid())
                                {
                                    parseInfo->staticContext->error(tr("%1 is an invalid namespace URI.").arg(formatURI(strNamespace)),
                                                                    ReportContext::XQST0022, fromYYLTYPE((yyloc), parseInfo));
                                }

                                if(prefix == StandardPrefixes::xmlns && localName == StandardPrefixes::xmlns)
                                {
                                    parseInfo->staticContext->error(tr("It is not possible to bind to the prefix %1")
                                                                       .arg(formatKeyword("xmlns")),
                                                                    ReportContext::XQST0070, fromYYLTYPE((yyloc), parseInfo));
                                }

                                if(ns == StandardNamespaces::xml && prefix != StandardPrefixes::xml)
                                {
                                    parseInfo->staticContext->error(tr("Namespace %1 can only be bound to %2(and it is in either case pre-declared).")
                                                                       .arg(formatURI(namePool->stringForNamespace(StandardNamespaces::xml)))
                                                                       .arg(formatKeyword("xml")),
                                                                    ReportContext::XQST0070, fromYYLTYPE((yyloc), parseInfo));
                                }

                                if(localName == StandardPrefixes::xml && ns != StandardNamespaces::xml)
                                {
                                    parseInfo->staticContext->error(tr("Prefix %1 can only be bound to %2(and it is in either case pre-declared).")
                                                                       .arg(formatKeyword("xml"))
                                                                       .arg(formatURI(namePool->stringForNamespace(StandardNamespaces::xml))),
                                                                    ReportContext::XQST0070, fromYYLTYPE((yyloc), parseInfo));
                                }

                                NamespaceBinding nb;

                                if(localName == StandardPrefixes::xmlns)
                                    nb = NamespaceBinding(StandardPrefixes::empty, ns);
                                else
                                    nb = NamespaceBinding(localName, ns);

                                if(usedDeclarations.contains(nb.prefix()))
                                {
                                    parseInfo->staticContext->error(tr("Two namespace declaration attributes have the same name: %1.")
                                                                       .arg(formatKeyword(namePool->stringForPrefix(nb.prefix()))),
                                                                    ReportContext::XQST0071, fromYYLTYPE((yyloc), parseInfo));

                                }
                                else
                                    usedDeclarations.insert(nb.prefix());

                                /* If the user has bound the XML namespace correctly, we in either
                                 * case don't want to output it.
                                 *
                                 * We only have to check the namespace parts since the above checks has ensured
                                 * consistency in the prefix parts. */
                                if(ns != StandardNamespaces::xml)
                                {
                                    /* We don't want default namespace declarations when the
                                     * default namespace already is empty. */
                                    if(!(ns == StandardNamespaces::empty          &&
                                         localName == StandardNamespaces::xmlns   &&
                                         resolver->lookupNamespaceURI(StandardPrefixes::empty) == StandardNamespaces::empty))
                                    {
                                        constructors.append(create(new NamespaceConstructor(nb), (yyloc), parseInfo));
                                        resolver->addBinding(nb);
                                    }
                                }
                            }
                        }

                        if(parseInfo->elementConstructorDepth == 1 && !hasDefaultDeclaration)
                        {
                            /* TODO But mostly this isn't needed, since the default element
                             * namespace is empty? How does this at all work? */
                            const NamespaceBinding def(StandardPrefixes::empty, resolver->lookupNamespaceURI(StandardPrefixes::empty));
                            constructors.append(create(new NamespaceConstructor(def), (yyloc), parseInfo));
                        }

                        parseInfo->staticContext->setNamespaceBindings(resolver);
                        (yyval.expressionList) = constructors;

                        /* Resolve the name of the element, now that the namespace attributes are read. */
                        {
                            const ReflectYYLTYPE ryy((yyloc), parseInfo);

                            const QName ele = QNameConstructor::expandQName<StaticContext::Ptr,
                                                                            ReportContext::XPST0081,
                                                                            ReportContext::XPST0081>((yyvsp[(2) - (4)].sval), parseInfo->staticContext, resolver, &ryy);
                            parseInfo->tagStack.push(ele);
                        }

                        parseInfo->tokenizer->resumeTokenizationFrom((yyvsp[(3) - (4)].enums.tokenizerPosition));
                    }
    break;

  case 301:
/* Line 1269 of yacc.c.  */
#line 2670 "../../sdk/QueryTransformParser.ypp"
    {
        /* We add the content constructor after the attribute constructors. This might result
         * in nested ExpressionSequences, but it will be optimized away later on. */

        Expression::List attributes((yyvsp[(5) - (8)].expressionList));
        const NamePool::Ptr namePool(parseInfo->staticContext->namePool());
        const int len = (yyvsp[(7) - (8)].attributeHolders).size();
        QSet<QName> declaredAttributes;
        declaredAttributes.reserve(len);

        /* For each namespace, resolve its name(now that we have resolved the namespace declarations) and
         * turn it into an attribute constructor. */
        for(int i = 0; i < len; ++i)
        {
            QString strLocalName;
            QString strPrefix;

            XPathHelper::splitQName((yyvsp[(7) - (8)].attributeHolders).at(i).first, strPrefix, strLocalName);
            const QName::PrefixCode prefix = namePool->allocatePrefix(strPrefix);
            const QName::LocalNameCode localName = namePool->allocateLocalName(strLocalName);

            if(prefix == StandardPrefixes::xmlns ||
               (prefix == StandardPrefixes::empty && localName == StandardLocalNames::xmlns))
            {
                /* It's a namespace declaration, and we've already handled those above. */
                continue;
            }
            else
            {
                const ReflectYYLTYPE ryy((yyloc), parseInfo);
                const QName att = QNameConstructor::expandQName<StaticContext::Ptr,
                                                                ReportContext::XPST0081,
                                                                ReportContext::XPST0081>((yyvsp[(7) - (8)].attributeHolders).at(i).first, parseInfo->staticContext,
                                                                                         parseInfo->staticContext->namespaceBindings(),
                                                                                         &ryy, true);
                if(declaredAttributes.contains(att))
                {
                    parseInfo->staticContext->error(tr("An attribute by name %1 has already appeared on this element.")
                                                      .arg(formatKeyword(parseInfo->staticContext->namePool(), att)),
                                            ReportContext::XQST0040, fromYYLTYPE((yyloc), parseInfo));

                }
                else
                    declaredAttributes.insert(att);

                /* wrapLiteral() needs the SourceLocationReflection of the AttributeConstructor, but
                 * it's unknown inside the arguments to its constructor. Hence we have to do this workaround of setting
                 * it twice.
                 *
                 * The AttributeConstructor's arguments are just dummies. */
                const Expression::Ptr ctor(create(new AttributeConstructor((yyvsp[(7) - (8)].attributeHolders).at(i).second, (yyvsp[(7) - (8)].attributeHolders).at(i).second), (yyloc), parseInfo));

                Expression::List ops;
                ops.append(wrapLiteral(toItem(QNameValue::fromValue(namePool, att)), parseInfo->staticContext, ctor.get()));
                ops.append((yyvsp[(7) - (8)].attributeHolders).at(i).second);
                ctor->setOperands(ops);

                attributes.append(ctor);
            }
        }

        Expression::Ptr contentOp;

        if(attributes.isEmpty())
            contentOp = (yyvsp[(8) - (8)].expr);
        else
        {
            attributes.append((yyvsp[(8) - (8)].expr));
            contentOp = create(new ExpressionSequence(attributes), (yyloc), parseInfo);
        }

        const Expression::Ptr name(create(new Literal(toItem(QNameValue::fromValue(parseInfo->staticContext->namePool(), parseInfo->tagStack.top()))), (yyloc), parseInfo));
        (yyval.expr) = create(new ElementConstructor(name, contentOp), (yyloc), parseInfo);

        /* Restore the old context. We don't want the namespaces
         * to be in-scope for expressions appearing after the
         * element they appeared on. */
        parseInfo->staticContext->setNamespaceBindings(parseInfo->resolvers.pop());
        parseInfo->tagStack.pop();

        --parseInfo->elementConstructorDepth;
    }
    break;

  case 302:
/* Line 1269 of yacc.c.  */
#line 2754 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new EmptySequence(), (yyloc), parseInfo);
    }
    break;

  case 303:
/* Line 1269 of yacc.c.  */
#line 2758 "../../sdk/QueryTransformParser.ypp"
    {
        if(!(yyvsp[(4) - (5)].qName).isLexicallyEqual(parseInfo->tagStack.top()))
        {
            parseInfo->staticContext->error(tr("A direct element constructor is not "
                                               "well-formed. %1 is ended with %2")
                                               .arg(formatKeyword(parseInfo->staticContext->namePool()->toLexical(parseInfo->tagStack.top())),
                                                    formatKeyword(parseInfo->staticContext->namePool()->toLexical((yyvsp[(4) - (5)].qName)))),
                                            ReportContext::XPST0003, fromYYLTYPE((yyloc), parseInfo));
        }

        if((yyvsp[(2) - (5)].expressionList).isEmpty())
            (yyval.expr) = create(new EmptySequence(), (yyloc), parseInfo);
        else if((yyvsp[(2) - (5)].expressionList).size() == 1)
            (yyval.expr) = (yyvsp[(2) - (5)].expressionList).first();
        else
            (yyval.expr) = create(new ExpressionSequence((yyvsp[(2) - (5)].expressionList)), (yyloc), parseInfo);
    }
    break;

  case 304:
/* Line 1269 of yacc.c.  */
#line 2777 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.attributeHolders) = AttributeHolderVector();
    }
    break;

  case 305:
/* Line 1269 of yacc.c.  */
#line 2781 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(1) - (2)].attributeHolders).append((yyvsp[(2) - (2)].attributeHolder));
        (yyval.attributeHolders) = (yyvsp[(1) - (2)].attributeHolders);
    }
    break;

  case 306:
/* Line 1269 of yacc.c.  */
#line 2787 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.attributeHolder) = qMakePair((yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].expr));
    }
    break;

  case 307:
/* Line 1269 of yacc.c.  */
#line 2792 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = createDirAttributeValue((yyvsp[(2) - (3)].expressionList), parseInfo, (yyloc));
    }
    break;

  case 308:
/* Line 1269 of yacc.c.  */
#line 2797 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = createDirAttributeValue((yyvsp[(2) - (3)].expressionList), parseInfo, (yyloc));
    }
    break;

  case 309:
/* Line 1269 of yacc.c.  */
#line 2802 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expressionList) = Expression::List();
    }
    break;

  case 310:
/* Line 1269 of yacc.c.  */
#line 2806 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(2) - (2)].expressionList).prepend(create(new SimpleContentConstructor((yyvsp[(1) - (2)].expr)), (yyloc), parseInfo));
        (yyval.expressionList) = (yyvsp[(2) - (2)].expressionList);
    }
    break;

  case 311:
/* Line 1269 of yacc.c.  */
#line 2811 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(2) - (2)].expressionList).prepend(create(new Literal(AtomicString::fromValue((yyvsp[(1) - (2)].sval))), (yyloc), parseInfo));
        (yyval.expressionList) = (yyvsp[(2) - (2)].expressionList);
    }
    break;

  case 312:
/* Line 1269 of yacc.c.  */
#line 2817 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expressionList) = Expression::List();
        parseInfo->isPreviousEnclosedExpr = false;
    }
    break;

  case 313:
/* Line 1269 of yacc.c.  */
#line 2822 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(1) - (2)].expressionList).append((yyvsp[(2) - (2)].expr));
        (yyval.expressionList) = (yyvsp[(1) - (2)].expressionList);
        parseInfo->isPreviousEnclosedExpr = false;
    }
    break;

  case 314:
/* Line 1269 of yacc.c.  */
#line 2828 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->staticContext->boundarySpacePolicy() == StaticContext::BSPStrip &&
           isWhitespaceOnly((yyvsp[(2) - (2)].sval)))
        {
            (yyval.expressionList) = (yyvsp[(1) - (2)].expressionList);
        }
        else
        {
            (yyvsp[(1) - (2)].expressionList).append(create(new TextNodeConstructor(create(new Literal(AtomicString::fromValue((yyvsp[(2) - (2)].sval))), (yyloc), parseInfo)), (yyloc), parseInfo));
            (yyval.expressionList) = (yyvsp[(1) - (2)].expressionList);
            parseInfo->isPreviousEnclosedExpr = false;
        }
    }
    break;

  case 315:
/* Line 1269 of yacc.c.  */
#line 2842 "../../sdk/QueryTransformParser.ypp"
    {
        (yyvsp[(1) - (2)].expressionList).append(create(new TextNodeConstructor(create(new Literal(AtomicString::fromValue((yyvsp[(2) - (2)].sval))), (yyloc), parseInfo)), (yyloc), parseInfo));
        (yyval.expressionList) = (yyvsp[(1) - (2)].expressionList);
        parseInfo->isPreviousEnclosedExpr = false;
    }
    break;

  case 316:
/* Line 1269 of yacc.c.  */
#line 2848 "../../sdk/QueryTransformParser.ypp"
    {
        /* We insert a text node constructor that send an empty text node between
         * the two enclosed expressions, in order to ensure that no space is inserted.
         *
         * However, we only do it when we have no node constructors. */
        if(parseInfo->isPreviousEnclosedExpr &&
           BuiltinTypes::xsAnyAtomicType->xdtTypeMatches((yyvsp[(2) - (2)].expr)->staticType()->itemType()) &&
           BuiltinTypes::xsAnyAtomicType->xdtTypeMatches((yyvsp[(1) - (2)].expressionList).last()->staticType()->itemType()))
            (yyvsp[(1) - (2)].expressionList).append(create(new TextNodeConstructor(create(new Literal(AtomicString::fromValue(QString())), (yyloc), parseInfo)), (yyloc), parseInfo));
        else
            parseInfo->isPreviousEnclosedExpr = true;

        (yyvsp[(1) - (2)].expressionList).append((yyvsp[(2) - (2)].expr));
        (yyval.expressionList) = (yyvsp[(1) - (2)].expressionList);
    }
    break;

  case 317:
/* Line 1269 of yacc.c.  */
#line 2865 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CommentConstructor(create(new Literal(AtomicString::fromValue((yyvsp[(2) - (2)].sval))), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 318:
/* Line 1269 of yacc.c.  */
#line 2870 "../../sdk/QueryTransformParser.ypp"
    {
        const ReflectYYLTYPE ryy((yyloc), parseInfo);
        const QString name
            (NCNameConstructor::validateTargetName<StaticContext::Ptr,
                                                   ReportContext::XPST0003,
                                                   ReportContext::XPST0003>((yyvsp[(2) - (3)].sval),
                                                                            parseInfo->staticContext, &ryy));

        (yyval.expr) = create(new ProcessingInstructionConstructor(
                             create(new Literal(AtomicString::fromValue((yyvsp[(2) - (3)].sval))), (yyloc), parseInfo),
                             create(new Literal(AtomicString::fromValue((yyvsp[(3) - (3)].sval))), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 325:
/* Line 1269 of yacc.c.  */
#line 2891 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new DocumentConstructor((yyvsp[(2) - (2)].expr)), (yyloc), parseInfo);
    }
    break;

  case 326:
/* Line 1269 of yacc.c.  */
#line 2896 "../../sdk/QueryTransformParser.ypp"
    {
                        /* This value is incremented before the action below is executed. */
                        ++parseInfo->elementConstructorDepth;
                     }
    break;

  case 327:
/* Line 1269 of yacc.c.  */
#line 2901 "../../sdk/QueryTransformParser.ypp"
    {
        Expression::Ptr effExpr;

        if((yyvsp[(4) - (4)].expr))
            effExpr = (yyvsp[(4) - (4)].expr);
        else
            effExpr = create(new EmptySequence(), (yyloc), parseInfo);

        const QName::NamespaceCode ns = parseInfo->resolvers.top()->lookupNamespaceURI(StandardPrefixes::empty);

        /* Ensure the default namespace gets counted as an in-scope binding, if such a one exists. If we're
         * a child of another constructor, it has already been done. */
        if(parseInfo->elementConstructorDepth == 1 && ns != StandardNamespaces::empty)
        {
            Expression::List exprList;

            /* We append the namespace constuctor before the body, in order to
             * comply with SequenceReceiver's contract. */
            const NamespaceBinding def(StandardPrefixes::empty, parseInfo->resolvers.top()->lookupNamespaceURI(StandardPrefixes::empty));
            exprList.append(create(new NamespaceConstructor(def), (yyloc), parseInfo));

            exprList.append(effExpr);

            effExpr = create(new ExpressionSequence(exprList), (yyloc), parseInfo);
        }

        --parseInfo->elementConstructorDepth;
        (yyval.expr) = create(new ElementConstructor((yyvsp[(2) - (4)].expr), effExpr), (yyloc), parseInfo);
    }
    break;

  case 328:
/* Line 1269 of yacc.c.  */
#line 2934 "../../sdk/QueryTransformParser.ypp"
    {
        const Expression::Ptr name(create(new AttributeNameValidator((yyvsp[(2) - (3)].expr)), (yyloc), parseInfo));

        if((yyvsp[(3) - (3)].expr))
            (yyval.expr) = create(new AttributeConstructor(name, create(new SimpleContentConstructor((yyvsp[(3) - (3)].expr)), (yyloc), parseInfo)), (yyloc), parseInfo);
        else
            (yyval.expr) = create(new AttributeConstructor(name, create(new EmptySequence(), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 329:
/* Line 1269 of yacc.c.  */
#line 2944 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new TextNodeConstructor(create(new SimpleContentConstructor((yyvsp[(2) - (2)].expr)), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 330:
/* Line 1269 of yacc.c.  */
#line 2949 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new CommentConstructor(create(new SimpleContentConstructor((yyvsp[(2) - (2)].expr)), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 331:
/* Line 1269 of yacc.c.  */
#line 2954 "../../sdk/QueryTransformParser.ypp"
    {
        if((yyvsp[(3) - (3)].expr))
        {
            (yyval.expr) = create(new ProcessingInstructionConstructor((yyvsp[(2) - (3)].expr),
                                         create(new SimpleContentConstructor((yyvsp[(3) - (3)].expr)), (yyloc), parseInfo)), (yyloc), parseInfo);
        }
        else
            (yyval.expr) = create(new ProcessingInstructionConstructor((yyvsp[(2) - (3)].expr), create(new EmptySequence(), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 332:
/* Line 1269 of yacc.c.  */
#line 2964 "../../sdk/QueryTransformParser.ypp"
    {
                        parseInfo->nodeTestSource = BuiltinTypes::attribute;
                   }
    break;

  case 333:
/* Line 1269 of yacc.c.  */
#line 2968 "../../sdk/QueryTransformParser.ypp"
    {
                        parseInfo->restoreNodeTestSource();
                   }
    break;

  case 334:
/* Line 1269 of yacc.c.  */
#line 2971 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new Literal(toItem(QNameValue::fromValue(parseInfo->staticContext->namePool(), (yyvsp[(2) - (3)].qName)))), (yyloc), parseInfo);
    }
    break;

  case 336:
/* Line 1269 of yacc.c.  */
#line 2977 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new Literal(toItem(QNameValue::fromValue(parseInfo->staticContext->namePool(), (yyvsp[(1) - (1)].qName)))), (yyloc), parseInfo);
    }
    break;

  case 338:
/* Line 1269 of yacc.c.  */
#line 2983 "../../sdk/QueryTransformParser.ypp"
    {
        if(BuiltinTypes::xsQName->xdtTypeMatches((yyvsp[(1) - (1)].expr)->staticType()->itemType()))
            (yyval.expr) = (yyvsp[(1) - (1)].expr);
        else
        {
            (yyval.expr) = create(new QNameConstructor((yyvsp[(1) - (1)].expr),
                                             parseInfo->staticContext->namespaceBindings()),
                        (yyloc), parseInfo);
        }
    }
    break;

  case 339:
/* Line 1269 of yacc.c.  */
#line 2998 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new NCNameConstructor(create(new Literal(AtomicString::fromValue((yyvsp[(1) - (1)].sval))), (yyloc), parseInfo)), (yyloc), parseInfo);
    }
    break;

  case 340:
/* Line 1269 of yacc.c.  */
#line 3002 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.expr) = create(new NCNameConstructor((yyvsp[(1) - (1)].expr)), (yyloc), parseInfo);
    }
    break;

  case 341:
/* Line 1269 of yacc.c.  */
#line 3007 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = makeGenericSequenceType((yyvsp[(1) - (1)].itemType), Cardinality::exactlyOne());
    }
    break;

  case 342:
/* Line 1269 of yacc.c.  */
#line 3011 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = makeGenericSequenceType((yyvsp[(1) - (2)].itemType), Cardinality::zeroOrOne());
    }
    break;

  case 343:
/* Line 1269 of yacc.c.  */
#line 3016 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = CommonSequenceTypes::ZeroOrMoreItems;
    }
    break;

  case 344:
/* Line 1269 of yacc.c.  */
#line 3020 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = (yyvsp[(2) - (2)].sequenceType);
    }
    break;

  case 345:
/* Line 1269 of yacc.c.  */
#line 3025 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = makeGenericSequenceType((yyvsp[(1) - (2)].itemType), (yyvsp[(2) - (2)].cardinality));
    }
    break;

  case 346:
/* Line 1269 of yacc.c.  */
#line 3030 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.sequenceType) = CommonSequenceTypes::Empty;
    }
    break;

  case 347:
/* Line 1269 of yacc.c.  */
#line 3034 "../../sdk/QueryTransformParser.ypp"
    {(yyval.cardinality) = Cardinality::exactlyOne();}
    break;

  case 348:
/* Line 1269 of yacc.c.  */
#line 3035 "../../sdk/QueryTransformParser.ypp"
    {(yyval.cardinality) = Cardinality::oneOrMore();}
    break;

  case 349:
/* Line 1269 of yacc.c.  */
#line 3036 "../../sdk/QueryTransformParser.ypp"
    {(yyval.cardinality) = Cardinality::zeroOrMore();}
    break;

  case 350:
/* Line 1269 of yacc.c.  */
#line 3037 "../../sdk/QueryTransformParser.ypp"
    {(yyval.cardinality) = Cardinality::zeroOrOne();}
    break;

  case 354:
/* Line 1269 of yacc.c.  */
#line 3043 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::item;
    }
    break;

  case 355:
/* Line 1269 of yacc.c.  */
#line 3048 "../../sdk/QueryTransformParser.ypp"
    {
        const SchemaType::Ptr t(parseInfo->staticContext->schemaDefinitions()->createSchemaType((yyvsp[(1) - (1)].qName)));

        if(!t)
        {
            parseInfo->staticContext->error(tr("The name %1 does not refer to any schema type")
                                               .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(1) - (1)].qName))), ReportContext::XPST0051, fromYYLTYPE((yyloc), parseInfo));
        }
        else if(BuiltinTypes::xsAnyAtomicType->wxsTypeMatches(t))
            (yyval.itemType) = AtomicType::Ptr(t);
        else
        {
            /* Try to give an intelligent message. */
            if(t->isComplexType())
            {
                parseInfo->staticContext->error(tr("%1 is an complex type. Casting to complex "
                                                   "types is not possible. However, casting "
                                                   "to atomic types such as %2 works")
                                                   .arg(formatType(parseInfo->staticContext->namePool(), t))
                                                   .arg(formatType(parseInfo->staticContext->namePool(), BuiltinTypes::xsInteger)),
                                                ReportContext::XPST0051, fromYYLTYPE((yyloc), parseInfo));
            }
            else
            {
                parseInfo->staticContext->error(tr("%1 is not an atomic type. Casting "
                                                   "is only possible to atomic types")
                                                   .arg(formatType(parseInfo->staticContext->namePool(), t)),
                                                ReportContext::XPST0051, fromYYLTYPE((yyloc), parseInfo));
            }
        }
    }
    break;

  case 363:
/* Line 1269 of yacc.c.  */
#line 3092 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::node;
    }
    break;

  case 364:
/* Line 1269 of yacc.c.  */
#line 3097 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::document;
    }
    break;

  case 365:
/* Line 1269 of yacc.c.  */
#line 3102 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO support for document element testing
        (yyval.itemType) = BuiltinTypes::document;
    }
    break;

  case 368:
/* Line 1269 of yacc.c.  */
#line 3111 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::text;
    }
    break;

  case 369:
/* Line 1269 of yacc.c.  */
#line 3116 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::comment;
    }
    break;

  case 370:
/* Line 1269 of yacc.c.  */
#line 3121 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::pi;
    }
    break;

  case 371:
/* Line 1269 of yacc.c.  */
#line 3126 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = LocalNameTest::create(BuiltinTypes::pi, parseInfo->staticContext->namePool()->allocateLocalName((yyvsp[(3) - (4)].sval)));
    }
    break;

  case 372:
/* Line 1269 of yacc.c.  */
#line 3131 "../../sdk/QueryTransformParser.ypp"
    {
        if(XPathHelper::isNCName((yyvsp[(3) - (4)].sval)))
        {
            (yyval.itemType) = LocalNameTest::create(BuiltinTypes::pi, parseInfo->staticContext->namePool()->allocateLocalName((yyvsp[(3) - (4)].sval)));
        }
        else
        {
            parseInfo->staticContext->warning(tr("%1 is not a valid name for a "
                                                 "processing-instruction. Therefore this "
                                                 "name test will never match.")
                                                 .arg(formatKeyword((yyvsp[(3) - (4)].sval))), fromYYLTYPE((yyloc), parseInfo));

            /* This one will never match. How can we compile it away? 'sum' is a dummy value. */
            (yyval.itemType) = LocalNameTest::create(BuiltinTypes::comment, StandardLocalNames::sum);
        }
    }
    break;

  case 375:
/* Line 1269 of yacc.c.  */
#line 3152 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::attribute;
    }
    break;

  case 376:
/* Line 1269 of yacc.c.  */
#line 3157 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::attribute;
    }
    break;

  case 377:
/* Line 1269 of yacc.c.  */
#line 3162 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = QNameTest::create(BuiltinTypes::attribute, (yyvsp[(3) - (4)].qName));
    }
    break;

  case 378:
/* Line 1269 of yacc.c.  */
#line 3166 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO Add support for type & name testing
        (yyval.itemType) = BuiltinTypes::attribute;
    }
    break;

  case 379:
/* Line 1269 of yacc.c.  */
#line 3171 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO Add support for type testing
        (yyval.itemType) = BuiltinTypes::attribute;
    }
    break;

  case 380:
/* Line 1269 of yacc.c.  */
#line 3177 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->staticContext->error(tr("%1 is not in the in-scope attribute "
                                           "declarations. Note that the schema import "
                                           "feature is not suppported.")
                                           .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(3) - (4)].qName))),
                                        ReportContext::XPST0008, fromYYLTYPE((yyloc), parseInfo));
        (yyval.itemType).reset();
    }
    break;

  case 381:
/* Line 1269 of yacc.c.  */
#line 3187 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::element;
    }
    break;

  case 382:
/* Line 1269 of yacc.c.  */
#line 3192 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = BuiltinTypes::element;
    }
    break;

  case 383:
/* Line 1269 of yacc.c.  */
#line 3197 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.itemType) = QNameTest::create(BuiltinTypes::element, (yyvsp[(3) - (4)].qName));
    }
    break;

  case 384:
/* Line 1269 of yacc.c.  */
#line 3202 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO Add support for type & name testing
        (yyval.itemType) = BuiltinTypes::element;
    }
    break;

  case 385:
/* Line 1269 of yacc.c.  */
#line 3208 "../../sdk/QueryTransformParser.ypp"
    {
        // TODO Add support for type testing
        (yyval.itemType) = BuiltinTypes::element;
    }
    break;

  case 386:
/* Line 1269 of yacc.c.  */
#line 3214 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->staticContext->error(tr("%1 is not in the in-scope attribute "
                                           "declarations. Note that the schema import "
                                           "feature is not supported.")
                                           .arg(formatKeyword(parseInfo->staticContext->namePool(), (yyvsp[(3) - (4)].qName))),
                                        ReportContext::XPST0008, fromYYLTYPE((yyloc), parseInfo));
        (yyval.itemType).reset();
    }
    break;

  case 389:
/* Line 1269 of yacc.c.  */
#line 3233 "../../sdk/QueryTransformParser.ypp"
    {
        if(parseInfo->nodeTestSource == BuiltinTypes::element)
            (yyval.qName) = parseInfo->staticContext->namePool()->allocateQName(parseInfo->staticContext->namespaceBindings()->lookupNamespaceURI(StandardPrefixes::empty), (yyvsp[(1) - (1)].sval));
        else
            (yyval.qName) = parseInfo->staticContext->namePool()->allocateQName(StandardNamespaces::empty, (yyvsp[(1) - (1)].sval));
    }
    break;

  case 392:
/* Line 1269 of yacc.c.  */
#line 3244 "../../sdk/QueryTransformParser.ypp"
    {
        (yyval.qName) = parseInfo->staticContext->namePool()->allocateQName(parseInfo->staticContext->defaultFunctionNamespace(), (yyvsp[(1) - (1)].sval));
    }
    break;

  case 396:
/* Line 1269 of yacc.c.  */
#line 3253 "../../sdk/QueryTransformParser.ypp"
    {
        parseInfo->staticContext->error(tr("The name of an extension expression must be in "
                                           "a namespace"),
                                        ReportContext::XPST0081, fromYYLTYPE((yyloc), parseInfo));
    }
    break;

  case 401:
/* Line 1269 of yacc.c.  */
#line 3266 "../../sdk/QueryTransformParser.ypp"
    {
        const ReflectYYLTYPE ryy((yyloc), parseInfo);

        (yyval.qName) = QNameConstructor::
             expandQName<StaticContext::Ptr,
                         ReportContext::XPST0081,
                         ReportContext::XPST0081>((yyvsp[(1) - (1)].sval), parseInfo->staticContext,
                                                  parseInfo->staticContext->namespaceBindings(), &ryy);

    }
    break;


/* Line 1269 of yacc.c.  */
#line 6022 "QueryTransformParser.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, parseInfo, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, parseInfo, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, parseInfo, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, parseInfo);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, parseInfo);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, parseInfo, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, parseInfo);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, parseInfo);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 1486 of yacc.c.  */
#line 3277 "../../sdk/QueryTransformParser.ypp"

// vim: et:ts=4:sw=4:sts=4:syntax=yacc

