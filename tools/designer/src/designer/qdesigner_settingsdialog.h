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

#ifndef QDESIGNER_SETTINGSDIALOG_H
#define QDESIGNER_SETTINGSDIALOG_H

#include <abstractsettingsdialog.h>

#include <QtCore/QPointer>

class QDesignerWorkbench;

class QDesignerSettingsDialog: public AbstractSettingsDialog
{
    Q_OBJECT
public:
    QDesignerSettingsDialog(QDesignerWorkbench *workbench, QWidget *parent);
    virtual ~QDesignerSettingsDialog();

private slots:
    void on_mdiButton_toggled(bool b);
    void on_sdiButton_toggled(bool b);
    void on_optShowDialog_toggled(bool b);

private:
    QPointer<QDesignerWorkbench> m_workbench;
};

#endif // QDESIGNER_SETTINGSDIALOG_H
