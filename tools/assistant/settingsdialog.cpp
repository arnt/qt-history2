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

#include "settingsdialog.h"
#include "docuparser.h"
#include "config.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qmap.h>


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    init();
}

void SettingsDialog::init()
{
    Config *config = Config::configuration();

    ui.browserApp->setText(config->webBrowser());
    ui.homePage->setText(config->homePage());
    ui.pdfApp->setText(config->pdfReader());
}


void SettingsDialog::on_colorButton_clicked()
{
    QPalette pal = ui.colorButton->palette();
	QColor c = QColorDialog::getColor(pal.color(QPalette::Button), this);
	pal.setColor(QPalette::Button, c);
    ui.colorButton->setPalette(pal);
}

void SettingsDialog::on_buttonBrowse_clicked()
{
    setFile(ui.browserApp, tr("Qt Assistant - Set Web Browser"));
}

void SettingsDialog::on_buttonPDF_clicked()
{
    setFile(ui.pdfApp, tr("Qt Assistant - Set PDF Browser"));
}

void SettingsDialog::on_buttonHome_clicked()
{
    setFile(ui.homePage, tr("Qt Assistant - Set Homepage"));
}

void SettingsDialog::setFile(QLineEdit *le, const QString &caption)
{
    QString fileName = QFileDialog::getOpenFileName(this, caption);
    if (!fileName.isEmpty())
        le->setText(fileName);    
}

void SettingsDialog::accept()
{
    Config *config = Config::configuration();

    config->setWebBrowser(ui.browserApp->text());
    config->setHomePage(ui.homePage->text());
    config->setPdfReader(ui.pdfApp->text());

    hide();
    done(Accepted);
}

void SettingsDialog::reject()
{
    init();
    done(Rejected);
}

void SettingsDialog::on_buttonOk_clicked()
{
    accept();
}

void SettingsDialog::on_buttonCancel_clicked()
{
    reject();
}

