#ifndef FORMBUILDER_H
#define FORMBUILDER_H

#include "uilib_global.h"

#include <resource.h>
#include <QMap>

class QAction;

class QT_UILIB_EXPORT FormBuilder: public Resource
{
public:
    FormBuilder();

    virtual QWidget *createWidget(DomWidget *ui_widget)
        { return Resource::create(ui_widget, 0); }
    virtual DomWidget *createDom(QWidget *widget)
        { return Resource::createDom(widget, 0); }

protected:
    virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);

    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

private:
    QMap<QString, QAction*> m_actions;
};

#endif // FORMBUILDER_H
