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

#include "saveformastemplate.h"
#include "qdesigner_settings.h"

#include <QtCore/QFile>
#include <QtGui/QMessageBox>

#include <abstractformwindow.h>

SaveFormAsTemplate::SaveFormAsTemplate(AbstractFormWindow *formWindow, QWidget *parent)
    : QDialog(parent),
      m_formWindow(formWindow)
{
    ui.setupUi(this);

    ui.templateNameEdit->setText(formWindow->mainContainer()->objectName());
    ui.templateNameEdit->selectAll();

    ui.templateNameEdit->setFocus();

    QDesignerSettings settings;
    QStringList paths = settings.formTemplatePaths();
    ui.categoryCombo->insertItems(0, paths);
    connect(ui.templateNameEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(updateOKButton(const QString &)));
}

SaveFormAsTemplate::~SaveFormAsTemplate()
{
}

void SaveFormAsTemplate::on_okButton_clicked()
{
    QString templateFileName = ui.categoryCombo->currentText() + QLatin1Char('/')
                                    + ui.templateNameEdit->text()
                                    + QLatin1String("_template.ui");
    QFile file(templateFileName);
    if (file.exists()) {
        if (QMessageBox::information(m_formWindow, tr("Template Exists"),
                                 tr("A template with the name %1 already exits\n"
                                    "Do you want overwrite the template?").arg(ui.templateNameEdit->text()),
                                 tr("Overwrite Template"), tr("Cancel"), QString(), 1, 1) == 1) {
            return;
        }
    }

    while (!file.open(QFile::WriteOnly)) {
        if (QMessageBox::information(m_formWindow, tr("Open Error"),
            tr("There was an error opening template %1 for writing. Reason: %2").arg(ui.templateNameEdit->text()).arg(file.errorString()),
            tr("Try again"), tr("Cancel"), QString(), 0, 1) == 1) {
            return;
        }
    }

    QByteArray ba = m_formWindow->contents().toUtf8();
    while (file.write(ba) != ba.size()) {
        if (QMessageBox::information(m_formWindow, tr("Write Error"),
            tr("There was an error writing the template %1 to disk. Reason: %2").arg(ui.templateNameEdit->text()).arg(file.errorString()),
            tr("Try again"), tr("Cancel"), QString(), 0, 1) == 1) {
                file.close();
                file.remove();
                return;
            }
            file.reset();
    }
    accept();
}

void SaveFormAsTemplate::on_cancelButton_clicked()
{
    reject();
}

void SaveFormAsTemplate::updateOKButton(const QString &str)
{
    ui.okButton->setEnabled(!str.isEmpty());
}
