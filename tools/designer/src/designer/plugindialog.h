/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "ui_plugindialog.h"

class QDesignerFormEditorInterface;

class PluginDialog : public QDialog
{
    Q_OBJECT
public:
    PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);

    QDesignerFormEditorInterface *core() const;

private:
    void populateTreeWidget();
    QIcon pluginIcon(const QIcon &icon);
    QTreeWidgetItem* setTopLevelItem(const QString &itemName);
    QTreeWidgetItem* setPluginItem(QTreeWidgetItem *topLevelItem, 
                                   const QString &itemName, const QFont &font);
    void setItem(QTreeWidgetItem *pluginItem, const QString &name, 
                 const QString &toolTip, const QString &whatsThis, const QIcon &icon);

private:
    QDesignerFormEditorInterface *m_core;
    Ui::PluginDialog ui;
    QIcon interfaceIcon;
    QIcon featureIcon;
};

#endif
