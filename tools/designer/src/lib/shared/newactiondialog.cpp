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

bool NewActionDialog::isMenuAction() const
{
    return ui.checkBoxMenuAction->isChecked();
}

void NewActionDialog::on_editActionText_textChanged(const QString &text)
{
    ui.okButton->setEnabled(!text.isEmpty());
}

} // namespace qdesigner_internal
