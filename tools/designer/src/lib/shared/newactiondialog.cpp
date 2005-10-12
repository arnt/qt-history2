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
    : QDialog(parent),
      m_actionEditor(parent)
{
    ui.setupUi(this);
    ui.editActionText->setFocus();
    ui.okButton->setEnabled(false);
}

QIcon NewActionDialog::actionIcon() const
{
    return ui.iconButton->icon();
}

void NewActionDialog::setActionData(const QString &text, const QIcon &icon)
{
    ui.editActionText->setText(text);
    ui.iconButton->setIcon(icon);
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
    QString name = actionText();
    if (name.isEmpty())
        return QString();

    name[0] = name.at(0).toUpper();
    name.prepend(QLatin1String("action"));
    name.replace(QRegExp(QString("[^a-zA-Z_0-9]")), QString("_"));
    name.replace(QRegExp("__*"), QString("_"));
    if (name.endsWith("_"))
        name.truncate(name.size() - 1);

    return name;
}

void NewActionDialog::on_editActionText_textChanged(const QString &text)
{
    ui.okButton->setEnabled(!text.isEmpty());
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
}

void NewActionDialog::on_removeIconButton_clicked()
{
    ui.iconButton->setIcon(QIcon());
}

} // namespace qdesigner_internal
