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

#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QSize>
#include <QtCore/QSizeF>
#include <QtCore/QLine>
#include <QtCore/QLineF>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <qdebug.h>

#include "../global.h"

QScriptValue constructByteArrayClass(QScriptEngine *eng);
QScriptValue constructDirClass(QScriptEngine *eng);
QScriptValue constructEventClass(QScriptEngine *eng);
QScriptValue constructFileClass(QScriptEngine *eng);
QScriptValue constructFileInfoClass(QScriptEngine *eng);
QScriptValue constructIODeviceClass(QScriptEngine *eng);
QScriptValue constructModelIndexClass(QScriptEngine *eng);
QScriptValue constructProcessClass(QScriptEngine *eng);
QScriptValue constructStringRefClass(QScriptEngine *eng);
QScriptValue constructTextStreamClass(QScriptEngine *eng);
QScriptValue constructTimeLineClass(QScriptEngine *eng);
QScriptValue constructTimerClass(QScriptEngine *eng);
void extendStringPrototype(QScriptEngine *eng);

static QScriptValue scriptValueFromQPoint(QScriptEngine *engine, const QPoint &point)
{
    return engine->evaluate(QString::fromLatin1("new QPoint(%0, %1)")
                            .arg(point.x()).arg(point.y()));
}

static void scriptValueToQPoint(const QScriptValue &value, QPoint &point)
{
    point = QPoint(value.property(QLatin1String("x")).toInt32(),
                   value.property(QLatin1String("y")).toInt32());
}

static QScriptValue scriptValueFromQPointF(QScriptEngine *engine, const QPointF &point)
{
    return engine->evaluate(QString::fromLatin1("new QPoint(%0, %1)")
                            .arg(point.x()).arg(point.y()));
}

static void scriptValueToQPointF(const QScriptValue &value, QPointF &point)
{
    point = QPointF(value.property(QLatin1String("x")).toNumber(),
                    value.property(QLatin1String("y")).toNumber());
}

static QScriptValue scriptValueFromQSize(QScriptEngine *engine, const QSize &size)
{
    return engine->evaluate(QString::fromLatin1("new QSize(%0, %1)")
                            .arg(size.width()).arg(size.height()));
}

static void scriptValueToQSize(const QScriptValue &value, QSize &size)
{
    size = QSize(value.property(QLatin1String("width")).toInt32(),
                 value.property(QLatin1String("height")).toInt32());
}

static QScriptValue scriptValueFromQSizeF(QScriptEngine *engine, const QSizeF &size)
{
    return engine->evaluate(QString::fromLatin1("new QSize(%0, %1)")
                            .arg(size.width()).arg(size.height()));
}

static void scriptValueToQSizeF(const QScriptValue &value, QSizeF &size)
{
    size = QSizeF(value.property(QLatin1String("width")).toNumber(),
                  value.property(QLatin1String("height")).toNumber());
}

static QScriptValue scriptValueFromQLine(QScriptEngine *engine, const QLine &line)
{
    return engine->evaluate(QString::fromLatin1("new QLine(%0, %1, %2, %3)")
                            .arg(line.x1()).arg(line.y1())
                            .arg(line.x2()).arg(line.y2()));
}

static void scriptValueToQLine(const QScriptValue &value, QLine &line)
{
    QScriptValue::ResolveFlags flags = QScriptValue::ResolvePrototype;
    line = QLine(value.property(QLatin1String("x1"), flags).toInt32(),
                 value.property(QLatin1String("y1"), flags).toInt32(),
                 value.property(QLatin1String("x2"), flags).toInt32(),
                 value.property(QLatin1String("y2"), flags).toInt32());
}

static QScriptValue scriptValueFromQLineF(QScriptEngine *engine, const QLineF &line)
{
    return engine->evaluate(QString::fromLatin1("new QLine(%0, %1, %2, %3)")
                            .arg(line.x1()).arg(line.y1())
                            .arg(line.x2()).arg(line.y2()));
}

static void scriptValueToQLineF(const QScriptValue &value, QLineF &line)
{
    QScriptValue::ResolveFlags flags = QScriptValue::ResolvePrototype;
    line = QLineF(value.property(QLatin1String("x1"), flags).toNumber(),
                  value.property(QLatin1String("y1"), flags).toNumber(),
                  value.property(QLatin1String("x2"), flags).toNumber(),
                  value.property(QLatin1String("y2"), flags).toNumber());
}

