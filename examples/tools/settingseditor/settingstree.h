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

#ifndef SETTINGSTREE_H
#define SETTINGSTREE_H

#include <QIcon>
#include <QTimer>
#include <QTreeWidget>

class QSettings;

class SettingsTree : public QTreeWidget
{
    Q_OBJECT

public:
    SettingsTree(QWidget *parent = 0);

    void setSettingsObject(QSettings *settings);
    QSize sizeHint() const;

public slots:
    void setAutoRefresh(bool autoRefresh);
    void setFallbacksEnabled(bool enabled);
    void maybeRefresh();
    void refresh();

protected:
    bool event(QEvent *event);

private slots:
    void updateSetting(QTreeWidgetItem *item);

private:
    void updateChildItems(QTreeWidgetItem *parent);
    QTreeWidgetItem *createItem(const QString &text, QTreeWidgetItem *parent,
                                int index);
    QTreeWidgetItem *childAt(QTreeWidgetItem *parent, int index);
    int childCount(QTreeWidgetItem *parent);
    int findChild(QTreeWidgetItem *parent, const QString &text, int startIndex);
    void moveItemForward(QTreeWidgetItem *parent, int oldIndex, int newIndex);

    QSettings *settings;
    QTimer refreshTimer;
    bool autoRefresh;
    QIcon groupIcon;
    QIcon keyIcon;
};

#endif
