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

#ifndef ABSTRACTWIDGETBOX_H
#define ABSTRACTWIDGETBOX_H

#include <QtGui/QWidget>
#include <QtGui/QIcon>

#include "sdk_global.h"

class DomUI;

class QT_SDK_EXPORT AbstractWidgetBox : public QWidget
{
    Q_OBJECT
public:
    class Widget {
    public:
        Widget(const QString &name = QString(), const QString &xml = QString(),
                const QIcon icon = QIcon())
            : m_name(name), m_xml(xml), m_icon(icon) {}
        QString name() const { return m_name; }
        void setName(const QString &name) { m_name = name; }
        QString domXml() const { return m_xml; }
        void setDomXml(const QString &xml) { m_xml = xml; }
        QIcon icon() const { return m_icon; }
        void setIcon(const QIcon &icon) { m_icon = icon; }

        bool isNull() const { return m_name.isEmpty(); }

    private:
        QString m_name;
        QString m_xml;
        QIcon m_icon;
    };
    typedef QList<Widget> WidgetList;

    class Category {
    public:
        Category(const QString &name = QString()) { m_name = name; }

        QString name() const { return m_name; }
        void setName(const QString &name) { m_name = name; }
        int widgetCount() { return m_widget_list.size(); }
        Widget widget(int idx) const { return m_widget_list.at(idx); }
        void removeWidget(int idx) { m_widget_list.removeAt(idx); }
        void addWidget(const Widget &widget) { m_widget_list.append(widget); }

        bool isNull() const { return m_name.isEmpty(); }

    private:
        QString m_name;
        QList<Widget> m_widget_list;
    };
    typedef QList<Category> CategoryList;

    AbstractWidgetBox(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~AbstractWidgetBox();

    virtual int categoryCount() const = 0;
    virtual Category category(int cat_idx) const = 0;
    virtual void addCategory(const Category &cat) = 0;
    virtual void removeCategory(int cat_idx) = 0;

    virtual int widgetCount(int cat_idx) const = 0;
    virtual Widget widget(int cat_idx, int wgt_idx) const = 0;
    virtual void addWidget(int cat_idx, const Widget &wgt) = 0;
    virtual void removeWidget(int cat_idx, int wgt_idx) = 0;

    int findOrInsertCategory(const QString &categoryName);

public slots:
    virtual void reload();
};

#endif // ABSTRACTWIDGETBOX_H
