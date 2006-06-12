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

#include "settingsdialog.h"
#include "docuparser.h"
#include "config.h"

#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QToolButton>
#include <QTabWidget>
#include <QMap>
#include <QFontDatabase>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    init();
}

void SettingsDialog::init()
{
    Config *config = Config::configuration();
    ui.homePage->setText(config->homePage());
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

void SettingsDialog::on_buttonOk_clicked()
{
    Config *config = Config::configuration();
    config->setHomePage(ui.homePage->text());
    accept();
}

void SettingsDialog::on_buttonCancel_clicked()
{
    reject();
}

