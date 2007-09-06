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
#ifndef Patternist_XQueryTokenizer_H
#define Patternist_XQueryTokenizer_H

#include <QHash>
#include <QStack>
#include <QString>
#include <QUrl>

#include "Tokenizer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    struct TokenMap;

    class XQueryTokenizer : public Tokenizer
    {
    public:
        XQueryTokenizer(const QString &query,
                        const QUrl &location);

        virtual Token nextToken(YYLTYPE *const sourceLocator);
        virtual QUrl uri() const;
        virtual int commenceScanOnly();
        virtual void resumeTokenizationFrom(const int position);

    private:
        /**
         * Tokenizer states. Organized alphabetically.
         */
        enum State
        {
            AfterAxisSeparator,
            AposAttributeContent,
            Axis,
            Default,
            ElementContent,
            EndTag,
            ItemType,
            KindTest,
            KindTestForPI,
            NamespaceDecl,
            NamespaceKeyword,
            OccurrenceIndicator,
            Operator,
            Pragma,
            PragmaContent,
            ProcessingInstructionContent,
            ProcessingInstructionName,
            QuotAttributeContent,
            StartTag,
            VarName,
            XMLComment,
            XMLSpaceDecl,
            XQueryVersion
        };

        /**
         * Returns the character corresponding to the builtin reference @p
         * reference. For instance, passing @c gt will give you '>' in return.
         *
         * If @p reference is an invalid character reference, a null QChar is
         * returned.
         *
         * @see QChar::isNull()
         */
        QChar charForReference(const QString &reference);

        inline Token tokenAndChangeState(const TokenType code,
                                         const State state,
                                         const int advance = 1);
        inline Token tokenAndChangeState(const TokenType code,
                                         const QString &value,
                                         const State state);
        inline Token tokenAndAdvance(const TokenType code,
                                     const int advance = 1);
        QChar tokenizeCharacterReference();
        inline Token tokenizeStringLiteral();
        inline Token tokenizeNumberLiteral();
        inline char peekAhead(const int length = 1) const;
        inline bool aheadEquals(const char *const chs,
                                const int len,
                                const int offset = 1) const;
        inline Token tokenizeNCName();
        static inline bool isOperatorKeyword(const TokenType);

        static inline bool isDigit(const char ch);
        static inline Token error();
        inline TokenType consumeWhitespace();
        inline char peekCurrent() const;
        static inline bool isNCNameStart(const QChar ch);
        static inline bool isNCNameBody(const QChar ch);
        static inline const TokenMap *lookupKeyword(const QString &keyword);
        inline const QChar current() const;
        inline void popState();
        inline void pushState(const State state);
        inline State state() const;
        inline void setState(const State s);
        static bool isTypeToken(const TokenType t);

        inline Token tokenizeNCNameOrQName();
        /**
         * Advances m_pos until content is encountered.
         *
         * Returned is the length stretching from m_pos when starting, until
         * @p content is encountered. @p content is not included in the length.
         */
        int scanUntil(const char *const content);

        /**
         * Same as calling:
         * @code
         * pushState(currentState());
         * @endcode
         */
        inline void pushState();

        /**
         * Consumes only whitespace, in the traditional sense. The function exits
         * if non-whitespace is encountered, such as the start of a comment.
         *
         * @returns @c true if the end was reached, otherwise @c false
         */
        inline bool consumeRawWhitespace();

        /**
         * @short Parses comments: <tt>(: comment content :)</tt>. It recurses for
         * parsing nested comments.
         *
         * It is assumed that the start token for the comment, "(:", has
         * already been parsed.
         *
         * Typically, don't call this function, but ignoreWhitespace().
         *
         * @see <a href="http://www.w3.org/TR/xpath20/#comments">XML Path Language (XPath)
         * 2.0, 2.6 Comments</a>
         * @returns
         * - SUCCESS if everything went ok
         * - ERROR if there was an error in parsing one or more comments
         * - END_OF_FILE if the end was reached
         */
        Tokenizer::TokenType consumeComment();

        /**
         * Determines whether @p code is a keyword
         * that is followed by a second keyword. For instance <tt>declare
         * function</tt>.
         */
        static inline bool isPhraseKeyword(const TokenType code);

        inline bool atEnd() const
        {
            return m_pos == m_length;
        }

        Token nextToken();
        /**
         * Instead of recognizing and tokenizing embedded expressions in
         * direct attriute constructors, this function is essentially a mini
         * recursive-descent parser that has the necessary logic to recognize
         * embedded expressions and their potentially interfering string literals, in
         * order to scan to the very end of the attribute value, and return the
         * whole as a string.
         *
         * There is of course syntax errors this function will not detect, but
         * that is ok since the attributes will be parsed once more.
         *
         * A not very elegant function but which gets the job done.
         *
         * @see commenceScanOnly(), resumeTokenizationFrom()
         */
        Token attributeAsRaw(const QChar separator,
                             int &stack,
                             const int startPos,
                             const bool inLiteral,
                             QString &result);

        const QString           m_data;
        const int               m_length;
        const QUrl              m_location;
        State                   m_state;
        QStack<State>           m_stateStack;
        int                     m_pos;

        /**
         * The current line number.
         *
         * The line number and column number both starts at 1.
         */
        int                     m_line;

        /**
         * The offset into m_length for where
         * the current column starts. So m_length - m_columnOffset
         * is the current column.
         *
         * The line number and column number both starts at 1.
         */
        int                     m_columnOffset;

        const NamePool::Ptr     m_namePool;
        QStack<Token>           m_tokenStack;
        QHash<QString, QChar>   m_charRefs;
        bool                    m_scanOnly;

        Q_DISABLE_COPY(XQueryTokenizer)
    };
}

QT_END_HEADER 

#endif
