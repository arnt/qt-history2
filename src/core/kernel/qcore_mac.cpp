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

QCFStringHelper::operator QString()
{
    if (string.isEmpty() && type) {
        CFIndex length = CFStringGetLength(type);
        const UniChar *chars = CFStringGetCharactersPtr(type);
        if (chars) {
            string = QString(reinterpret_cast<const QChar *>(chars), length);
        } else {
            QVarLengthArray<UniChar> buffer(length);
            CFStringGetCharacters(type, CFRangeMake(0, length), buffer);
            string = QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
        }
    }
    return string;
}

QCFStringHelper::operator CFStringRef()
{
    if (!type)
        type = CFStringCreateWithCharacters(0,
                reinterpret_cast<const UniChar *>(string.unicode()), string.length());
    return static_cast<CFStringRef>(type);
}
