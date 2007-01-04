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
#include <QPushButton>
#include <QtScript>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QScriptEngine engine;

    QPushButton button;
    QScriptValue scriptButton = engine.scriptValueFromQObject(&button);
    engine.globalObject().setProperty("button", scriptButton);

    engine.evaluate("button.text = 'Hello World!'");
    engine.evaluate("button.styleSheet = 'font-style: italic'");
    engine.evaluate("s = button.show");
    engine.evaluate("button.show = function foo() { print(this); with (this) s(); }");
    engine.evaluate("button.show()");

    return app.exec();
}
