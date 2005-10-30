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

#include "newactiondialog_p.h"
#include "actioneditor_p.h"
#include "findicondialog_p.h"
#include <QtDesigner/abstracticoncache.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>

#include <QRegExp>

namespace qdesigner_internal {

NewActionDialog::NewActionDialog(ActionEditor *parent)
    : QDialog(parent, Qt::Sheet),
      m_actionEditor(parent)
{
    ui.setupUi(this);
    ui.editActionText->setFocus();
    m_auto_update_object_name = true;
    updateButtons();
}

QIcon NewActionDialog::actionIcon() const
{
    return ui.iconButton->icon();
}

void NewActionDialog::setActionData(const QString &text, const QString &name, const QIcon &icon)
{
    ui.editActionText->setText(text);
    ui.editObjectName->setText(name);
    ui.iconButton->setIcon(icon);
    m_auto_update_object_name = false;
    updateButtons();
}

NewActionDialog::~NewActionDialog()
{
}

void NewActionDialog::accept()
{
    QDialog::accept();
}

QString NewActionDialog::actionText() const
{
    return ui.editActionText->text();
}

QString NewActionDialog::actionName() const
{
    return ui.editObjectName->text();
}

void NewActionDialog::on_editActionText_textEdited(const QString &text)
{
    if (text.isEmpty())
        m_auto_update_object_name = true;

    if (m_auto_update_object_name)
        ui.editObjectName->setText(ActionEditor::actionTextToName(text));

    updateButtons();
}

void NewActionDialog::on_editObjectName_textEdited(const QString&)
{
    updateButtons();
    m_auto_update_object_name = false;
}

void NewActionDialog::on_iconButton_clicked()
{
    QDesignerFormWindowInterface *form = m_actionEditor->formWindow();
    QDesignerFormEditorInterface *core = form->core();

    QString file_path = form->absoluteDir().absolutePath();
    QString qrc_path;
    if (!actionIcon().isNull()) {
        file_path = core->iconCache()->iconToFilePath(actionIcon());
        qrc_path = core->iconCache()->iconToQrcPath(actionIcon());
    }

    FindIconDialog dialog(form, this);
    dialog.setPaths(qrc_path, file_path);

    if (!dialog.exec())
        return;

    file_path = dialog.filePath();
    qrc_path = dialog.qrcPath();

    if (file_path.isEmpty())
        return;

    QIcon icon = core->iconCache()->nameToIcon(file_path, qrc_path);
    ui.iconButton->setIcon(icon);
    updateButtons();
}

void NewActionDialog::on_removeIconButton_clicked()
{
    ui.iconButton->setIcon(QIcon());
    updateButtons();
}

void NewActionDialog::updateButtons()
{
    ui.okButton->setEnabled(!actionText().isEmpty() && !actionName().isEmpty());
    ui.removeIconButton->setEnabled(!ui.iconButton->icon().isNull());
}

} // namespace qdesigner_internal
