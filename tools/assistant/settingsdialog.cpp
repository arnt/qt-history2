/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include <qlistbox.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qptrstack.h>
#include <qsettings.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qmap.h>


SettingsDialog::SettingsDialog(QWidget *parent, const char* name)
    : QDialog(parent, name)
{
    gui.setupUI(this);
    init();
}

void SettingsDialog::init()
{
    Config *config = Config::configuration();

    gui.browserApp->setText(config->webBrowser());
    gui.homePage->setText(config->homePage());
    gui.pdfApp->setText(config->pdfReader());
}


void SettingsDialog::on_colorButton_clicked()
{
    QColor c = QColorDialog::getColor(gui.colorButton->paletteBackgroundColor(), this);
    gui.colorButton->setPaletteBackgroundColor(c);
}

void SettingsDialog::on_buttonBrowse_clicked()
{
    setFile(gui.browserApp, tr("Qt Assistant - Set Web Browser"));
}

void SettingsDialog::on_buttonPDF_clicked()
{
    setFile(gui.pdfApp, tr("Qt Assistant - Set PDF Browser"));
}

void SettingsDialog::on_buttonHome_clicked()
{
    setFile(gui.homePage, tr("Qt Assistant - Set Homepage"));
}

void SettingsDialog::setFile(QLineEdit *le, const QString &caption)
{
    QFileDialog *fd = new QFileDialog(this);
    fd->setWindowTitle(caption);
    fd->setFileMode(QFileDialog::AnyFile);
    // ### fd->setDir(QDir::homeDirPath());

    if (fd->exec() == QDialog::Accepted) {
        if (!fd->selectedFiles().isEmpty())
           le->setText(fd->selectedFiles().at(0));
    }
}

void SettingsDialog::accept()
{
    Config *config = Config::configuration();

    config->setWebBrowser(gui.browserApp->text());
    config->setHomePage(gui.homePage->text());
    config->setPdfReader(gui.pdfApp->text());

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

