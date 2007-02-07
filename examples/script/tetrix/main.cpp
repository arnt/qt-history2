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

#include "tetrixboard.h"

#include <QtGui>
#include <QtScript>
#include <QUiLoader>

struct QtMetaObject : private QObject
{
public:
    static const QMetaObject *get()
        { return &static_cast<QtMetaObject*>(0)->staticQtMetaObject; }
};

class TetrixUiLoader : public QUiLoader
{
public:
    TetrixUiLoader(QObject *parent = 0)
        : QUiLoader(parent)
        { }
    virtual QWidget *createWidget(const QString &className, QWidget *parent = 0,
                                  const QString &name = QString())
    {
        if (className == QLatin1String("TetrixBoard")) {
            QWidget *board = new TetrixBoard(parent);
            board->setObjectName(name);
            return board;
        }
        return QUiLoader::createWidget(className, parent, name);
    }
};

static QScriptValue evaluateFile(QScriptEngine &engine, const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    return engine.evaluate(file.readAll());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QScriptEngine engine;

    QScriptValue Qt = engine.newQMetaObject(QtMetaObject::get());
    Qt.setProperty("App", engine.newQObject(&app));
    engine.globalObject().setProperty("Qt", Qt);

    evaluateFile(engine, ":/tetrixpiece.js");
    evaluateFile(engine, ":/tetrixboard.js");
    evaluateFile(engine, ":/tetrixwindow.js");

    TetrixUiLoader loader;
    QFile uiFile(":/tetrixwindow.ui");
    uiFile.open(QIODevice::ReadOnly);
    QWidget *ui = loader.load(&uiFile);
    uiFile.close();

    QScriptValue tetrix = engine.newObject();
    tetrix.ref();

    QScriptValue ctor = engine.evaluate("TetrixWindow");
    QScriptValue scriptUi = engine.newQObject(ui);
    ctor.call(tetrix, QScriptValueList() << scriptUi);

    ui->resize(550, 370);
    ui->show();

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    return app.exec();
}
