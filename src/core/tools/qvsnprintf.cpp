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

#include "qplatformdefs.h"

#include "qbytearray.h"
#include "qstring.h"

#include "string.h"

#ifndef QT_VSNPRINTF

int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    if (!str || !fmt)
        return -1;

    QString buf;
    buf.vsprintf(fmt, ap);

    QByteArray ba = buf.toLocal8Bit();

    if (n > 0) {
        size_t blen = qMin(size_t(ba.length()), size_t(n - 1));
        memcpy(str, ba.constData(), blen);
        str[blen] = '\0'; // make sure str is always 0 terminated
    }

    return ba.length();
}

#else

#include <stdio.h>

int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    return QT_VSNPRINTF(str, n, fmt, ap);
}

#endif

// qsnprintf and qvsnprintf are declared in qbytearray.h


/*! \fn int qvsnprintf(char *str, size_t n, char const *fmt, va_list ap)

  \relates QByteArray

  A portable vsnprintf() function. Will call ::vsnprintf(),
  ::_vsnprintf() or ::vsnprintf_s depending on the system
  or fall back to an internal version.

  The caller is responsible to call va_end on \a ap.

  \warning Since vsnprintf() shows different behavior on certain
  platforms, you should not rely on the return value or on the fact
  that you will always get a 0 terminated string back.

  Ideally, you should never call this function but use QString::sprintf()
  instead.

  \sa qvsnprintf, QString::sprintf()
*/

/*! \fn int qsnprintf(char *str, size_t n, char const *fmt, ...);

  \relates QByteArray

  A portable snprintf() function, calls qvsnprintf.

  \warning Call this function only when you know what you are doing
  since it shows different behavior on certain platforms.
  Use QString::sprintf() to format a string instead.

  \sa qvsnprintf, QString::sprintf()
*/

int qsnprintf(char *str, size_t n, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int ret = qvsnprintf(str, n, fmt, ap);
    va_end(ap);

    return ret;
}
