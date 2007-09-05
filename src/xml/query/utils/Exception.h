/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_Exception_H
#define Patternist_Exception_H

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short An exception, used for signalling errors at runtime or compile time.
     *
     * @note This class should be thrown and is thrown by value. The
     * proper way of catching it is by reference.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @see <a href="http://www.parashift.com/c++-faq-lite/exceptions.html">C++ FAQ Lite,
     * [17] Exceptions and error handling</a>
     * @ingroup Patternist
     */
    class Exception
    {
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
