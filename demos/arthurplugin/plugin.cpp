/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <QtGui/QLineEdit>

#include "xform.h"
#include "pathdeform.h"
#include "gradients.h"
#include "pathstroke.h"
#include "hoverpoints.h"
#include "composition.h"

class QDesignerFormEditorInterface;

class PathDeformRendererEx : public PathDeformRenderer
{
    Q_OBJECT
public:
    PathDeformRendererEx(QWidget *parent) : PathDeformRenderer(parent) { }
    QSize sizeHint() const { return QSize(300, 200); }
};

class DemoPlugin : public QDesignerCustomWidgetInterface
{
    Q_INTERFACES(QDesignerCustomWidgetInterface)
        public:
    DemoPlugin() : m_initialized(false) { }
    bool isContainer() const { return false; }
    bool isInitialized() const { return m_initialized; }
    QIcon icon() const { return QIcon(); }
    QString codeTemplate() const { return QString(); }
    QString whatsThis() const { return QString(); }
    QString toolTip() const { return QString(); }
    QString group() const { return "Arthur Widgets [Demo]"; }
    void initialize(QDesignerFormEditorInterface *)
    {
        if (m_initialized)
            return;
        m_initialized = true;
    }
private:
    bool m_initialized;
};

class DeformPlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    DeformPlugin(QObject *parent = 0) : QObject(parent) { }

    QString includeFile() const { return "deform.h"; }
    QString name() const { return "PathDeformRendererEx"; }

    QWidget *createWidget(QWidget *parent)
    {
        PathDeformRenderer *deform = new PathDeformRendererEx(parent);
        deform->setRadius(70);
        deform->setAnimated(false);
        deform->setFontSize(20);
        deform->setText("Arthur Widgets Demo");

        return deform;
    }

};

class XFormRendererEx : public XFormView
{
    Q_OBJECT
public:
    XFormRendererEx(QWidget *parent) : XFormView(parent) { textEditor = new QLineEdit; }
    QSize sizeHint() const { return QSize(300, 200); }

public slots:
void setText(const QString &text) { textEditor->setText(text); }
};

class XFormPlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    XFormPlugin(QObject *parent = 0) : QObject(parent) { }
    QString includeFile() const { return "xform.h"; }
    QString name() const { return "XFormRendererEx"; }

    QWidget *createWidget(QWidget *parent)
    {
        XFormRendererEx *xform = new XFormRendererEx(parent);
        xform->setText("Qt - Hello World!!");
        return xform;
    }
};


class GradientEditorPlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    GradientEditorPlugin(QObject *parent = 0) : QObject(parent) { }
    QString includeFile() const { return "gradients.h"; }
    QString name() const { return "GradientEditor"; }

    QWidget *createWidget(QWidget *parent)
    {
        GradientEditor *editor = new GradientEditor(parent);
        return editor;
    }
};

class GradientRendererEx : public GradientRenderer
{
    Q_OBJECT
public:
    GradientRendererEx(QWidget *p) : GradientRenderer(p) { }
    QSize sizeHint() const { return QSize(300, 200); }
};

class GradientRendererPlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    GradientRendererPlugin(QObject *parent = 0) : QObject(parent) { }
    QString includeFile() const { return "gradients.h"; }
    QString name() const { return "GradientRendererEx"; }

    QWidget *createWidget(QWidget *parent)
    {
        GradientRenderer *renderer = new GradientRendererEx(parent);
        renderer->setConicalGradient();
        return renderer;
    }
};

class PathStrokeRendererEx : public PathStrokeRenderer
{
    Q_OBJECT
public:
    PathStrokeRendererEx(QWidget *p) : PathStrokeRenderer(p) { }
    QSize sizeHint() const { return QSize(300, 200); }
};

class StrokeRenderPlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    StrokeRenderPlugin(QObject *parent = 0) : QObject(parent) { }
    QString includeFile() const { return "pathstroke.h"; }
    QString name() const { return "PathStrokeRendererEx"; }

    QWidget *createWidget(QWidget *parent)
    {
        PathStrokeRenderer *stroke = new PathStrokeRendererEx(parent);
        return stroke;
    }
};


class CompositionModePlugin : public QObject, public DemoPlugin
{
    Q_OBJECT
public:
    CompositionModePlugin(QObject *parent = 0) : QObject(parent) { }
    QString includeFile() const { return "composition.h"; }
    QString name() const { return "CompositionRenderer"; }

    QWidget *createWidget(QWidget *parent)
    {
        CompositionRenderer *renderer = new CompositionRenderer(parent);
        renderer->setAnimationEnabled(false);
        return renderer;
    }
};


class ArthurPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
        public:
    QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
        QList<QDesignerCustomWidgetInterface *> plugins;
        plugins
            << new DeformPlugin
            << new XFormPlugin
            << new GradientEditorPlugin
            << new GradientRendererPlugin
            << new StrokeRenderPlugin
            << new CompositionModePlugin;
        return plugins;
    }
};


#include "plugin.moc"

Q_EXPORT_PLUGIN(ArthurPlugins)