static QScriptValue scriptValueFromQRect(QScriptEngine *engine, const QRect &rect)
{
    return engine->evaluate(QString::fromLatin1("new QRect(%0, %1, %2, %3)")
                            .arg(rect.x()).arg(rect.y())
                            .arg(rect.width()).arg(rect.height()));
}

static void scriptValueToQRect(const QScriptValue &value, QRect &rect)
{
    rect = QRect(value.property(QLatin1String("x")).toInt32(),
                 value.property(QLatin1String("y")).toInt32(),
                 value.property(QLatin1String("width")).toInt32(),
                 value.property(QLatin1String("height")).toInt32());
}

static QScriptValue scriptValueFromQRectF(QScriptEngine *engine, const QRectF &rect)
{
    return engine->evaluate(QString::fromLatin1("new QRect(%0, %1, %2, %3)")
                            .arg(rect.x()).arg(rect.y())
                            .arg(rect.width()).arg(rect.height()));
}

static void scriptValueToQRectF(const QScriptValue &value, QRectF &rect)
{
    rect = QRectF(value.property(QLatin1String("x")).toNumber(),
                  value.property(QLatin1String("y")).toNumber(),
                  value.property(QLatin1String("width")).toNumber(),
                  value.property(QLatin1String("height")).toNumber());
}

class QtCoreScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList QtCoreScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt");
    list << QLatin1String("qt.core");
    return list;
}

struct QtMetaObject : private QObject
{
public:
    static const QMetaObject *get()
        { return &static_cast<QtMetaObject*>(0)->staticQtMetaObject; }
};

void QtCoreScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt")) {
        QScriptValue qt = engine->newQMetaObject(QtMetaObject::get());
        // a few extra enums we need
        ADD_ENUM_VALUE(qt, Qt, FastTransformation);
        ADD_ENUM_VALUE(qt, Qt, SmoothTransformation);
        engine->globalObject().setProperty("qt", qt);
    } else if (key == QLatin1String("qt.core")) {
        QScriptValue extensionObject = engine->globalObject();

        qScriptRegisterMetaType<QPoint>(engine, scriptValueFromQPoint, scriptValueToQPoint);
        qScriptRegisterMetaType<QPointF>(engine, scriptValueFromQPointF, scriptValueToQPointF);

        qScriptRegisterMetaType<QSize>(engine, scriptValueFromQSize, scriptValueToQSize);
        qScriptRegisterMetaType<QSizeF>(engine, scriptValueFromQSizeF, scriptValueToQSizeF);

        qScriptRegisterMetaType<QLine>(engine, scriptValueFromQLine, scriptValueToQLine);
        qScriptRegisterMetaType<QLineF>(engine, scriptValueFromQLineF, scriptValueToQLineF);

        qScriptRegisterMetaType<QRect>(engine, scriptValueFromQRect, scriptValueToQRect);
        qScriptRegisterMetaType<QRectF>(engine, scriptValueFromQRectF, scriptValueToQRectF);

        extensionObject.setProperty("QByteArray", constructByteArrayClass(engine));
        extensionObject.setProperty("QDir", constructDirClass(engine));
        extensionObject.setProperty("QEvent", constructEventClass(engine));
        extensionObject.setProperty("QIODevice", constructIODeviceClass(engine));
        extensionObject.setProperty("QFile", constructFileClass(engine));
        extensionObject.setProperty("QFileInfo", constructFileInfoClass(engine));
        extensionObject.setProperty("QModelIndex", constructModelIndexClass(engine));
        extensionObject.setProperty("QProcess", constructProcessClass(engine));
        extensionObject.setProperty("QStringRef", constructStringRefClass(engine));
        extensionObject.setProperty("QTextStream", constructTextStreamClass(engine));
        extensionObject.setProperty("QTimeLine", constructTimeLineClass(engine));
        extensionObject.setProperty("QTimer", constructTimerClass(engine));
        extendStringPrototype(engine);
    } else {
        Q_ASSERT_X(false, "initialize", qPrintable(key));
    }
}

Q_EXPORT_STATIC_PLUGIN(QtCoreScriptPlugin)
Q_EXPORT_PLUGIN2(qtscriptcore, QtCoreScriptPlugin)
