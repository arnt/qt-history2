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

#include "qdesigner_settingsdialog.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"

#include "ui_designer_preferences.h"

#include <QtCore/qdebug.h>

QDesignerSettingsDialog::QDesignerSettingsDialog(QDesignerWorkbench *workbench, QWidget *parent)
    : AbstractSettingsDialog(parent),
      m_workbench(workbench)
{
    Q_ASSERT(workbench != 0);

    setAttribute(Qt::WA_DeleteOnClose, true);

    QWidget *page = new QWidget();
    Ui::DesignerPreferences ui;
    ui.setupUi(page);

    addPage(page, QIcon(":/trolltech/designer/images/workbench.png"));

    // ### use the Designer Resource
    ui.lblMDI->setPixmap(QPixmap(":/trolltech/designer/images/mdi.png"));
    ui.lblSDI->setPixmap(QPixmap(":/trolltech/designer/images/sdi.png"));

    QDesignerSettings settings;

    int mode = settings.uiMode();
    if (mode == QDesignerWorkbench::TopLevelMode)
        ui.optSDI->setChecked(true);
    else
        ui.optMDI->setChecked(true);

    ui.optShowDialog->setChecked(settings.showNewFormOnStartup());

    QMetaObject::connectSlotsByName(this);
}

QDesignerSettingsDialog::~QDesignerSettingsDialog()
{
}

void QDesignerSettingsDialog::on_optMDI_toggled(bool b)
{
    if (b == false) {
        // nothing to do
        return;
    }

    QDesignerSettings settings;
    settings.setUIMode(QDesignerWorkbench::WorkspaceMode);

    m_workbench->switchToWorkspaceMode();
}

void QDesignerSettingsDialog::on_optSDI_toggled(bool b)
{
    if (b == false) {
        // nothing to do
        return;
    }

    QDesignerSettings settings;
    settings.setUIMode(QDesignerWorkbench::TopLevelMode);

    m_workbench->switchToTopLevelMode();
}

void QDesignerSettingsDialog::on_optShowDialog_toggled(bool b)
{
    QDesignerSettings settings;
    settings.setShowNewFormOnStartup(b);
}


