/****************************************************************************
**
** Copyright (C) 2001-2004 Roberto Raggi
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef ERRORS_H
#define ERRORS_H

#include <qstring.h>

struct Error
{
    int code;
    int level;
    QString text;

    inline Error(int c, int l, const QString &s)
        : code(c), level(l), text(s) {}
};

class Errors
{
public:
    QT_STATIC_CONST Error& InternalError;
    QT_STATIC_CONST Error& SyntaxError;
    QT_STATIC_CONST Error& ParseError;
};

#endif
