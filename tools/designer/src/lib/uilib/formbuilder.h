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

#ifndef FORMBUILDER_H
#define FORMBUILDER_H

#include "uilib_global.h"

#include <resource.h>
#include <QtCore/QMap>

class QAction;
class QListWidget;
class QComboBox;

class QT_UILIB_EXPORT FormBuilder: public Resource
{
public:
    FormBuilder();

protected:
    virtual QWidget *create(DomUI *ui, QWidget *parentWidget);
    virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
    virtual QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget);
    virtual QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget);
    virtual QAction *create(DomAction *ui_action, QObject *parent);
    virtual QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent);

    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);

    virtual void createConnections(DomConnections *connections, QWidget *widget);

    virtual void saveExtraInfo(QWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
    virtual void loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

    static QWidget *widgetByName(QWidget *topLevel, const QString &name);

    void loadListWidgetExtraInfo(DomWidget *ui_widget, QListWidget *listWidget, QWidget *parentWidget);
    void loadComboBoxExtraInfo(DomWidget *ui_widget, QComboBox *comboBox, QWidget *parentWidget);

    void saveListWidgetExtraInfo(QListWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);
    void saveComboBoxExtraInfo(QComboBox *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget);

private:
    QMap<QString, QAction*> m_actions;
};

#endif // FORMBUILDER_H
