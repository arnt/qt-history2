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

    QUiLoader loader;
    QFile uiFile(":/calculator.ui");
    uiFile.open(QIODevice::ReadOnly);
    QWidget *calc = loader.load(&uiFile);

    QScriptEngine engine;
    QScriptValue scriptCalc = engine.scriptValueFromQObject(calc);
    engine.globalObject().setProperty("calc", scriptCalc);

    QFile scriptFile(":/calculator.js");
    scriptFile.open(QIODevice::ReadOnly);
    QScriptValue result = engine.evaluate(scriptFile.readAll());
    scriptFile.close();
    if (result.isError()) {
        QMessageBox::warning(0, QLatin1String("Script Error"), result.toString());
        return (-1);
    }

    calc->show();
    return app.exec();
}
