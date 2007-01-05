/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QUiLoader>
#include <QtScript>
#include <QWidget>
#include <QFile>
#include <QMessageBox>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QScriptEngine engine;

    QFile scriptFile(":/calculator.js");
    scriptFile.open(QIODevice::ReadOnly);
    qDebug() << engine.evaluate(scriptFile.readAll()).toString();
    scriptFile.close();

    QUiLoader loader;
    QFile uiFile(":/calculator.ui");
    uiFile.open(QIODevice::ReadOnly);
    QWidget *ui = loader.load(&uiFile);
    uiFile.close();

    QScriptValue calc = engine.newObject();
    engine.addRootObject(calc);

    QScriptValue ctor = engine.evaluate("Calculator");
    QScriptValue scriptUi = engine.scriptValueFromQObject(ui);
    qDebug() << ctor.call(calc, QScriptValueList() << scriptUi).toString();

    ui->show();
    return app.exec();
}
