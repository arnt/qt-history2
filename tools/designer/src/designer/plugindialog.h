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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QIcon>

class QDesignerFormEditorInterface;

class QLabel;
class QPushButton;
class QStringList;
class QTreeWidget;
class QTreeWidgetItem;

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
    PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);

    QDesignerFormEditorInterface *core() const;

private:
    void populateTreeWidget();
    QIcon pluginIcon(const QIcon &icon);

private:
    QDesignerFormEditorInterface *m_core;
    QLabel *label;
    QTreeWidget *treeWidget;
    QPushButton *okButton;
    QIcon interfaceIcon;
    QIcon featureIcon;
};

#endif
