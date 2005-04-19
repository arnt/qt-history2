/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionFactory>

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/qplugin.h>
#include <QtCore/qdebug.h>

#include <QtGui/QIcon>

#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3Header>

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties) // ### remove me
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

class Q3ListViewExtraInfo: public QObject, public QDesignerExtraInfoExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerExtraInfoExtension)
public:
    Q3ListViewExtraInfo(Q3ListView *widget, QDesignerFormEditorInterface *core, QObject *parent)
        : QObject(parent), m_widget(widget), m_core(core) {}

    virtual QWidget *widget() const
    { return m_widget; }

    virtual QDesignerFormEditorInterface *core() const
    { return m_core; }

    virtual bool saveUiExtraInfo(DomUi *ui)
    { Q_UNUSED(ui); return false; }

    virtual bool loadUiExtraInfo(DomUi *ui)
    { Q_UNUSED(ui); return false; }

    virtual bool saveWidgetExtraInfo(DomWidget *ui_widget);
    virtual bool loadWidgetExtraInfo(DomWidget *ui_widget);

    void initializeQ3ListViewItems(const QList<DomItem *> &items, Q3ListViewItem *parentItem = 0);

private:
    QPointer<Q3ListView> m_widget;
    QPointer<QDesignerFormEditorInterface> m_core;
};

class Q3ListViewExtraInfoFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    inline Q3ListViewExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent = 0)
        : QExtensionFactory(parent), m_core(core) {}

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const
    {
        if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
            return 0;

        if (Q3ListView *w = qobject_cast<Q3ListView*>(object))
            return new Q3ListViewExtraInfo(w, m_core, parent);

        return 0;
    }

private:
    QDesignerFormEditorInterface *m_core;
};

class Q3ListViewPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    inline Q3ListViewPlugin(QObject *parent = 0)
        : QObject(parent), m_initialized(false) {}

    virtual QString name() const
    { return QLatin1String("Q3ListView"); }

    virtual QString group() const
    { return QLatin1String("Compat"); }

    virtual QString toolTip() const
    { return QString(); }

    virtual QString whatsThis() const
    { return QString(); }

    virtual QString includeFile() const
    { return QLatin1String("q3listview.h"); }

    virtual QIcon icon() const
    { return QIcon(); }

    virtual bool isContainer() const
    { return false; }

    virtual bool isForm() const
    { return false; }

    virtual QWidget *createWidget(QWidget *parent)
    { return new Q3ListView(parent); }

    virtual bool isInitialized() const
    { return m_initialized; }

    virtual void initialize(QDesignerFormEditorInterface *core)
    {
        Q_UNUSED(core);

        if (m_initialized)
            return;

        QExtensionManager *mgr = core->extensionManager();
        Q_ASSERT(mgr != 0);

        mgr->registerExtensions(new Q3ListViewExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

        m_initialized = true;
    }

    virtual QString codeTemplate() const
    { return QString(); }

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3ListView\" name=\"listView\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>100</width>\
                    <height>80</height>\
                </rect>\
            </property>\
        </widget>\
      "); }

private:
    bool m_initialized;
};

bool Q3ListViewExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);
    return true;
}

bool Q3ListViewExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q3ListView *listView = qobject_cast<Q3ListView*>(widget());
    Q_ASSERT(listView != 0);

    QList<DomColumn*> columns = ui_widget->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        DomProperty *clickable = properties.value(QLatin1String("clickable"));
        DomProperty *resizable = properties.value(QLatin1String("resizable"));

        QString txt = text->elementString()->text();
        listView->addColumn(txt);

        if (pixmap) {
            DomResourcePixmap *pix = pixmap->elementIconSet();
            QIcon icon(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
            listView->header()->setLabel(listView->header()->count() - 1, icon, txt);
        }

        if (!clickable) {
            listView->header()->setClickEnabled(false, listView->header()->count() - 1);
        }

        if (!resizable) {
            listView->header()->setResizeEnabled(false, listView->header()->count() - 1);
        }
    }

    if (ui_widget->elementItem().size()) {
        initializeQ3ListViewItems(ui_widget->elementItem());
    }

    return true;
}

void Q3ListViewExtraInfo::initializeQ3ListViewItems(const QList<DomItem *> &items, Q3ListViewItem *parentItem)
{
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        Q3ListViewItem *__item = 0;
        if (parentItem != 0)
            __item = new Q3ListViewItem(parentItem);
        else
            __item = new Q3ListViewItem(static_cast<Q3ListView*>(widget()));

        int textCount = 0, pixCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                __item->setText(textCount++, p->elementString()->text());

            if (p->attributeName() == QLatin1String("pixmap")) {
                DomResourcePixmap *pix = p->elementPixmap();
                QPixmap pixmap(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
                __item->setPixmap(pixCount++, pixmap);
            }
        }

        if (item->elementItem().size()) {
            __item->setOpen(true);
            initializeQ3ListViewItems(item->elementItem(), __item);
        }
    }
}

Q_EXPORT_PLUGIN(Q3ListViewPlugin)

#include "plugin.moc"
