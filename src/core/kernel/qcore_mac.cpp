/****************************************************************************
**
** Definition of various Qt/Mac specifics.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcore_mac.h"
#include "qvarlengtharray.h"

QString QCFStringHelper::cfstring2qstring(CFStringRef str)
{
    CFIndex length = CFStringGetLength(str);
    const UniChar *chars = CFStringGetCharactersPtr(str);
    if (chars)
        return QString(reinterpret_cast<const QChar *>(chars), length);

    QVarLengthArray<UniChar> buffer(length);
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
    return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);    
}

QCFStringHelper::operator QString() const
{
    if (string.isEmpty() && type)
        const_cast<QCFStringHelper *>(this)->string = cfstring2qstring(type);
    return string;
}

CFStringRef QCFStringHelper::qstring2cfstring(const QString &string)
{
    return CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(string.unicode()),
                                        string.length());
}

QCFStringHelper::operator CFStringRef() const
{
    if (!type)
        const_cast<QCFStringHelper *>(this)->type = qstring2cfstring(string);
    return type;
}
