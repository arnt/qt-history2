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

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QList>

class PreferenceInterface;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;
class AbstractFormEditor;
class QPushButton;

class PreferenceDialog : public QDialog
{
    Q_OBJECT
public:
    PreferenceDialog(AbstractFormEditor *core, QWidget *parent);
    ~PreferenceDialog();

signals:
    void preferencesChanged();

private slots:
    void accept();
    void reject();
    void changePane(QTreeWidgetItem *);
    void preferenceChanged();

private:
    QList<PreferenceInterface *> m_preferences;
    QStackedWidget *m_stack;
    QTreeWidget *m_treeWidget;
    AbstractFormEditor *m_core;
    QPushButton *m_ok_button;
};
#endif
