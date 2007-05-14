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
#include <QtGui/QApplication>
#include <qdebug.h>

QScriptValue constructBrushClass(QScriptEngine *engine);
QScriptValue constructColorClass(QScriptEngine *engine);
QScriptValue constructFontClass(QScriptEngine *engine);
QScriptValue constructGradientClass(QScriptEngine *engine);
QScriptValue constructConicalGradientClass(QScriptEngine *engine);
QScriptValue constructLinearGradientClass(QScriptEngine *engine);
QScriptValue constructRadialGradientClass(QScriptEngine *engine);
QScriptValue constructIconClass(QScriptEngine *engine);
QScriptValue constructImageClass(QScriptEngine *engine);
QScriptValue constructKeyEventClass(QScriptEngine *engine);
QScriptValue constructMatrixClass(QScriptEngine *engine);
QScriptValue constructMouseEventClass(QScriptEngine *engine);
QScriptValue constructPenClass(QScriptEngine *engine);
QScriptValue constructPixmapClass(QScriptEngine *engine);
QScriptValue constructPolygonClass(QScriptEngine *engine);
QScriptValue constructPainterClass(QScriptEngine *engine);
QScriptValue constructPainterPathClass(QScriptEngine *engine);
QScriptValue constructPaletteClass(QScriptEngine *engine);
QScriptValue constructRegionClass(QScriptEngine *engine);
QScriptValue constructTransformClass(QScriptEngine *engine);

QScriptValue constructGraphicsItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemAnimationClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemGroupClass(QScriptEngine *engine);
QScriptValue constructAbstractGraphicsShapeItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsEllipseItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsLineItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsPathItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsPolygonItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsPixmapItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsRectItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsSimpleTextItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsTextItemClass(QScriptEngine *engine);
QScriptValue constructGraphicsSceneClass(QScriptEngine *engine);
QScriptValue constructGraphicsViewClass(QScriptEngine *engine);

class QtGuiScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList QtGuiScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt.gui");
    return list;
}

void QtGuiScriptPlugin::initialize(const QString &key,
                                   QScriptEngine *engine)
{
    if (key == QLatin1String("qt.gui")) {
        engine->importExtension("qt.core"); // dependency
        QScriptValue extensionObject = engine->globalObject();
        extensionObject.setProperty("QBrush", constructBrushClass(engine));
        extensionObject.setProperty("QColor", constructColorClass(engine));
        extensionObject.setProperty("QConicalGradient", constructConicalGradientClass(engine));
        extensionObject.setProperty("QFont", constructFontClass(engine));
        extensionObject.setProperty("QGradient", constructGradientClass(engine));
        extensionObject.setProperty("QIcon", constructIconClass(engine));
        extensionObject.setProperty("QImage", constructImageClass(engine));
        extensionObject.setProperty("QLinearGradient", constructLinearGradientClass(engine));
        extensionObject.setProperty("QKeyEvent", constructKeyEventClass(engine));
        extensionObject.setProperty("QMatrix", constructMatrixClass(engine));
        extensionObject.setProperty("QPen", constructPenClass(engine));
        extensionObject.setProperty("QPolygon", constructPolygonClass(engine));
        extensionObject.setProperty("QPainter", constructPainterClass(engine));
        extensionObject.setProperty("QPainterPath", constructPainterPathClass(engine));
        extensionObject.setProperty("QPalette", constructPaletteClass(engine));
        extensionObject.setProperty("QRadialGradient", constructRadialGradientClass(engine));
        extensionObject.setProperty("QRegion", constructRegionClass(engine));
        extensionObject.setProperty("QTransform", constructTransformClass(engine));

        extensionObject.setProperty("QGraphicsItem", constructGraphicsItemClass(engine));
        extensionObject.setProperty("QGraphicsItemAnimation", constructGraphicsItemAnimationClass(engine));
        extensionObject.setProperty("QGraphicsItemGroup", constructGraphicsItemGroupClass(engine));
        extensionObject.setProperty("QAbstractGraphicsShapeItem", constructAbstractGraphicsShapeItemClass(engine));
        extensionObject.setProperty("QGraphicsEllipseItem", constructGraphicsEllipseItemClass(engine));
        extensionObject.setProperty("QGraphicsLineItem", constructGraphicsLineItemClass(engine));
        extensionObject.setProperty("QGraphicsPathItem", constructGraphicsPathItemClass(engine));
        extensionObject.setProperty("QGraphicsPolygonItem", constructGraphicsPolygonItemClass(engine));
        extensionObject.setProperty("QGraphicsRectItem", constructGraphicsRectItemClass(engine));
        extensionObject.setProperty("QGraphicsSimpleTextItem", constructGraphicsSimpleTextItemClass(engine));
        extensionObject.setProperty("QGraphicsTextItem", constructGraphicsTextItemClass(engine));
        extensionObject.setProperty("QGraphicsScene", constructGraphicsSceneClass(engine));

        if (QApplication::type() != QApplication::Tty) {
            extensionObject.setProperty("QMouseEvent", constructMouseEventClass(engine));
            extensionObject.setProperty("QPixmap", constructPixmapClass(engine));
            extensionObject.setProperty("QGraphicsPixmapItem", constructGraphicsPixmapItemClass(engine));
            extensionObject.setProperty("QGraphicsView", constructGraphicsViewClass(engine));
        }
    } else {
        Q_ASSERT_X(false, "initialize", qPrintable(key));
    }
}

Q_EXPORT_STATIC_PLUGIN(QtGuiScriptPlugin)
Q_EXPORT_PLUGIN2(qtscriptgui, QtGuiScriptPlugin)
