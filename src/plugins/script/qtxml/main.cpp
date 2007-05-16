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

#include <QtScript/QScriptExtensionPlugin>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

#include <QtXml/QXmlStreamReader>
#include <QtXml/QXmlStreamWriter>

#include "../global.h"

QScriptValue constructXmlStreamAttributeClass(QScriptEngine *eng);
QScriptValue constructXmlStreamEntityDeclarationClass(QScriptEngine *eng);
QScriptValue constructXmlStreamNamespaceDeclarationClass(QScriptEngine *eng);
QScriptValue constructXmlStreamNotationDeclarationClass(QScriptEngine *eng);
QScriptValue constructXmlStreamReaderClass(QScriptEngine *eng);
QScriptValue constructXmlStreamWriterClass(QScriptEngine *eng);

class QtXmlScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList QtXmlScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt.xml");
    return list;
}

void QtXmlScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt.xml")) {
        QScriptValue extensionObject = engine->globalObject();
        extensionObject.setProperty("QXmlStreamAttribute", constructXmlStreamAttributeClass(engine));
        extensionObject.setProperty("QXmlStreamEntityDeclaration", constructXmlStreamEntityDeclarationClass(engine));
        extensionObject.setProperty("QXmlStreamNamespaceDeclaration", constructXmlStreamNamespaceDeclarationClass(engine));
        extensionObject.setProperty("QXmlStreamNotationDeclaration", constructXmlStreamNotationDeclarationClass(engine));
        extensionObject.setProperty("QXmlStreamReader", constructXmlStreamReaderClass(engine));
//        extensionObject.setProperty("QXmlStreamWriter", constructXmlStreamWriterClass(engine));
    } else {
        Q_ASSERT_X(false, "initialize", qPrintable(key));
    }
}

Q_EXPORT_STATIC_PLUGIN(QtXmlScriptPlugin)
Q_EXPORT_PLUGIN2(qtscriptxml, QtXmlScriptPlugin)
