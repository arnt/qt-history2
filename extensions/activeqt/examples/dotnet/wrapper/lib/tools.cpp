#include <qstring.h>

#using <mscorlib.dll>
#include <vcclr.h>

using namespace System;

String *QStringToString(const QString &qstring)
{
    return new String(qstring.ucs2());
}

QString StringToQString(String *string)
{
    wchar_t __pin *chars = PtrToStringChars(string);
    return QString::fromUcs2(chars);
}
