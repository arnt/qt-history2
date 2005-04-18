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

#include <QtDesigner/container.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/ui4.h>

#include <QtCore/qplugin.h>
#include <QtCore/QObject>

#include <QtGui/QIcon>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3Header>

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties)
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}


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

    virtual bool saveExtraInfo(QWidget *widget, DomWidget *ui_widget);
    virtual bool loadExtraInfo(QWidget *widget, DomWidget *ui_widget);

private:
    bool m_initialized;
};

bool Q3ListViewPlugin::saveExtraInfo(QWidget *widget, DomWidget *ui_widget)
{
    return true;
}

bool Q3ListViewPlugin::loadExtraInfo(QWidget *widget, DomWidget *ui_widget)
{
    Q3ListView *listView = qobject_cast<Q3ListView*>(widget);
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
            QIcon icon; // ###
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
        // ### initializeQ3ListViewItems(className, varName, w->elementItem());
    }

    return true;
}


Q_EXPORT_PLUGIN(Q3ListViewPlugin)

#include "plugin.moc"
