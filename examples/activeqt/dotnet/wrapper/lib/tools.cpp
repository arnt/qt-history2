/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>

#using <mscorlib.dll>
#include <vcclr.h>

using namespace System;

String *QStringToString(const QString &qstring)
{
    return new String((const wchar_t *)qstring.utf16());
}

QString StringToQString(String *string)
{
    const wchar_t __pin *chars = PtrToStringChars(string);
    return QString::fromUtf16((const ushort *)chars);
}
