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
    m_auto_update_object_name = true;
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

static QString actionTextToName(const QString &text)
{
    QString name = text;
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

    if (text.isEmpty())
        m_auto_update_object_name = true;

    if (m_auto_update_object_name)
        ui.editObjectName->setText(actionTextToName(text));
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
