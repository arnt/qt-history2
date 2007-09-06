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

#ifndef Patternist_Debug_H
#define Patternist_Debug_H

/**
 * @file
 * @short Contains macros for debugging.
 */

#include <QtDebug>

/**
 * @short Enables detailed parser debug output.
 *
 * If this macro is defined, @em a @em lot of debugging information will be outputted.
 * This is all the state transitions, token shifting, and rule reductions that
 * the parser do.
 *
 * This is automatically disabled if @c QT_NO_DEBUG is defined.
 */
#define Patternist_DEBUG_PARSER

#undef Patternist_DEBUG_PARSER // disable it for now

#ifdef QT_NO_DEBUG
#undef Patternist_DEBUG_PARSER
#endif

/**
 * @def Patternist_TOKENIZER_MSG(msg)
 * @short Prints a debug message, used in tokenizer code.
 *
 * Used in tokenizer code and prints the debug message @p msg to @c stderr. Valid
 * types of @p msg is what qDebug() accepts.
 *
 * A no-op is generated if @c QT_NO_DEBUG is defined.
 */

#ifdef Patternist_DEBUG_PARSER
#   define Patternist_TOKENIZER_MSG(msg) qDebug() << "Tokenizer message:" << msg;
#else
#   define Patternist_TOKENIZER_MSG(msg) do{}while(0)
#endif

#endif
// vim: et:ts=4:sw=4:sts=4
