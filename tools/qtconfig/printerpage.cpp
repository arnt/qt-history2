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

#include "printerpage.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFSFileEngine>
#include <QWidget>
#include <QFrame>
#include <QString>
#include <QCompleter>
#include <QDirModel>
#include <QLatin1String>
#include <QDir>
#include <QLatin1Char>
#include <QKeyEvent>
PrinterPage::PrinterPage(QWidget *parent)
    : QFrame(parent)
{
    setupUi(this);
    connect(leFontPath, SIGNAL(textChanged(QString)), this, SLOT(onLineEditTextChanged(QString)));
    connect(lwFontPaths, SIGNAL(currentRowChanged(int)), this, SLOT(onCurrentChanged()));
    connect(gbFontEmbedding, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(pbAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(pbRemove, SIGNAL(clicked()), this, SLOT(remove()));
    connect(lwFontPaths, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(tbUp, SIGNAL(clicked()), lwFontPaths, SLOT(moveCurrentUp()));
    connect(tbDown, SIGNAL(clicked()), lwFontPaths, SLOT(moveCurrentDown()));
    connect(pbBrowse, SIGNAL(clicked()), this, SLOT(onBrowseClicked()));
    connect(leFontPath, SIGNAL(returnPressed()), pbAdd, SLOT(animateClick()));
    QCompleter *completer = new QCompleter(leFontPath);
    QDirModel *dirmodel = new QDirModel(completer);
    dirmodel->setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    completer->setModel(dirmodel);
    leFontPath->setCompleter(completer);
    load();
}
void PrinterPage::save()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    settings.setValue("embedFonts", gbFontEmbedding->isChecked());
    settings.setValue("fontPath", lwFontPaths->items());
    settings.endGroup(); // Qt
}
void PrinterPage::load()
{
    QSettings settings(QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    gbFontEmbedding->setChecked(settings.value("embedFonts", true).toBool());
    lwFontPaths->setItems(settings.value("fontPath").toStringList());
    settings.endGroup(); // Qt
}
void PrinterPage::onBrowseClicked()
{
    const QDir dir(leFontPath->text());
    const QString dirname = QFileDialog::getExistingDirectory(this,
                                                              tr("Select a Directory"),
                                                              dir.exists() ? dir.path() : QDir::currentPath());
    if (!dirname.isEmpty()) {
        leFontPath->setText(dirname);
    }
}
void PrinterPage::add()
{
    const QString path = leFontPath->text();
    if (!path.isEmpty()) {
        if (lwFontPaths->items().contains(path)) {
            QMessageBox::critical(this, tr("Already added"),
                                  QLatin1Char('\'') + path + QLatin1String("' ")
                                  + tr("is already added"));
            return;
        } else if (!QDir::isAbsolutePath(path)) {
            QMessageBox::critical(this, "Not a valid path",
                                  QString("'%1' is not an absolute path").
                                  arg(path));
            return;
        } else if (!QDir(path).exists()) {
            if (QMessageBox::warning(this, tr("Not an existing directory"),
                                     QLatin1Char('\'') + path + QLatin1String("' ")
                                     + tr("does not exist. Are you sure you want to add this path?"),
                                     QMessageBox::Yes|QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
            }
        }


        lwFontPaths->addItem(path);
        lwFontPaths->setCurrentRow(lwFontPaths->count() - 1);
        leFontPath->selectAll();
        leFontPath->setFocus();
        pbAdd->setEnabled(false);
        emit changed();
    }
}

void PrinterPage::remove()
{
    if (lwFontPaths->currentItem()) {
        delete lwFontPaths->currentItem();
        emit changed();
    }
}

void PrinterPage::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
    switch (e->key()) {
    case Qt::Key_Delete:
        if (e->modifiers() == Qt::NoModifier) {
            remove();
            e->accept();
        }
        break;
    case Qt::Key_D:
        if (e->modifiers() == Qt::ControlModifier) {
            remove();
            e->accept();
        }
        break;
    default:
        break;
    }
}
void PrinterPage::onLineEditTextChanged(const QString &str)
{
    pbAdd->setEnabled(!str.isEmpty()
                      && !lwFontPaths->items().contains(str,
                                                        QFSFileEngine().caseSensitive()
                                                        ? Qt::CaseSensitive
                                                        : Qt::CaseInsensitive));
}
void PrinterPage::onCurrentChanged()
{
    pbRemove->setEnabled(lwFontPaths->currentItem() != 0);
    tbUp->setEnabled(lwFontPaths->isUpEnabled());
    tbDown->setEnabled(lwFontPaths->isDownEnabled());
}
