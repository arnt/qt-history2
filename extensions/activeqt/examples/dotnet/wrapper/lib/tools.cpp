#include <qstring.h>

#using <mscorlib.dll>
using namespace System;

String *QStringToString(const QString &qstring)
{
    return new String(qstring.ucs2());
}

QString StringToQString(String *string)
{
    wchar_t __pin *chars = &string->ToCharArray()[0];
    return QString::fromUcs2(chars);
}
