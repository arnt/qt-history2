#ifndef TOOLS_H
#define TOOLS_H

#using <mscorlib.dll>

class QString;

System::String *QStringToString(const QString &qstring);
QString StringToQString(System::String *string);

#endif // TOOLS_H
