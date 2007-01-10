/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtScript>

template <class Container>
QScriptValue toScriptValue(QScriptEngine *eng, const Container &cont)
{
    QScriptValue a = eng->newArray();
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    for (it = begin; it != end; ++it)
        a.setProperty(quint32(it - begin), qScriptValueFromValue(eng, *it));
    return a;
}

template <class Container>
void fromScriptValue(const QScriptValue &value, Container &cont)
{
    quint32 len = value.property("length").toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        QScriptValue item = value.property(i);
        cont.push_back(qscript_cast<typename Container::value_type>(item));
    }
}

typedef QVector<int> IntVector;
typedef QVector<QString> StringVector;

Q_DECLARE_METATYPE(IntVector)
Q_DECLARE_METATYPE(StringVector)

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QScriptEngine eng;
    // register our custom types
    qScriptRegisterMetaType<IntVector>(&eng, toScriptValue, fromScriptValue);
    qScriptRegisterMetaType<StringVector>(&eng, toScriptValue, fromScriptValue);

    QScriptValue val = eng.evaluate("[1, 4, 7, 11, 50, 3, 19, 60]");

    fprintf(stdout, "Script array: %s\n", qPrintable(val.toString()));

    IntVector iv = qscript_cast<IntVector>(val);

    fprintf(stdout, "qscript_cast to QVector<int>: ");
    for (int i = 0; i < iv.size(); ++i)
        fprintf(stdout, "%s%d", (i > 0) ? "," : "", iv.at(i));
    fprintf(stdout, "\n");

    val = eng.evaluate("[9, 'foo', 46.5, 'bar', 'Qt', 555, 'hello']");

    fprintf(stdout, "Script array: %s\n", qPrintable(val.toString()));

    StringVector sv = qscript_cast<StringVector>(val);

    fprintf(stdout, "qscript_cast to QVector<QString>: ");
    for (int i = 0; i < sv.size(); ++i)
        fprintf(stdout, "%s%s", (i > 0) ? "," : "", qPrintable(sv.at(i)));
    fprintf(stdout, "\n");

    return 0;
}
