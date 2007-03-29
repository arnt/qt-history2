/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "saveformastemplate.h"
#include "qdesigner_settings.h"
#include "preferencesdialog.h"

#include <QtCore/QFile>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include <QtDesigner/abstractformwindow.h>

SaveFormAsTemplate::SaveFormAsTemplate(QDesignerFormWindowInterface *formWindow, QWidget *parent)
    : QDialog(parent, Qt::Sheet),
      m_formWindow(formWindow)
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui.templateNameEdit->setText(formWindow->mainContainer()->objectName());
    ui.templateNameEdit->selectAll();

    ui.templateNameEdit->setFocus();

    QDesignerSettings settings;
    QStringList paths = settings.formTemplatePaths();
    ui.categoryCombo->addItems(paths);
    ui.categoryCombo->addItem(tr("Add path..."));
    m_addPathIndex = ui.categoryCombo->count() - 1;
    connect(ui.templateNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateOKButton(QString)));
    connect(ui.categoryCombo, SIGNAL(activated(int)), this, SLOT(checkToAddPath(int)));
}

SaveFormAsTemplate::~SaveFormAsTemplate()
{
}

void SaveFormAsTemplate::accept()
{
    QString templateFileName = ui.categoryCombo->currentText();
    templateFileName += QLatin1Char('/');
    const QString name = ui.templateNameEdit->text();
    templateFileName +=  name;
    const QString extension = QLatin1String(".ui");
    if (!templateFileName.endsWith(extension))
        templateFileName.append(extension);
    QFile file(templateFileName);

    if (file.exists()) {
        QMessageBox msgBox(QMessageBox::Information, tr("Template Exists"),
                        tr("A template with the name %1 already exists.\n"
                           "Do you want overwrite the template?").arg(name), QMessageBox::Cancel, m_formWindow);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite Template"), QMessageBox::AcceptRole);
        msgBox.exec();
        if (msgBox.clickedButton() != overwriteButton)
            return;
    }

    while (!file.open(QFile::WriteOnly)) {
        if (QMessageBox::information(m_formWindow, tr("Open Error"),
            tr("There was an error opening template %1 for writing. Reason: %2").arg(name).arg(file.errorString()),
            QMessageBox::Retry|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
            return;
        }
    }

    QByteArray ba = m_formWindow->contents().toUtf8();
    while (file.write(ba) != ba.size()) {
        if (QMessageBox::information(m_formWindow, tr("Write Error"),
            tr("There was an error writing the template %1 to disk. Reason: %2").arg(name).arg(file.errorString()),
            QMessageBox::Retry|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel) {
                file.close();
                file.remove();
                return;
            }
            file.reset();
    }
    // update the list of places too...
    QStringList sl;
    for (int i = 0; i < m_addPathIndex; ++i)
        sl << ui.categoryCombo->itemText(i);

    QDesignerSettings().setFormTemplatePaths(sl);

    QDialog::accept();
}

void SaveFormAsTemplate::updateOKButton(const QString &str)
{
    QPushButton *okButton = ui.buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(!str.isEmpty());
}

void SaveFormAsTemplate::checkToAddPath(int itemIndex)
{
    if (itemIndex != m_addPathIndex)
        return;

    const QString dir = PreferencesDialog::chooseTemplatePath(this);
    if (dir.isEmpty()) {
        ui.categoryCombo->setCurrentIndex(0);
        return;
    }

    ui.categoryCombo->insertItem(m_addPathIndex, dir);
    ui.categoryCombo->setCurrentIndex(m_addPathIndex);
    ++m_addPathIndex;
}
