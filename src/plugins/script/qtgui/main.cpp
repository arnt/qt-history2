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
        QScriptValue extensionObject = setupPackage("qt.gui", engine);
        extensionObject.setProperty("Brush", constructBrushClass(engine));
        extensionObject.setProperty("Color", constructColorClass(engine));
        extensionObject.setProperty("ConicalGradient", constructConicalGradientClass(engine));
        extensionObject.setProperty("Font", constructFontClass(engine));
        extensionObject.setProperty("Gradient", constructGradientClass(engine));
        extensionObject.setProperty("Icon", constructIconClass(engine));
        extensionObject.setProperty("Image", constructImageClass(engine));
        extensionObject.setProperty("LinearGradient", constructLinearGradientClass(engine));
        extensionObject.setProperty("KeyEvent", constructKeyEventClass(engine));
        extensionObject.setProperty("Matrix", constructMatrixClass(engine));
        extensionObject.setProperty("MouseEvent", constructMouseEventClass(engine));
        extensionObject.setProperty("Pen", constructPenClass(engine));
        extensionObject.setProperty("Pixmap", constructPixmapClass(engine));
        extensionObject.setProperty("Polygon", constructPolygonClass(engine));
        extensionObject.setProperty("Painter", constructPainterClass(engine));
        extensionObject.setProperty("PainterPath", constructPainterPathClass(engine));
        extensionObject.setProperty("Palette", constructPaletteClass(engine));
        extensionObject.setProperty("RadialGradient", constructRadialGradientClass(engine));
        extensionObject.setProperty("Region", constructRegionClass(engine));
        extensionObject.setProperty("Transform", constructTransformClass(engine));

        extensionObject.setProperty("GraphicsItem", constructGraphicsItemClass(engine));
        extensionObject.setProperty("GraphicsItemAnimation", constructGraphicsItemAnimationClass(engine));
        extensionObject.setProperty("GraphicsItemGroup", constructGraphicsItemGroupClass(engine));
        extensionObject.setProperty("AbstractGraphicsShapeItem", constructAbstractGraphicsShapeItemClass(engine));
        extensionObject.setProperty("GraphicsEllipseItem", constructGraphicsEllipseItemClass(engine));
        extensionObject.setProperty("GraphicsLineItem", constructGraphicsLineItemClass(engine));
        extensionObject.setProperty("GraphicsPathItem", constructGraphicsPathItemClass(engine));
        extensionObject.setProperty("GraphicsPixmapItem", constructGraphicsPixmapItemClass(engine));
        extensionObject.setProperty("GraphicsPolygonItem", constructGraphicsPolygonItemClass(engine));
        extensionObject.setProperty("GraphicsRectItem", constructGraphicsRectItemClass(engine));
        extensionObject.setProperty("GraphicsSimpleTextItem", constructGraphicsSimpleTextItemClass(engine));
        extensionObject.setProperty("GraphicsTextItem", constructGraphicsTextItemClass(engine));
        extensionObject.setProperty("GraphicsScene", constructGraphicsSceneClass(engine));
        extensionObject.setProperty("GraphicsView", constructGraphicsViewClass(engine));
    } else {
        Q_ASSERT_X(false, "initialize", qPrintable(key));
    }
}

Q_EXPORT_STATIC_PLUGIN(QtGuiScriptPlugin)
Q_EXPORT_PLUGIN2(qtscriptgui, QtGuiScriptPlugin)
