/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTWIDGETBOX_H
#define ABSTRACTWIDGETBOX_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QMetaType>
#include <QtGui/QWidget>
#include <QtGui/QIcon>

QT_BEGIN_HEADER

class DomUI;
class QDesignerDnDItemInterface;

class QDESIGNER_SDK_EXPORT QDesignerWidgetBoxInterface : public QWidget
{
    Q_OBJECT
public:
    class Widget {
    public:
        enum Type { Default, Custom };
        Widget(const QString &aname = QString(), const QString &xml = QString(),
                const QString &icon_name = QString(), Type atype = Default)
            : m_name(aname), m_xml(xml), m_icon_name(icon_name), m_type(atype) {}
        QString name() const { return m_name; }
        void setName(const QString &aname) { m_name = aname; }
        QString domXml() const { return m_xml; }
        void setDomXml(const QString &xml) { m_xml = xml; }
        QString iconName() const { return m_icon_name; }
        void setIconName(const QString &icon_name) { m_icon_name = icon_name; }
        Type type() const { return m_type; }
        void setType(Type atype) { m_type = atype; }

        bool isNull() const { return m_name.isEmpty(); }

    private:
        QString m_name;
        QString m_xml;
        QString m_icon_name;
        Type m_type;
    };
    typedef QList<Widget> WidgetList;

    class Category {
    public:
        enum Type { Default, Scratchpad };

        Category(const QString &aname = QString(), Type atype = Default)
            : m_name(aname), m_type(atype) {}

        QString name() const { return m_name; }
        void setName(const QString &aname) { m_name = aname; }
        int widgetCount() const { return m_widget_list.size(); }
        Widget widget(int idx) const { return m_widget_list.at(idx); }
        void removeWidget(int idx) { m_widget_list.removeAt(idx); }
        void addWidget(const Widget &awidget) { m_widget_list.append(awidget); }
        Type type() const { return m_type; }
        void setType(Type atype) { m_type = atype; }

        bool isNull() const { return m_name.isEmpty(); }

    private:
        QString m_name;
        Type m_type;
        QList<Widget> m_widget_list;
    };
    typedef QList<Category> CategoryList;

    QDesignerWidgetBoxInterface(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~QDesignerWidgetBoxInterface();

    virtual int categoryCount() const = 0;
    virtual Category category(int cat_idx) const = 0;
    virtual void addCategory(const Category &cat) = 0;
    virtual void removeCategory(int cat_idx) = 0;

    virtual int widgetCount(int cat_idx) const = 0;
    virtual Widget widget(int cat_idx, int wgt_idx) const = 0;
    virtual void addWidget(int cat_idx, const Widget &wgt) = 0;
    virtual void removeWidget(int cat_idx, int wgt_idx) = 0;

    int findOrInsertCategory(const QString &categoryName);

    virtual void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list,
                                const QPoint &global_mouse_pos) = 0;

    virtual void setFileName(const QString &file_name) = 0;
    virtual QString fileName() const = 0;
    virtual bool load() = 0;
    virtual bool save() = 0;
};

Q_DECLARE_METATYPE(QDesignerWidgetBoxInterface::Widget)

QT_END_HEADER

#endif // ABSTRACTWIDGETBOX_H
