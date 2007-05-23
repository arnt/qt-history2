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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "preferences.h"
#include <iconloader_p.h>
#include <stylesheeteditor_p.h>

#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>

PreferencesDialog::PreferencesDialog(QWidget *parentWidget) :
    QDialog(parentWidget)
{
    m_ui = new Ui::PreferencesDialog();
    m_ui->setupUi(this);

    m_ui->m_addTemplatePathButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("plus.png")));
    m_ui->m_removeTemplatePathButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("minus.png")));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_ui->m_uiModeCombo->addItem(tr("Docked Window"), QVariant(DockedMode));
    m_ui->m_uiModeCombo->addItem(tr("Multiple Top-Level Windows"), QVariant(TopLevelMode));

    m_ui->m_fontPanel->setCheckable(true);
    m_ui->m_fontPanel->setTitle(tr("Toolwindow Font"));
    m_ui->m_gridPanel->setTitle(tr("Default Grid"));

    connect(m_ui->m_templatePathListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(templatePathSelectionChanged()));
    connect(m_ui->m_addTemplatePathButton, SIGNAL(clicked()), this, SLOT(addTemplatePath()));
    connect(m_ui->m_removeTemplatePathButton, SIGNAL(clicked()), this, SLOT(removeTemplatePath()));
    connect(m_ui->m_dialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_ui->m_dialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));

    m_ui->m_styleCombo->addItems(QStyleFactory::keys());

    m_ui->m_appStyleSheetLineEdit->setTextPropertyValidationMode( qdesigner_internal::ValidationStyleSheet);
    m_ui->m_appStyleSheetClearButton->setIcon(qdesigner_internal::createIconSet(QString::fromUtf8("resetproperty.png")));
    connect(m_ui->m_appStyleSheetClearButton, SIGNAL(clicked()), m_ui->m_appStyleSheetLineEdit, SLOT(clear()));
    connect(m_ui->m_appStyleSheetChangeButton, SIGNAL(clicked()), this, SLOT(editAppStyleSheet()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

bool PreferencesDialog::showDialog(Preferences &p)
{
    setPreferences(p);
    if (exec() != Accepted)
        return false;

    getPreferences(p);
    return true;
}

void PreferencesDialog::setPreferences(const Preferences &p)
{
    m_ui->m_uiModeCombo->setCurrentIndex(m_ui->m_uiModeCombo->findData(QVariant(p.m_uiMode)));
    m_ui->m_fontPanel->setWritingSystem(p.m_writingSystem);
    m_ui->m_fontPanel->setSelectedFont(p.m_font);
    m_ui->m_fontPanel->setChecked(p.m_useFont);
    // add paths and select 0
    m_ui->m_templatePathListWidget->clear();
    if (p.m_additionalTemplatePaths.empty()) {
        // disable button
        templatePathSelectionChanged();
    } else {
        foreach (QString templatePath, p.m_additionalTemplatePaths)
            m_ui->m_templatePathListWidget->addItem(templatePath);
        m_ui->m_templatePathListWidget->setCurrentItem(m_ui->m_templatePathListWidget->item(0));
    }
    m_ui->m_gridPanel->setGrid(p.m_defaultGrid);

    const bool hasStyle = !p.m_style.isEmpty();
    m_ui->m_styleGroupBox->setChecked(hasStyle);

    int styleIndex = hasStyle ?  m_ui->m_styleCombo->findText(p.m_style) : -1;
    if (styleIndex == -1)
        styleIndex = 0;
    m_ui->m_styleCombo->setCurrentIndex(styleIndex);
    m_ui->m_appStyleSheetLineEdit->setText(p.m_appStyleSheet);
}

void PreferencesDialog::getPreferences(Preferences &p) const
{
    p.m_uiMode = static_cast<UIMode>(m_ui->m_uiModeCombo->itemData(m_ui->m_uiModeCombo->currentIndex()).toInt());
    p.m_font = m_ui->m_fontPanel->selectedFont();
    p.m_useFont = m_ui->m_fontPanel->isChecked();
    p.m_writingSystem = m_ui->m_fontPanel->writingSystem();

    p.m_additionalTemplatePaths.clear();
    const int count = m_ui->m_templatePathListWidget->count();
    for (int i = 0; i <  count; i++) {
        p.m_additionalTemplatePaths += m_ui->m_templatePathListWidget->item(i)->text();
    }
    p.m_defaultGrid = m_ui->m_gridPanel->grid();

    if (m_ui->m_styleGroupBox->isChecked()) {
        p.m_style = m_ui->m_styleCombo->currentText();
    } else {
        p.m_style.clear();
    }
    p.m_appStyleSheet = m_ui->m_appStyleSheetLineEdit->text();
}

void PreferencesDialog::addTemplatePath()
{
    const QString templatePath = chooseTemplatePath(this);
    if (templatePath.isEmpty())
        return;

    const QList<QListWidgetItem *> existing = m_ui->m_templatePathListWidget->findItems(templatePath, Qt::MatchExactly);
    if (!existing.empty())
        return;

    QListWidgetItem *newItem = new QListWidgetItem(templatePath);
    m_ui->m_templatePathListWidget->addItem(newItem);
    m_ui->m_templatePathListWidget->setCurrentItem(newItem);
}


void PreferencesDialog::removeTemplatePath()
{
    const QList<QListWidgetItem *> selectedPaths = m_ui->m_templatePathListWidget->selectedItems();
    if (selectedPaths.empty())
        return;
    delete selectedPaths.front();
}

void PreferencesDialog::templatePathSelectionChanged()
{
    const QList<QListWidgetItem *> selectedPaths = m_ui->m_templatePathListWidget->selectedItems();
    m_ui->m_removeTemplatePathButton->setEnabled(!selectedPaths.empty());
}

QString PreferencesDialog::chooseTemplatePath(QWidget *parent)
{
    QString rc = QFileDialog::getExistingDirectory(parent, tr("Pick a directory to save templates in"));
    if (rc.isEmpty())
        return rc;

    if (rc.endsWith(QDir::separator()))
        rc.remove(rc.size() - 1, 1);
    return rc;
}

void PreferencesDialog::editAppStyleSheet()
{
    qdesigner_internal::StyleSheetEditorDialog dlg(this);
    dlg.setText(m_ui->m_appStyleSheetLineEdit->text());
    if (dlg.exec() == QDialog::Accepted)
        m_ui->m_appStyleSheetLineEdit->setText(dlg.text());
}
