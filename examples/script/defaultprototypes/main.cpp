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

#include <QtGui>
#include <QtScript>
#include "prototypes.h"

Q_DECLARE_METATYPE(QListWidgetItem*)
Q_DECLARE_METATYPE(QListWidget*)

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QScriptEngine engine;

    ListWidgetItemPrototype lwiProto;
    engine.setDefaultPrototype(qMetaTypeId<QListWidgetItem*>(),
                               engine.newQObject(&lwiProto));

    ListWidgetPrototype lwProto;
    engine.setDefaultPrototype(qMetaTypeId<QListWidget*>(),
                               engine.newQObject(&lwProto));

    QListWidget listWidget;
    engine.globalObject().setProperty("listWidget",
                                      engine.newQObject(&listWidget));

    QFile file(":/code.js");
    file.open(QIODevice::ReadOnly);
    QScriptValue result = engine.evaluate(file.readAll());
    file.close();
    if (engine.hasUncaughtException()) {
        int lineNo = engine.uncaughtExceptionLineNumber();
        qWarning() << "line" << lineNo << ":" << result.toString();
    }

    return app.exec();
}
